/*!
 * \file MainWindow.cpp
 * \brief Implementation of MainWindow.h: GUI, worker threads, result evaluation.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */

#include "MainWindow.h"

#include <wx/app.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/datetime.h>
#include <wx/filefn.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/msw/wrapwin.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/strconv.h>
#include <wx/txtstrm.h>
#include <wx/utils.h>
#include <wx/wfstream.h>

#include <algorithm>
#include <string>
#include <thread>

wxDEFINE_EVENT(wxEVT_THREAD_RESULT, wxThreadEvent);

namespace {

/*! \brief Text colour of a PASS row, readable on the dark background. */
const wxColour kPassColour(120, 220, 120);
/*! \brief Text colour of a FAIL row, readable on the dark background. */
const wxColour kFailColour(255, 130, 130);

/*! \brief Size of the buffer used to read the pipe of a command (heap, not the thread's small stack). */
constexpr std::size_t kPipeBufferSize = 64 * 1024;

/*!
 * \brief Converts the bytes written by a console program to text.
 *
 * Console programs started through "cmd /c" write in the OEM code page of the system, not in the
 * ANSI one used by the GUI.
 * \param raw bytes read from the pipe.
 * \return the text; never throws, undecodable bytes fall back to a 1:1 mapping.
 */
wxString DecodeConsoleOutput(const std::string& raw)
{
    if (raw.empty()) return wxString();
    wxCSConv conv(wxString::Format("CP%u", static_cast<unsigned>(GetOEMCP())));
    wxString text;
    if (conv.IsOk()) text = wxString(raw.data(), conv, raw.size());
    if (text.empty()) text = wxString::From8BitData(raw.data(), raw.size());
    return text;
}

/*!
 * \brief Runs a command line through "cmd /c" without any visible window and captures its output.
 *
 * Standard output and standard error go to the same pipe; standard input is the NUL device so a
 * command that asks for input (for example "pause") ends instead of waiting forever. Called from a
 * worker thread: it must not touch any window.
 *
 * The console of the command cannot be shown and captured at the same time with this technique; the
 * "Show" option of the rows is therefore not implemented.
 * \param command the command line as typed in the row.
 * \return the output, or a text starting with "Error:" if the command could not be started.
 */
wxString RunCommand(const wxString& command)
{
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

    HANDLE hRead = nullptr;
    HANDLE hWrite = nullptr;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return "Error: Failed to create pipe!";
    // The child must inherit only the write end.
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    HANDLE hNul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = (hNul != INVALID_HANDLE_VALUE) ? hNul : nullptr;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite; // stderr goes to the same pipe as stdout

    // CreateProcessW may modify the command line, so it needs its own writable buffer.
    std::wstring commandLine = L"cmd /c \"" + command.ToStdWstring() + L"\"";

    PROCESS_INFORMATION pi = {};
    const BOOL started = CreateProcessW(nullptr, &commandLine[0], nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                                        nullptr, nullptr, &si, &pi);

    // The parent must close its copy of the write end, or ReadFile() would never see the end of the output.
    CloseHandle(hWrite);
    if (hNul != INVALID_HANDLE_VALUE) CloseHandle(hNul);

    if (!started) {
        CloseHandle(hRead);
        return "Error: Failed to execute command!";
    }

    std::string raw;
    std::vector<char> buffer(kPipeBufferSize);
    DWORD bytesRead = 0;
    while (ReadFile(hRead, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr) && bytesRead > 0) {
        raw.append(buffer.data(), bytesRead); // append by length: the output may contain NUL bytes
    }

    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return DecodeConsoleOutput(raw);
}

/*!
 * \brief Splits an expected-result text of the form ":File:<path>::<text>".
 * \param spec the text of the "expected result" field.
 * \param tag the ":File:" prefix.
 * \param separator the "::" separator.
 * \param[out] path the file to search.
 * \param[out] text the text to find in it.
 * \return false if \p spec is not of that form.
 */
bool ParseFileSpec(const wxString& spec, const wxString& tag, const wxString& separator, wxString& path, wxString& text)
{
    const int tagPos = spec.Find(tag);
    if (tagPos == wxNOT_FOUND) return false;
    const wxString rest = spec.Mid(static_cast<size_t>(tagPos) + tag.length());
    // The first "::" ends the path: a path cannot contain it (the ':' of a drive letter is alone).
    const int sepPos = rest.Find(separator);
    if (sepPos == wxNOT_FOUND) return false;
    path = rest.Left(static_cast<size_t>(sepPos));
    path.Trim().Trim(false);
    text = rest.Mid(static_cast<size_t>(sepPos) + separator.length());
    return !path.empty();
}

/*!
 * \brief Looks for a text in a file, line by line (the file is never loaded entirely in memory).
 *
 * Because the file is read line by line the text cannot span several lines.
 * \param filePath the file to read.
 * \param searchString the text to find.
 * \param[out] opened false if the file could not be opened.
 * \return true if a line of the file contains \p searchString.
 */
bool FindInFile(const wxString& filePath, const wxString& searchString, bool& opened)
{
    wxFileInputStream input(filePath);
    opened = input.IsOk();
    if (!opened) return false;

    wxTextInputStream text(input);
    while (input.IsOk() && !input.Eof()) {
        if (text.ReadLine().Contains(searchString)) return true;
    }
    return false;
}

/*!
 * \brief Decides whether a command passed. Runs in the worker thread.
 * \param expected the "expected result" field: plain text to find in \p output, or ":File:<path>::<text>".
 * \param output console output of the command.
 * \param tag the ":File:" prefix.
 * \param separator the "::" separator.
 * \param[out] note explanation for the operator when the evaluation could not be done (missing file...).
 * \return true for PASS, false for FAIL.
 */
bool EvaluateOutcome(const wxString& expected, const wxString& output, const wxString& tag,
                     const wxString& separator, wxString& note)
{
    if (expected.Find(tag) == wxNOT_FOUND) return output.Contains(expected);

    wxString path;
    wxString text;
    if (!ParseFileSpec(expected, tag, separator, path, text)) {
        note = wxString::Format("Expected result \"%s\" is not of the form %s<file>%s<text>", expected, tag, separator);
        return false;
    }
    if (!wxFileExists(path)) {
        note = wxString::Format("File %s doesn't exist", path);
        return false;
    }
    bool opened = false;
    const bool found = FindInFile(path, text, opened);
    if (!opened) note = wxString::Format("Failed to open file \"%s\"", path);
    return found;
}

} // namespace

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "Parallel Command Runner - PCR")
{
    m_sink->target = this;

    SetMinSize(FromDIP(wxSize(900, 500)));
    SetSize(FromDIP(wxSize(1000, 640)));

    // --- menus -------------------------------------------------------------------------------
    auto* menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);

    auto* settingsMenu = new wxMenu;
    settingsMenu->Append(windowIDs::ID_ENABLE_EDIT, "&Enable Edit\tCtrl-E", "Enable edit of commands properties");
    settingsMenu->Append(windowIDs::ID_DISABLE_EDIT, "&Disable Edit\tCtrl-D", "Disable edit of commands properties");
    settingsMenu->AppendSeparator();
    settingsMenu->Append(windowIDs::ID_STOP_WAITING, "&Stop waiting\tCtrl-B",
                         "Stop waiting for the running single command and go on with the next ones");

    auto* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    auto* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(settingsMenu, "&Settings");
    menuBar->Append(menuHelp, "&Info");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Command Runner!");

    // --- controls ----------------------------------------------------------------------------
    m_mainPanel = new wxPanel(this, windowIDs::ID_MAIN_PANEL);

    m_runBT = new wxButton(m_mainPanel, windowIDs::ID_RUN_COMMAND_BT, "Run command(s)", wxDefaultPosition,
                           FromDIP(wxSize(-1, 40)));

    m_cmdsSizer = new wxBoxSizer(wxVERTICAL);
    m_cmds.reserve(kNrOfCmds);
    for (int i = 0; i < kNrOfCmds; i++) {
        m_cmds.push_back(std::make_unique<cmdgui>(m_mainPanel, i));
        m_cmds.back()->setCounters(wxString::Format("counters of CMD: %d", i + 1));
        m_cmdsSizer->Add(m_cmds.back()->getPointer(), 0, wxEXPAND | wxALL, 1);
    }

    m_resultList = new wxListCtrl(m_mainPanel, windowIDs::ID_COMMAND_LIST, wxDefaultPosition, wxDefaultSize,
                                  wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    m_resultList->InsertColumn(0, "Timestamp");
    m_resultList->InsertColumn(1, "Information");

    // --- layout ------------------------------------------------------------------------------
    auto* componentsSizer = new wxBoxSizer(wxVERTICAL);
    componentsSizer->Add(m_runBT, 0, wxEXPAND | wxALL, 1);
    componentsSizer->Add(m_cmdsSizer, 0, wxEXPAND | wxALL, 1);
    componentsSizer->Add(m_resultList, 1, wxEXPAND | wxALL, 1); // the list takes all the remaining space
    m_mainPanel->SetSizer(componentsSizer);

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_mainPanel, 1, wxEXPAND);
    SetSizer(mainSizer);

    // --- events ------------------------------------------------------------------------------
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainWindow::OnEnable, this, windowIDs::ID_ENABLE_EDIT);
    Bind(wxEVT_MENU, &MainWindow::OnDisable, this, windowIDs::ID_DISABLE_EDIT);
    Bind(wxEVT_MENU, &MainWindow::OnStopWaiting, this, windowIDs::ID_STOP_WAITING);
    // Command events of the children travel up to the frame.
    Bind(wxEVT_CHECKBOX, &MainWindow::OnGuiEvent, this);
    Bind(wxEVT_BUTTON, &MainWindow::OnButtonEvent, this);
    Bind(wxEVT_THREAD_RESULT, &MainWindow::OnThreadResult, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnClose, this);
    m_resultList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnKeyDown, this);
    m_resultList->Bind(wxEVT_MOTION, &MainWindow::OnMouseMove, this);
    m_resultList->Bind(wxEVT_SIZE, &MainWindow::OnListSize, this);

    Layout();
}

MainWindow::~MainWindow()
{
    // Whatever the way the window goes away, the workers must not post to it any more.
    std::lock_guard<std::mutex> lock(m_sink->mutex);
    m_sink->target = nullptr;
}

void MainWindow::OnExit(wxCommandEvent&)
{
    Close(true);
}

int MainWindow::RowOfId(int id, int& offset) const
{
    const int relative = id - windowIDs::ID_GUI_CLASS;
    if (relative < 0 || relative >= kNrOfCmds * windowIDs::kIdsPerCmdRow) return -1;
    offset = relative % windowIDs::kIdsPerCmdRow;
    return relative / windowIDs::kIdsPerCmdRow;
}

void MainWindow::OnGuiEvent(wxCommandEvent& event)
{
    int offset = 0;
    const int row = RowOfId(event.GetId(), offset);
    if (row < 0) {
        event.Skip();
        return;
    }

    cmdgui& cmd = *m_cmds[static_cast<size_t>(row)];
    switch (offset) {
    case 0: // ON / OFF
        if (cmd.Cmd_active_CB->GetValue()) cmd.enable();
        else cmd.disable();
        break;
    case cmdgui::SEQUENTIAL_ID_INDEX:
        cmd.setSequential(cmd.Cmd_sequential_CB->GetValue());
        break;
    case cmdgui::VIEW_ID_INDEX:
        cmd.setView(cmd.Cmd_view_CB->GetValue());
        break;
    default:
        event.Skip();
        break;
    }
}

void MainWindow::OnButtonEvent(wxCommandEvent& event)
{
    if (event.GetId() == windowIDs::ID_RUN_COMMAND_BT) {
        onRunCommand(event);
        return;
    }

    int offset = 0;
    const int row = RowOfId(event.GetId(), offset);
    if (row < 0 || offset != cmdgui::RUNBUTTON_ID_INDEX) {
        event.Skip();
        return;
    }

    // The "Run" button of a single row: start only that command (it never blocks the others).
    cmdgui& cmd = *m_cmds[static_cast<size_t>(row)];
    if (cmd.getRunning()) return;
    if (cmd.setRunning(true)) {
        StartThread(cmd.getCmd(), row);
    }
    else {
        AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD %d gui to BUSY", row + 1));
    }
}

void MainWindow::OnAbout(wxCommandEvent&)
{
    wxString text = "This APP can run multiple \"*.bat\" file or CMD like commands in separated threads (aka simultaneously)\n";
    text += "Results of each execution will be shown as separated messages in the list\n\n";
    text += "Dev. Coga F. (EGO Group S.r.L @ fation.coga@egogroup.eu)\n\n";
    text += wxString::Format("Built with %s", wxVERSION_STRING);
    wxMessageBox(text, "About PCR", wxOK | wxICON_INFORMATION, this);
}

void MainWindow::OnEnable(wxCommandEvent&)
{
    EnableCmds();
}

void MainWindow::EnableCmds()
{
    for (auto& cmd : m_cmds) cmd->enableEditables();
}

void MainWindow::OnDisable(wxCommandEvent&)
{
    DisableCmds();
}

void MainWindow::DisableCmds()
{
    for (auto& cmd : m_cmds) cmd->disableEditables();
}

void MainWindow::OnStopWaiting(wxCommandEvent&)
{
    if (!m_blocking) return;
    AddMessage(get_current_timestamp(), wxString::Format("Stopped waiting for CMD %d", m_blockingCommandIndex + 1));
    m_blocking = false;
    m_blockingCommandIndex = -1;
}

bool MainWindow::AnyCommandRunning() const
{
    return std::any_of(m_cmds.begin(), m_cmds.end(), [](const std::unique_ptr<cmdgui>& c) { return c->getRunning(); });
}

void MainWindow::OnKeyDown(wxKeyEvent& event)
{
    if (event.ControlDown() && event.GetKeyCode() == 'A') {
        SelectAllItems();
    }
    else if (event.ControlDown() && event.GetKeyCode() == 'C') {
        CopySelectedRow();
    }
    else {
        event.Skip(); // let the other keys work normally
    }
}

void MainWindow::SelectAllItems()
{
    const long count = m_resultList->GetItemCount();
    for (long i = 0; i < count; i++) {
        m_resultList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

wxString MainWindow::get_current_timestamp() const
{
    return wxDateTime::Now().Format("%Y-%m-%d %H:%M:%S");
}

void MainWindow::OnMouseMove(wxMouseEvent& event)
{
    int flags = 0;
    const long item = m_resultList->HitTest(event.GetPosition(), flags);
    // Changing the tooltip at every mouse move makes it flicker: do it only when the row changes.
    if (item != m_lastTipItem) {
        m_lastTipItem = item;
        if (item >= 0) {
            m_resultList->SetToolTip(m_resultList->GetItemText(item, 0) + "\n" + m_resultList->GetItemText(item, 1));
        }
        else {
            m_resultList->UnsetToolTip();
        }
    }
    event.Skip();
}

void MainWindow::OnListSize(wxSizeEvent& event)
{
    // A fixed-width timestamp column, the information column takes the rest (without a horizontal scroll bar).
    const int timestampWidth = FromDIP(150);
    const int scrollBar = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, m_resultList);
    const int messageWidth = std::max(FromDIP(100), m_resultList->GetClientSize().GetWidth() - timestampWidth - scrollBar);
    m_resultList->SetColumnWidth(0, timestampWidth);
    m_resultList->SetColumnWidth(1, messageWidth);
    event.Skip();
}

void MainWindow::CopySelectedRow()
{
    wxString allSelectedRows;
    const int columns = m_resultList->GetColumnCount();

    long item = -1;
    while ((item = m_resultList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != wxNOT_FOUND) {
        for (int col = 0; col < columns; col++) {
            allSelectedRows += m_resultList->GetItemText(item, col);
            if (col < columns - 1) allSelectedRows += "\t"; // tab separated: pastes well into spreadsheets
        }
        allSelectedRows += "\n";
    }
    if (allSelectedRows.IsEmpty()) return;

    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(allSelectedRows));
        wxTheClipboard->Close();
    }
}

void MainWindow::AddMessage(const wxString& timestamp, const wxString& message, const wxColour& colour)
{
    const long index = m_resultList->InsertItem(0, timestamp); // newest on top
    m_resultList->SetItem(index, 1, message);
    if (colour.IsOk()) m_resultList->SetItemTextColour(index, colour);
}

void MainWindow::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto() && AnyCommandRunning()) {
        if (wxMessageBox("Are you sure you want to quit, unfinished CMDs will be terminated!!!",
                         "Request to quit application", wxICON_QUESTION | wxYES_NO, this) != wxYES) {
            // Like the veto power in the UN Security Council: it keeps the window open.
            event.Veto();
            return;
        }
    }
    m_closing = true;
    m_blocking = false; // releases WaitWhileBlocking()
    {
        std::lock_guard<std::mutex> lock(m_sink->mutex);
        m_sink->target = nullptr; // no result is posted to this window from now on
    }
    Destroy();
}

void MainWindow::WaitWhileBlocking()
{
    // wxYield() keeps the GUI responsive inside a loop that could run for ever if the command never
    // ends. "Settings > Stop waiting" (Ctrl-B) and closing the window both end it.
    while (m_blocking && !m_closing) {
        wxYield();
        wxMilliSleep(10); // do not burn a whole CPU core while waiting
    }
}

bool MainWindow::StartSingleCommand(int i)
{
    cmdgui& cmd = *m_cmds[static_cast<size_t>(i)];

    // The operator may wait for the previous commands in case this one depends on them.
    wxMessageBox("Next command will be executed in single mode, the next ones will wait for this to complete\n"
                 "Make sure it does not depend on previous commands and hit OK when ready",
                 "ATTENTION Blocking thread", wxOK | wxICON_INFORMATION, this);
    if (m_closing) return false;

    // There is no "previous" command for the first row (and no out-of-bounds access either).
    const bool previousFailed = (i > 0) && !m_cmds[static_cast<size_t>(i) - 1]->getResult();
    if (previousFailed &&
        wxMessageBox("Found at least one previous executed CMD with fail result\nDo you still want to proceed?",
                     "ATTENTION Previous Failure", wxYES_NO, this) != wxYES) {
        // Keep track of the decision and leave the row idle.
        AddMessage(get_current_timestamp(), wxString::Format("Skipped CMD: %d", i + 1));
        cmd.setRunning(false);
        return false;
    }

    // The flags are set before the thread starts, so the result can never arrive before them.
    m_blocking = true;
    m_blockingCommandIndex = i;
    StartThread(cmd.getCmd(), i);
    return true;
}

void MainWindow::onRunCommand(wxCommandEvent&)
{
    // The loop below waits (yielding to the GUI) for single commands: the button could be pressed again meanwhile.
    if (m_runAllInProgress) {
        AddMessage(get_current_timestamp(), "A run of the commands is already in progress");
        return;
    }

    DisableCmds(); // the commands must not be edited while they run

    bool foundRunning = false;
    for (size_t i = 0; i < m_cmds.size(); i++) {
        if (m_cmds[i]->getRunning()) {
            AddMessage(get_current_timestamp(), wxString::Format("Found cmd %d active", static_cast<int>(i) + 1));
            foundRunning = true;
        }
        m_cmds[i]->setResult(true); // every command starts as "not failed"
    }

    if (foundRunning &&
        wxMessageBox("Found at least one thread still active:\nDo you want to launch all the not running commands?",
                     " PCR - Confirmation Request", wxYES_NO, this) != wxYES) {
        return;
    }

    m_runAllInProgress = true;
    for (int i = 0; i < kNrOfCmds && !m_closing; i++) {
        cmdgui& cmd = *m_cmds[static_cast<size_t>(i)];

        if (cmd.getRunning()) {
            // Still running from a previous run, only trace that it was skipped.
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD %d already active", i + 1));
            continue;
        }
        if (!cmd.isActive()) continue; // OFF: not to be run, visible from the GUI

        if (!cmd.setRunning(true)) {
            // Never seen, but possible if a widget has a problem.
            AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD %d gui to BUSY", i + 1));
            continue;
        }

        if (cmd.isSequential()) {
            if (StartSingleCommand(i)) WaitWhileBlocking();
        }
        else {
            StartThread(cmd.getCmd(), i);
        }
    }
    m_runAllInProgress = false;
}

void MainWindow::StartThread(const wxString& input, int commandIndex)
{
    // Everything the worker needs is copied here, on the main thread: it never reads a widget.
    const std::shared_ptr<EventSink> sink = m_sink;
    const wxString command = input;
    const wxString expected = m_cmds[static_cast<size_t>(commandIndex)]->getPositiveVal();
    const wxString tag = m_searchOnFileTag;
    const wxString separator = m_separator;

    std::thread([sink, command, expected, tag, separator, commandIndex]() {
        CommandOutcome outcome;
        outcome.index = commandIndex;
        outcome.output = RunCommand(command);
        outcome.pass = EvaluateOutcome(expected, outcome.output, tag, separator, outcome.note);

        auto* event = new wxThreadEvent(wxEVT_THREAD_RESULT);
        event->SetPayload(outcome);

        std::lock_guard<std::mutex> lock(sink->mutex);
        if (sink->target != nullptr) wxQueueEvent(sink->target, event); // the window takes ownership
        else delete event;                                              // the window is gone
    }).detach(); // detached: the GUI never waits for it

#ifdef _DEBUG // only in debug builds
    AddMessage(get_current_timestamp(), wxString::Format("Started thread for CMD %d", commandIndex + 1));
#endif
}

void MainWindow::OnThreadResult(wxThreadEvent& event)
{
    const CommandOutcome outcome = event.GetPayload<CommandOutcome>();
    if (outcome.index < 0 || outcome.index >= kNrOfCmds) return;
    const size_t i = static_cast<size_t>(outcome.index);
    cmdgui& cmd = *m_cmds[i];

    if (outcome.pass) m_passed[i]++;
    else m_failed[i]++;
    cmd.setResult(outcome.pass);
    cmd.setCounters(wxString::Format("  P= %03d || F= %03d", m_passed[i], m_failed[i]));
    cmd.setRunning(false);

    // If this is the command that blocks "Run command(s)", let the next ones go.
    if (m_blocking && m_blockingCommandIndex == outcome.index) {
        m_blocking = false;
        m_blockingCommandIndex = -1;
    }

    wxString output = outcome.output;
    output.Trim(); // the trailing line break of a console program only makes the row taller
    const wxString status = outcome.pass ? "PASS" : "FAIL";
    const wxColour colour = outcome.pass ? kPassColour : kFailColour;

    if (!outcome.note.empty()) AddMessage(get_current_timestamp(), outcome.note, kFailColour);
    AddMessage(get_current_timestamp(), wxString::Format("Cmd %d %s, result is: %s", outcome.index + 1, status, output),
               colour);
}
