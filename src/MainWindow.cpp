/*!
 * \file MainWindow.cpp
 * \brief Implementation of MainWindow.h: GUI, worker threads, result evaluation.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */

#include "MainWindow.h"

#include "AboutDialog.h"
#include "App.h"
#include "AppInfo.h"
#include "AppSettings.h"
#include "CommandRunner.h"
#include "BatchEditorFrame.h"
#include "BuildInfo.h"
#include "DataDir.h"
#include "LicenseDialog.h"
#include "LicenseManager.h"
#include "I18n.h"
#include "LicensePolicy.h"
#include "Log.h"
#include "ResultCheck.h"
#include "UVT.h"

#include <wx/app.h>
#include <wx/clipbrd.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/imagpng.h>
#include <wx/dataobj.h>
#include <wx/datetime.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/msw/wrapwin.h>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/strconv.h>
#include <wx/txtstrm.h>
#include <wx/utils.h>
#include <wx/wfstream.h>
#include <wx/file.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <thread>

wxDEFINE_EVENT(wxEVT_THREAD_RESULT, wxThreadEvent);

namespace {

/*! \brief Text colour of a PASS row, readable on the dark background. */
const wxColour kPassColour(120, 220, 120);
/*! \brief Text colour of a FAIL row, readable on the dark background. */
const wxColour kFailColour(255, 130, 130);

} // namespace

MainWindow::MainWindow(const StartOptions& options)
    : wxFrame(nullptr, wxID_ANY, wxString::FromUTF8(GetWindowTitle())),
      m_licenseTimer(this, windowIDs::ID_LICENSE_TIMER),
      m_options(options)
{
    m_unattended = options.run;
    m_sink->target = this;

    SetMinSize(FromDIP(wxSize(900, 500)));
    SetSize(FromDIP(wxSize(1000, 640)));

    // --- menus -------------------------------------------------------------------------------
    auto* menuFile = new wxMenu;
    menuFile->Append(windowIDs::ID_NEW_BATCH, "&New batch file\tCtrl-N", "Open the batch file editor with a PASS / FAIL skeleton");
    menuFile->Append(windowIDs::ID_OPEN_BATCH, "&Open batch file...\tCtrl-O", "View and edit a batch file, with a reference of the batch commands");
    menuFile->Append(windowIDs::ID_OPEN_PROJECT, "Open p&roject...", "Open a project file: the command rows, the result file, the repeat count");
    menuFile->Append(windowIDs::ID_SAVE_PROJECT, "&Save project\tCtrl-S", "Save the command rows in the current project file");
    menuFile->Append(windowIDs::ID_SAVE_PROJECT_AS, "Save project &as...", "Save the command rows in another project file");
    menuFile->AppendSeparator();
    menuFile->Append(windowIDs::ID_EXPORT_CSV, "E&xport results (CSV)...", "Save the results of this session as a CSV file (spreadsheet)");
    menuFile->Append(windowIDs::ID_CLEAR_RESULTS, "C&lear results", "Empty the result list (and the results kept for the export)");
    menuFile->AppendSeparator();
    menuFile->Append(windowIDs::ID_OPEN_EXAMPLES, "Open &examples folder", "Open the folder with the example batch files");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    auto* settingsMenu = new wxMenu;
    settingsMenu->Append(windowIDs::ID_ENABLE_EDIT, "&Enable Edit\tCtrl-E", "Enable edit of commands properties");
    settingsMenu->Append(windowIDs::ID_DISABLE_EDIT, "&Disable Edit\tCtrl-D", "Disable edit of commands properties");
    settingsMenu->AppendSeparator();
    settingsMenu->Append(windowIDs::ID_SELECT_RESULT_FILE, "Select &result file...", "Choose the file where the batch files write their result");
    settingsMenu->Append(windowIDs::ID_OPEN_RESULT_FILE, "Open r&esult file", "Open the result file with its default program");
    settingsMenu->Append(windowIDs::ID_OPEN_DATA_FOLDER, "Open &data folder", "Open the folder with the log, settings and license");
    settingsMenu->AppendSeparator();
    settingsMenu->Append(windowIDs::ID_STOP_WAITING, "&Stop waiting\tCtrl-B",
                         "Stop waiting for the running single command (or for the end of a run) and go on");
    settingsMenu->Append(windowIDs::ID_STOP_REPEAT, "Stop &repeating\tCtrl-R",
                         "Let the current run end, then do not start the next repetition");

    auto* menuHelp = new wxMenu;
    menuHelp->Append(windowIDs::ID_LICENSE, "&License...\tCtrl-K", "The license in use, the trial and how to get a license");
    menuHelp->AppendSeparator();
    menuHelp->Append(wxID_ABOUT, "&About...", "About Process Launcher");

    auto* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(settingsMenu, "&Settings");
    menuBar->Append(menuHelp, "&Info");
    SetMenuBar(menuBar);

    CreateStatusBar(2);
    const int statusWidths[2] = { -1, FromDIP(330) };
    SetStatusWidths(2, statusWidths);
    SetStatusText("Command Runner!", 0);

    // --- controls ----------------------------------------------------------------------------
    m_mainPanel = new wxPanel(this, windowIDs::ID_MAIN_PANEL);

    m_runBT = new wxButton(m_mainPanel, windowIDs::ID_RUN_COMMAND_BT, "Run command(s)", wxDefaultPosition,
                           FromDIP(wxSize(-1, 40)));
    auto* repeatLabel = new wxStaticText(m_mainPanel, wxID_ANY, "Repeat:");
    m_repeatSpin = new wxSpinCtrl(m_mainPanel, windowIDs::ID_REPEAT_SPIN, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(90, -1)),
                                  wxSP_ARROW_KEYS, 0, 100000, 1);
    m_repeatSpin->SetToolTip("How many times Run command(s) runs the rows; each run waits for the previous one to end.\n"
                             "0 = until Settings > Stop repeating (Ctrl-R).");
    auto* runRow = new wxBoxSizer(wxHORIZONTAL);
    runRow->Add(m_runBT, 1, wxEXPAND | wxRIGHT, FromDIP(6));
    runRow->Add(repeatLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(4));
    runRow->Add(m_repeatSpin, 0, wxALIGN_CENTER_VERTICAL);

    // Result file: the full path is always visible and can be typed or chosen.
    auto* resultRow = new wxBoxSizer(wxHORIZONTAL);
    auto* resultLabel = new wxStaticText(m_mainPanel, wxID_ANY, "Result file:");
    m_resultFileTxt = new wxTextCtrl(m_mainPanel, windowIDs::ID_RESULT_FILE_TXT, wxString(), wxDefaultPosition,
                                     wxDefaultSize, wxTE_PROCESS_ENTER);
    m_resultFileTxt->SetToolTip("Full path of the file where the batch files append their time stamped PASS / FAIL lines.\n"
                                "Default: result.txt next to the exe. Commands receive it in the variable PCR_RESULT_FILE.");
    auto* resultBrowse = new wxButton(m_mainPanel, windowIDs::ID_RESULT_BROWSE_BT, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    resultBrowse->SetToolTip("Choose another result file");
    resultRow->Add(resultLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, FromDIP(4));
    resultRow->Add(m_resultFileTxt, 1, wxEXPAND | wxRIGHT, FromDIP(4));
    resultRow->Add(resultBrowse, 0, wxEXPAND);
    SetResultFile(AppSettings::getString("resultFile", DefaultResultFile()));

    m_cmdsSizer = new wxBoxSizer(wxVERTICAL);
    m_cmds.reserve(kNrOfCmds);
    for (int i = 0; i < kNrOfCmds; i++) {
        m_cmds.push_back(std::make_unique<cmdgui>(m_mainPanel, i));
        m_cmds.back()->setCounters(wxString::Format("counters of CMD: %d", i + 1));
        if (i == 0) { // the first row shows how a batch file reports its result through the result file
            m_cmds.back()->setCmd("bat_examples\\01_minimal_pass_fail.bat");
            // The [name] tag keeps a PASS written by another command running in parallel from counting.
            m_cmds.back()->setPostVal(":File:::[01_minimal_pass_fail] PASS");
        }
        m_cmdsSizer->Add(m_cmds.back()->getPointer(), 0, wxEXPAND | wxALL, 1);
    }

    m_resultList = new wxListCtrl(m_mainPanel, windowIDs::ID_COMMAND_LIST, wxDefaultPosition, wxDefaultSize,
                                  wxLC_REPORT | wxLC_HRULES | wxLC_VRULES);
    m_resultList->InsertColumn(0, "Timestamp");
    m_resultList->InsertColumn(1, "Information");

    // The rows of the last session (or of the project chosen last time); needs the result list for its messages.
    m_projectPath = AppSettings::getString("projectFile", DefaultProjectFile());
    if (!wxFileExists(m_projectPath)) m_projectPath = DefaultProjectFile(); // the project chosen last time is gone
    bool loaded = false;
    if (!options.projectPath.empty()) {
        wxFileName given(options.projectPath);
        given.MakeAbsolute();
        loaded = LoadProject(given.GetFullPath(), true, !options.run);
        m_projectLoadFailed = !loaded;
    }
    if (!loaded && !options.run && wxFileExists(m_projectPath)) loaded = LoadProject(m_projectPath, true);
    if (!loaded && !options.run && wxFileExists(m_projectPath)) {
        // An unreadable project (damaged, or from a newer version) is never overwritten by the rows of this session.
        if (m_projectPath == DefaultProjectFile()) {
            wxRenameFile(m_projectPath, m_projectPath + ".bad", true);
            AddMessage(get_current_timestamp(), "The unreadable project was kept as " + m_projectPath + ".bad", kFailColour);
        }
        m_projectPath = DefaultProjectFile();
    }
    if (options.repeat >= 0) m_repeatSpin->SetValue(options.repeat);
    UpdateTitle();

    // --- layout ------------------------------------------------------------------------------
    auto* componentsSizer = new wxBoxSizer(wxVERTICAL);
    componentsSizer->Add(runRow, 0, wxEXPAND | wxALL, 1);
    componentsSizer->Add(resultRow, 0, wxEXPAND | wxALL, 1);
    componentsSizer->Add(m_cmdsSizer, 0, wxEXPAND | wxALL, 1);
    componentsSizer->Add(m_resultList, 1, wxEXPAND | wxALL, 1); // the list takes all the remaining space
    m_mainPanel->SetSizer(componentsSizer);

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_mainPanel, 1, wxEXPAND);
    SetSizer(mainSizer);

    // --- events ------------------------------------------------------------------------------
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnLicense, this, windowIDs::ID_LICENSE);
    Bind(wxEVT_MENU, &MainWindow::OnNewBatch, this, windowIDs::ID_NEW_BATCH);
    Bind(wxEVT_MENU, &MainWindow::OnOpenBatch, this, windowIDs::ID_OPEN_BATCH);
    Bind(wxEVT_MENU, &MainWindow::OnOpenExamples, this, windowIDs::ID_OPEN_EXAMPLES);
    Bind(wxEVT_MENU, &MainWindow::OnOpenProject, this, windowIDs::ID_OPEN_PROJECT);
    Bind(wxEVT_MENU, &MainWindow::OnExportCsv, this, windowIDs::ID_EXPORT_CSV);
    Bind(wxEVT_MENU, &MainWindow::OnClearResults, this, windowIDs::ID_CLEAR_RESULTS);
    Bind(wxEVT_MENU, &MainWindow::OnSaveProject, this, windowIDs::ID_SAVE_PROJECT);
    Bind(wxEVT_MENU, &MainWindow::OnSaveProjectAs, this, windowIDs::ID_SAVE_PROJECT_AS);
    Bind(wxEVT_MENU, &MainWindow::OnOpenDataFolder, this, windowIDs::ID_OPEN_DATA_FOLDER);
    Bind(wxEVT_MENU, &MainWindow::OnOpenResultFile, this, windowIDs::ID_OPEN_RESULT_FILE);
    Bind(wxEVT_MENU, &MainWindow::OnSelectResultFile, this, windowIDs::ID_SELECT_RESULT_FILE);
    Bind(wxEVT_BUTTON, &MainWindow::OnSelectResultFile, this, windowIDs::ID_RESULT_BROWSE_BT);
    Bind(wxEVT_TIMER, &MainWindow::OnLicenseTimer, this, windowIDs::ID_LICENSE_TIMER);
    m_resultFileTxt->Bind(wxEVT_TEXT_ENTER, &MainWindow::OnResultFileEntered, this);
    m_resultFileTxt->Bind(wxEVT_KILL_FOCUS, &MainWindow::OnResultFileLostFocus, this);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainWindow::OnEnable, this, windowIDs::ID_ENABLE_EDIT);
    Bind(wxEVT_MENU, &MainWindow::OnDisable, this, windowIDs::ID_DISABLE_EDIT);
    Bind(wxEVT_MENU, &MainWindow::OnStopWaiting, this, windowIDs::ID_STOP_WAITING);
    Bind(wxEVT_MENU, &MainWindow::OnStopRepeat, this, windowIDs::ID_STOP_REPEAT);
    // Command events of the children travel up to the frame.
    Bind(wxEVT_CHECKBOX, &MainWindow::OnGuiEvent, this);
    Bind(wxEVT_BUTTON, &MainWindow::OnButtonEvent, this);
    Bind(wxEVT_THREAD_RESULT, &MainWindow::OnThreadResult, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnClose, this);
    m_resultList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnKeyDown, this);
    m_resultList->Bind(wxEVT_MOTION, &MainWindow::OnMouseMove, this);
    m_resultList->Bind(wxEVT_SIZE, &MainWindow::OnListSize, this);

    m_licenseTimer.Start(60 * 1000); // the day may change while the program runs
    UpdateLicenseStatus();

    if (!options.screenshotsDir.empty()) CallAfter(&MainWindow::TakeScreenshots); // once the window is shown
    else if (options.run) CallAfter(&MainWindow::RunUnattended);

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
        if (cmd.Cmd_sequential_CB->GetValue() &&
            !RequireFeature(LicensePolicy::kFeatureSequential, LicenseDialog::featureName(LicensePolicy::kFeatureSequential))) {
            cmd.setSequential(false); // not licensed: stays parallel
            break;
        }
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
    // While the command runs the same button is "Stop".
    cmdgui& cmd = *m_cmds[static_cast<size_t>(row)];
    if (cmd.getRunning()) {
        if (CommandRunner::stop(row)) AddMessage(get_current_timestamp(), wxString::Format("Stopping CMD %d", row + 1));
        return;
    }
    if (!RequireFeature(LicensePolicy::kFeatureRun, LicenseDialog::featureName(LicensePolicy::kFeatureRun))) return;
    if (UsesResultFile(row) && !EnsureResultFile()) {
        AddMessage(get_current_timestamp(), wxString::Format("CMD %d not started: no result file", row + 1));
        return;
    }
    if (cmd.setRunning(true)) {
        StartThread(cmd.getCmd(), row, 0); // not part of a run of all the rows
    }
    else {
        AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD %d gui to BUSY", row + 1));
    }
}

void MainWindow::OnAbout(wxCommandEvent&)
{
    AboutDialog dialog(this);
    dialog.ShowModal();
    if (dialog.licenseChanged()) UpdateLicenseStatus();
}

void MainWindow::OnLicense(wxCommandEvent&)
{
    ShowLicenseDialog();
}

void MainWindow::ShowLicenseDialog()
{
    LicenseDialog dialog(this);
    dialog.ShowModal();
    UpdateLicenseStatus();
}

void MainWindow::OnLicenseTimer(wxTimerEvent&)
{
    if (Licensing::refreshIfDayChanged()) UpdateLicenseStatus();
}

void MainWindow::UpdateLicenseStatus()
{
    SetStatusText(LicenseDialog::statusBarText(), 1);
}

bool MainWindow::RequireFeature(const char* feature, const wxString& featureName)
{
    if (Licensing::allows(feature)) return true;
    const wxString message = wxString::Format(tr(UVT::LICENSE_FEATURE_NEEDED_FMT), featureName, LicenseDialog::featureBlockedText());
    if (wxMessageBox(message, tr(UVT::LICENSE_TITLE), wxYES_NO | wxICON_INFORMATION, this) == wxYES) ShowLicenseDialog();
    return Licensing::allows(feature); // a license may have been installed meanwhile
}

void MainWindow::OnNewBatch(wxCommandEvent&)
{
    if (!RequireFeature(LicensePolicy::kFeatureEditor, LicenseDialog::featureName(LicensePolicy::kFeatureEditor))) return;
    (new BatchEditorFrame(this))->Show();
}

void MainWindow::OnOpenBatch(wxCommandEvent&)
{
    if (!RequireFeature(LicensePolicy::kFeatureEditor, LicenseDialog::featureName(LicensePolicy::kFeatureEditor))) return;
    const wxString exeDir = wxString(DataDir::exeDirectory().wstring());
    const wxString examples = exeDir + wxFileName::GetPathSeparator() + AppInfo::kExamplesFolderName;
    wxFileDialog dialog(this, "Open batch file", wxDirExists(examples) ? examples : exeDir, wxString(),
                        "Batch files (*.bat;*.cmd)|*.bat;*.cmd|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK) return;
    (new BatchEditorFrame(this, dialog.GetPath()))->Show();
}

void MainWindow::OnOpenExamples(wxCommandEvent&)
{
    const wxString folder = wxString(DataDir::exeDirectory().wstring()) + wxFileName::GetPathSeparator() + AppInfo::kExamplesFolderName;
    if (wxDirExists(folder)) wxLaunchDefaultApplication(folder);
    else wxMessageBox(wxString::Format("The folder %s does not exist.", folder), "Examples", wxOK | wxICON_INFORMATION, this);
}

void MainWindow::OnOpenDataFolder(wxCommandEvent&)
{
    wxLaunchDefaultApplication(wxString(DataDir::path().wstring()));
}

void MainWindow::OnOpenResultFile(wxCommandEvent&)
{
    const wxString path = ResultFilePath();
    if (!wxFileExists(path) && !EnsureResultFile()) return;
    wxLaunchDefaultApplication(ResultFilePath());
}

// ------------------------------------------------------------------------------------------------
// Command line
// ------------------------------------------------------------------------------------------------

void MainWindow::ConsoleLine(const wxString& text)
{
    std::fputs(text.utf8_str(), stdout);
    std::fputs("\n", stdout);
    std::fflush(stdout);
}

bool MainWindow::CaptureWindow(wxWindow* window, const wxString& path)
{
    window->Raise();
    window->Refresh();
    window->Update();
    for (int i = 0; i < 25; i++) { // let the window paint itself completely
        wxYield();
        wxMilliSleep(20);
    }
    // The visible frame, without the invisible resize borders that GetScreenRect() includes on Windows 10 / 11.
    wxRect rect = window->GetScreenRect();
    RECT bounds;
    if (SUCCEEDED(DwmGetWindowAttribute(window->GetHWND(), DWMWA_EXTENDED_FRAME_BOUNDS, &bounds, sizeof(bounds))))
        rect = wxRect(bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top);

    wxBitmap bitmap(rect.width, rect.height);
    {
        wxScreenDC screen;
        wxMemoryDC memory(bitmap);
        memory.Blit(0, 0, rect.width, rect.height, &screen, rect.x, rect.y);
    }
    if (wxImage::FindHandler(wxBITMAP_TYPE_PNG) == nullptr) wxImage::AddHandler(new wxPNGHandler);
    return bitmap.SaveFile(path, wxBITMAP_TYPE_PNG);
}

void MainWindow::TakeScreenshots()
{
    m_unattended = true; // no question, no project or setting written
    wxGetApp().SetExitCode(2);
    wxFileName folder = wxFileName::DirName(m_options.screenshotsDir);
    folder.MakeAbsolute();
    if (!folder.DirExists() && !wxFileName::Mkdir(folder.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
        ConsoleLine("Process Launcher: cannot create " + folder.GetPath());
        Close(true);
        return;
    }
    auto file = [&](const char* name) { return folder.GetPathWithSep() + name; };

    // Demonstration rows, run for real so the list shows genuine results.
    Project::Data demo;
    demo.repeat = 1;
    demo.rows = {
        { true, false, "bat_examples\\01_minimal_pass_fail.bat", ":File:::[01_minimal_pass_fail] PASS", 0 },
        { true, false, "bat_examples\\02_ping_host.bat 127.0.0.1", ":File:::[02_ping_host] PASS", 30 },
        { true, false, "bat_examples\\31_tool_installed.bat where", ":Exit:0", 0 },
        { true, true, "ping -n 2 127.0.0.1", "TTL=", 10 },
        { false, false, "bat_examples\\12_disk_free_space.bat C 5", "PASS", 0 },
    };
    ApplyProject(demo);
    SetTitle(wxString::FromUTF8(GetWindowTitle()) + " - demo.pcr");
    if (Licensing::allows(LicensePolicy::kFeatureRun)) {
        wxCommandEvent event;
        onRunCommand(event);
        if (m_closing) return;
    }

    bool ok = CaptureWindow(this, file("main-window.png"));

    auto* editor = new BatchEditorFrame(this);
    editor->Show();
    ok = CaptureWindow(editor, file("batch-editor.png")) && ok;
    editor->Close(true);

    auto* about = new AboutDialog(this);
    about->Show();
    ok = CaptureWindow(about, file("about.png")) && ok;
    about->Destroy();

    ConsoleLine(wxString::Format("Process Launcher: screenshots %s in %s", ok ? "saved" : "NOT all saved", folder.GetPath()));
    wxGetApp().SetExitCode(ok ? 0 : 2);
    Close(true);
}

void MainWindow::RunUnattended()
{
    wxGetApp().SetExitCode(2); // until the run proves otherwise
    if (m_projectLoadFailed) {
        ConsoleLine("Process Launcher: the project could not be read: " + m_options.projectPath);
    }
    else if (!Licensing::allows(LicensePolicy::kFeatureRun)) {
        ConsoleLine("Process Launcher: running commands needs a license: " + LicenseDialog::statusBarText());
        Log::error("Command line run refused: no license for running commands.");
    }
    else {
        wxCommandEvent event;
        onRunCommand(event);
        if (m_closing) return; // closed by the operator meanwhile: exit code 2
        if (!m_options.csvPath.empty()) {
            wxFileName csv(m_options.csvPath);
            csv.MakeAbsolute();
            ExportCsv(csv.GetFullPath(), true);
        }
        const int code = m_lastRunCount == 0 ? 2 : (m_lastRunFailures > 0 ? 1 : 0);
        wxGetApp().SetExitCode(code);
        const wxString summary = wxString::Format("Process Launcher: %d run(s), %d PASS, %d FAIL - exit code %d",
                                                  m_lastRunCount, m_lastRunPasses, m_lastRunFailures, code);
        ConsoleLine(summary);
        Log::info(std::string(summary.utf8_str()));
    }
    Close(true);
}

// ------------------------------------------------------------------------------------------------
// Results
// ------------------------------------------------------------------------------------------------

wxChar MainWindow::ListSeparator()
{
    wchar_t separator[8] = {};
    if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SLIST, separator, 8) > 0 && separator[0] != L'\0' && separator[1] == L'\0')
        return separator[0];
    return ',';
}

bool MainWindow::ExportCsv(const wxString& path, bool quiet)
{
    wxString error;
    if (!ResultCsv::write(path, m_records, ListSeparator(), error)) {
        Log::error(std::string(error.utf8_str()));
        if (quiet) AddMessage(get_current_timestamp(), error, kFailColour);
        else wxMessageBox(error, "Export results", wxOK | wxICON_ERROR, this);
        return false;
    }
    AddMessage(get_current_timestamp(), wxString::Format("%d result(s) exported to %s", static_cast<int>(m_records.size()), path));
    return true;
}

void MainWindow::OnExportCsv(wxCommandEvent&)
{
    if (m_records.empty()) {
        wxMessageBox("There are no results to export yet.", "Export results", wxOK | wxICON_INFORMATION, this);
        return;
    }
    const wxString name = wxDateTime::Now().Format("results_%Y-%m-%d_%H%M%S.csv");
    wxFileDialog dialog(this, "Export results", wxFileName(ResultFilePath()).GetPath(), name,
                        "CSV files (*.csv)|*.csv|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK) return;
    ExportCsv(dialog.GetPath(), false);
}

void MainWindow::OnClearResults(wxCommandEvent&)
{
    if (!m_records.empty() &&
        wxMessageBox("Empty the result list? The results not exported are lost.", "Clear results", wxYES_NO | wxICON_QUESTION, this) != wxYES)
        return;
    m_resultList->DeleteAllItems();
    m_records.clear();
    m_lastTipItem = -1;
}

// ------------------------------------------------------------------------------------------------
// Project
// ------------------------------------------------------------------------------------------------

wxString MainWindow::DefaultProjectFile()
{
    return wxString(DataDir::file(std::string("commands.") + Project::kExtension.utf8_string()).wstring());
}

Project::Data MainWindow::CollectProject()
{
    Project::Data data;
    data.resultFile = ResultFilePath();
    data.repeat = m_repeatSpin->GetValue();
    for (auto& cmd : m_cmds) {
        data.rows.push_back({ cmd->isActive(), cmd->isSequential(), cmd->getCmd(), cmd->getPositiveVal(), cmd->getTimeout() });
    }
    return data;
}

void MainWindow::ApplyProject(const Project::Data& data)
{
    const bool singleAllowed = Licensing::allows(LicensePolicy::kFeatureSequential);
    for (size_t i = 0; i < m_cmds.size(); i++) {
        cmdgui& cmd = *m_cmds[i];
        const Project::Row row = i < data.rows.size() ? data.rows[i] : Project::Row{ false, false, wxString(), wxString(), 0 };
        cmd.setCmd(row.command);
        cmd.setPostVal(row.expected);
        cmd.setTimeout(row.timeout);
        cmd.setSequential(row.single && singleAllowed); // without the license feature the row stays Parallel
        if (row.active) cmd.enable();
        else cmd.disable();
        if (m_userEditLock) cmd.disableEditables();
    }
    if (data.rows.size() > m_cmds.size()) {
        AddMessage(get_current_timestamp(), wxString::Format("The project has %d rows, only the first %d are shown",
                                                             static_cast<int>(data.rows.size()), static_cast<int>(m_cmds.size())), kFailColour);
    }
    if (!data.resultFile.empty()) SetResultFile(data.resultFile);
    m_repeat = data.repeat;
    m_repeatSpin->SetValue(m_repeat);
}

bool MainWindow::LoadProject(const wxString& path, bool quiet, bool remember)
{
    Project::Data data;
    wxString error;
    if (!Project::load(path, data, error)) {
        Log::error(std::string(error.utf8_str()));
        if (quiet) AddMessage(get_current_timestamp(), error, kFailColour);
        else wxMessageBox(error, "Open project", wxOK | wxICON_ERROR, this);
        return false;
    }
    ApplyProject(data);
    m_projectPath = path;
    if (remember) AppSettings::set("projectFile", path);
    UpdateTitle();
    Log::info("Project: " + std::string(path.utf8_str()));
    return true;
}

bool MainWindow::SaveProject(const wxString& path, bool quiet)
{
    wxString error;
    if (!Project::save(path, CollectProject(), error)) {
        Log::error(std::string(error.utf8_str()));
        if (quiet) AddMessage(get_current_timestamp(), error, kFailColour);
        else wxMessageBox(error, "Save project", wxOK | wxICON_ERROR, this);
        return false;
    }
    if (path != m_projectPath) {
        m_projectPath = path;
        AppSettings::set("projectFile", path);
        UpdateTitle();
    }
    return true;
}

void MainWindow::UpdateTitle()
{
    SetTitle(wxString::FromUTF8(GetWindowTitle()) + " - " + wxFileName(m_projectPath).GetFullName());
}

void MainWindow::OnOpenProject(wxCommandEvent&)
{
    if (m_runAllInProgress || AnyCommandRunning()) {
        wxMessageBox("Wait for the commands to end before opening another project.", "Open project", wxOK | wxICON_INFORMATION, this);
        return;
    }
    const wxFileName current(m_projectPath);
    wxFileDialog dialog(this, "Open project", current.GetPath(), wxString(),
                        "Process Launcher projects (*.pcr)|*.pcr|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK) return;
    SaveProject(m_projectPath, true); // the rows of the project being left are kept
    LoadProject(dialog.GetPath(), false);
}

void MainWindow::OnSaveProject(wxCommandEvent&)
{
    if (SaveProject(m_projectPath, false)) AddMessage(get_current_timestamp(), "Project saved: " + m_projectPath);
}

void MainWindow::OnSaveProjectAs(wxCommandEvent&)
{
    const wxFileName current(m_projectPath);
    wxFileDialog dialog(this, "Save project as", current.GetPath(), current.GetFullName(),
                        "Process Launcher projects (*.pcr)|*.pcr|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK) return;
    wxString path = dialog.GetPath();
    if (wxFileName(path).GetExt().empty()) path += "." + Project::kExtension;
    if (SaveProject(path, false)) AddMessage(get_current_timestamp(), "Project saved: " + path);
}

// ------------------------------------------------------------------------------------------------
// Result file
// ------------------------------------------------------------------------------------------------

wxString MainWindow::DefaultResultFile()
{
    return wxString(DataDir::exeDirectory().wstring()) + wxFileName::GetPathSeparator() + AppInfo::kDefaultResultFileName;
}

wxString MainWindow::ResultFilePath() const
{
    wxString path = m_resultFileTxt->GetValue();
    path.Trim().Trim(false);
    return path.empty() ? DefaultResultFile() : path;
}

void MainWindow::SetResultFile(const wxString& path)
{
    wxString full = path;
    full.Trim().Trim(false);
    if (full.empty()) full = DefaultResultFile();
    wxFileName name(full);
    name.MakeAbsolute(wxString(DataDir::exeDirectory().wstring())); // a relative path is relative to the exe
    full = name.GetFullPath();

    m_resultFileTxt->ChangeValue(full);
    AppSettings::set("resultFile", full);
    // Every command started from now on finds the file in its environment.
    wxSetEnv("PCR_RESULT_FILE", full);
    wxSetEnv("PCR_APP_DIR", wxString(DataDir::exeDirectory().wstring()));
    Log::info("Result file: " + std::string(full.utf8_str()));
}

void MainWindow::OnResultFileEntered(wxCommandEvent&)
{
    SetResultFile(m_resultFileTxt->GetValue());
}

void MainWindow::OnResultFileLostFocus(wxFocusEvent& event)
{
    SetResultFile(m_resultFileTxt->GetValue());
    event.Skip();
}

void MainWindow::OnSelectResultFile(wxCommandEvent&)
{
    const wxFileName current(ResultFilePath());
    wxFileDialog dialog(this, "Select the result file (an existing file, or a name to create)", current.GetPath(), current.GetFullName(),
                        "Text files (*.txt;*.log)|*.txt;*.log|All files (*.*)|*.*", wxFD_SAVE);
    // wxFD_OVERWRITE_PROMPT is left out on purpose: choosing an existing file is fine, it is only appended to.
    if (dialog.ShowModal() != wxID_OK) return;
    SetResultFile(dialog.GetPath());
    if (!wxFileExists(ResultFilePath())) CreateResultFile(ResultFilePath());
}

bool MainWindow::CreateResultFile(const wxString& path)
{
    const wxFileName name(path);
    if (!name.DirExists() && !wxFileName::Mkdir(name.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
        wxMessageBox(wxString::Format("Cannot create the folder %s", name.GetPath()), "Result file", wxOK | wxICON_ERROR, this);
        return false;
    }
    // Appending (never truncating) and one line with the time, like the batch files do.
    wxFile file(path, wxFile::write_append);
    const wxScopedCharBuffer line = (get_current_timestamp() + " [" + AppInfo::kName + "] result file created\r\n").utf8_str();
    if (!file.IsOpened() || file.Write(line.data(), line.length()) != line.length()) {
        wxMessageBox(wxString::Format("Cannot write %s", path), "Result file", wxOK | wxICON_ERROR, this);
        return false;
    }
    AddMessage(get_current_timestamp(), wxString::Format("Result file created: %s", path));
    return true;
}

bool MainWindow::EnsureResultFile()
{
    if (wxFileExists(ResultFilePath())) return true;
    if (m_unattended) return CreateResultFile(ResultFilePath()); // no questions in a command line run
    if (m_askingResultFile) return false; // the question is already open (a result arrived while it was shown)
    m_askingResultFile = true;

    bool ok = false;
    const wxString expected = ResultFilePath();
    wxMessageDialog question(this,
        wxString::Format("The result file\n\n%s\n\ndoesn't exist.\nThe batch files append their PASS / FAIL lines to it.", expected),
        "Result file", wxYES_NO | wxCANCEL | wxICON_QUESTION);
    question.SetYesNoCancelLabels("Create here", "Choose location...", "Cancel");
    const int answer = question.ShowModal();
    if (answer == wxID_YES) {
        ok = CreateResultFile(expected);
    }
    else if (answer == wxID_NO) {
        const wxFileName current(expected);
        wxFileDialog dialog(this, "Where to save the result file", current.GetPath(), current.GetFullName(),
                            "Text files (*.txt;*.log)|*.txt;*.log|All files (*.*)|*.*", wxFD_SAVE);
        if (dialog.ShowModal() == wxID_OK) {
            SetResultFile(dialog.GetPath());
            ok = wxFileExists(ResultFilePath()) || CreateResultFile(ResultFilePath());
        }
    }
    m_askingResultFile = false;
    return ok;
}

bool MainWindow::UsesResultFile(int commandIndex)
{
    const ResultCheck::Spec spec = ResultCheck::parse(m_cmds[static_cast<size_t>(commandIndex)]->getPositiveVal());
    return spec.isFile && spec.valid && spec.usesResultFile;
}

void MainWindow::OnEnable(wxCommandEvent&)
{
    m_userEditLock = false;
    EnableCmds();
}

void MainWindow::EnableCmds()
{
    for (auto& cmd : m_cmds) cmd->enableEditables();
}

void MainWindow::OnDisable(wxCommandEvent&)
{
    m_userEditLock = true;
    DisableCmds();
}

void MainWindow::UnlockWhenIdle()
{
    if (!m_userEditLock && !m_runAllInProgress && !m_closing && !AnyCommandRunning()) EnableCmds();
}

void MainWindow::DisableCmds()
{
    for (auto& cmd : m_cmds) cmd->disableEditables();
}

void MainWindow::OnStopWaiting(wxCommandEvent&)
{
    if (!m_blocking) return;
    AddMessage(get_current_timestamp(), m_blockingCommandIndex < 0 ? wxString("Stopped waiting for the running commands")
                                                                   : wxString::Format("Stopped waiting for CMD %d", m_blockingCommandIndex + 1));
    m_blocking = false;
    m_blockingCommandIndex = -1;
}

void MainWindow::OnStopRepeat(wxCommandEvent&)
{
    if (!m_runAllInProgress || m_stopRepeat) return;
    m_stopRepeat = true;
    AddMessage(get_current_timestamp(), "Repeating stops after the current run");
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
    if (m_closing) return; // already closing, waiting for onRunCommand() to return

    // The batch editors are children of this window: give them the chance to save their changes.
    for (wxWindow* child : GetChildren()) {
        auto* editor = dynamic_cast<BatchEditorFrame*>(child);
        if (editor != nullptr && !editor->IsBeingDeleted() && !editor->Close(!event.CanVeto())) {
            event.Veto();
            return;
        }
    }

    if (event.CanVeto() && AnyCommandRunning()) {
        if (wxMessageBox("Are you sure you want to quit, unfinished CMDs will be terminated!!!",
                         "Request to quit application", wxICON_QUESTION | wxYES_NO, this) != wxYES) {
            // Like the veto power in the UN Security Council: it keeps the window open.
            event.Veto();
            return;
        }
    }
    if (!m_unattended) SaveProject(m_projectPath, true); // the rows are there again at the next start
    m_closing = true;
    m_blocking = false; // releases WaitWhileBlocking()
    {
        std::lock_guard<std::mutex> lock(m_sink->mutex);
        m_sink->target = nullptr; // no result is posted to this window from now on
    }
    CommandRunner::stopAll();

    // onRunCommand() may be waiting for a single command further up this call stack (wxYield): the
    // window must outlive it, so it destroys the window itself when it returns.
    if (m_runAllInProgress) {
        Hide();
        return;
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

bool MainWindow::StartSingleCommand(int i, bool ask)
{
    cmdgui& cmd = *m_cmds[static_cast<size_t>(i)];

    // The operator may wait for the previous commands in case this one depends on them.
    if (ask) wxMessageBox("Next command will be executed in single mode, the next ones will wait for this to complete\n"
                 "Make sure it does not depend on previous commands and hit OK when ready",
                 "ATTENTION Blocking thread", wxOK | wxICON_INFORMATION, this);
    if (m_closing) return false;

    // Every earlier row counts, not only the one just above (an OFF row keeps its "not failed" state).
    wxString failed;
    for (int previous = 0; previous < i; previous++) {
        if (!m_cmds[static_cast<size_t>(previous)]->getResult()) failed += wxString::Format(" %d", previous + 1);
    }
    if (!failed.empty() && !ask) {
        // repeated or unattended runs go on: only the list records it
        AddMessage(get_current_timestamp(), wxString::Format("CMD %d started although CMD%s failed", i + 1, failed));
    }
    else if (!failed.empty() &&
        wxMessageBox("Found at least one previous executed CMD with fail result (CMD" + failed + ")\nDo you still want to proceed?",
                     "ATTENTION Previous Failure", wxYES_NO, this) != wxYES) {
        // Keep track of the decision and leave the row idle.
        AddMessage(get_current_timestamp(), wxString::Format("Skipped CMD: %d", i + 1));
        cmd.setRunning(false);
        return false;
    }

    // The flags are set before the thread starts, so the result can never arrive before them.
    m_blocking = true;
    m_blockingCommandIndex = i;
    StartThread(cmd.getCmd(), i, m_runTag);
    return true;
}

void MainWindow::onRunCommand(wxCommandEvent&)
{
    // The loop below waits (yielding to the GUI) for the commands: the button could be pressed again meanwhile.
    if (m_runAllInProgress) {
        AddMessage(get_current_timestamp(), "A run of the commands is already in progress");
        return;
    }

    if (!RequireFeature(LicensePolicy::kFeatureRun, LicenseDialog::featureName(LicensePolicy::kFeatureRun))) return;

    bool foundRunning = false;
    for (size_t i = 0; i < m_cmds.size(); i++) {
        if (m_cmds[i]->getRunning()) {
            AddMessage(get_current_timestamp(), wxString::Format("Found cmd %d active", static_cast<int>(i) + 1));
            foundRunning = true;
        }
    }

    if (foundRunning && !m_unattended &&
        wxMessageBox("Found at least one thread still active:\nDo you want to launch all the not running commands?",
                     " PCR - Confirmation Request", wxYES_NO, this) != wxYES) {
        return;
    }

    m_repeat = m_repeatSpin->GetValue();
    if (!m_unattended) SaveProject(m_projectPath, true); // what runs is what is saved (a command line run changes no file)
    DisableCmds(); // the commands must not be edited while they run (unlocked again by UnlockWhenIdle())
    m_runAllInProgress = true;
    m_stopRepeat = false;

    const int total = m_repeat; // 0 = until Settings > Stop repeating
    int runs = 0, passTotal = 0, failTotal = 0;
    for (int iteration = 1; (total == 0 || iteration <= total) && !m_closing && !m_stopRepeat; iteration++) {
        m_runTag++;
        m_tagPass = 0;
        m_tagFail = 0;
        m_tagFailedRows.clear();
        const int started = RunRowsOnce(iteration == 1);
        if (m_closing) break;
        if (started == 0) {
            AddMessage(get_current_timestamp(), "No command was started (no row ON, or all still running)", kFailColour);
            break;
        }
        WaitWhileAnyRunning();
        if (m_closing) break;

        runs++;
        passTotal += m_tagPass;
        failTotal += m_tagFail;
        wxString failedRows;
        for (int row : m_tagFailedRows) failedRows += wxString::Format(" %d", row + 1);
        const wxString of = total == 0 ? wxString::Format("%d", iteration) : wxString::Format("%d/%d", iteration, total);
        AddMessage(get_current_timestamp(),
                   wxString::Format("Run %s ended: %d PASS, %d FAIL", of, m_tagPass, m_tagFail) +
                       (failedRows.empty() ? wxString() : " (CMD" + failedRows + ")"),
                   m_tagFail == 0 ? kPassColour : kFailColour);
        Log::info(std::string(wxString::Format("Run %s: %d PASS, %d FAIL", of, m_tagPass, m_tagFail).utf8_str()));
    }
    if (runs > 1 || m_stopRepeat) {
        AddMessage(get_current_timestamp(),
                   wxString::Format("Repeat ended after %d run(s)%s: %d PASS, %d FAIL", runs, m_stopRepeat ? " (stopped)" : "",
                                    passTotal, failTotal),
                   failTotal == 0 ? kPassColour : kFailColour);
    }
    m_lastRunFailures = failTotal;
    m_lastRunPasses = passTotal;
    m_lastRunCount = runs;

    m_runAllInProgress = false;
    if (m_closing) { // the window was closed while the commands were waited for
        Destroy();
        return;
    }
    UnlockWhenIdle();
}

int MainWindow::RunRowsOnce(bool firstRun)
{
    int started = 0;
    for (auto& cmd : m_cmds) cmd->setResult(true); // every command starts as "not failed"

    for (int i = 0; i < kNrOfCmds && !m_closing; i++) {
        cmdgui& cmd = *m_cmds[static_cast<size_t>(i)];

        if (cmd.getRunning()) {
            // Still running from a previous run, only trace that it was skipped.
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD %d already active", i + 1));
            continue;
        }
        if (!cmd.isActive()) continue; // OFF: not to be run, visible from the GUI

        // A command that reports through the result file needs it: ask now, not after it has failed.
        if (UsesResultFile(i) && !EnsureResultFile()) {
            AddMessage(get_current_timestamp(), wxString::Format("CMD %d not started: no result file", i + 1));
            continue;
        }

        if (!cmd.setRunning(true)) {
            // Never seen, but possible if a widget has a problem.
            AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD %d gui to BUSY", i + 1));
            continue;
        }

        if (cmd.isSequential()) {
            if (StartSingleCommand(i, firstRun && !m_unattended)) {
                started++;
                WaitWhileBlocking();
            }
        }
        else {
            StartThread(cmd.getCmd(), i, m_runTag);
            started++;
        }
    }
    return started;
}

void MainWindow::WaitWhileAnyRunning()
{
    // Like WaitWhileBlocking(), for every command; "Stop waiting" (Ctrl-B) also ends this wait.
    m_blocking = true;
    m_blockingCommandIndex = -1;
    while (m_blocking && !m_closing && AnyCommandRunning()) {
        wxYield();
        wxMilliSleep(10);
    }
    m_blocking = false;
}

void MainWindow::StartThread(const wxString& input, int commandIndex, int runTag)
{
    // Everything the worker needs is copied here, on the main thread: it never reads a widget.
    const std::shared_ptr<EventSink> sink = m_sink;
    const wxString command = input;
    const wxString expected = m_cmds[static_cast<size_t>(commandIndex)]->getPositiveVal();
    const wxString resultFile = ResultFilePath();
    // Only what the command appends to the result file counts: an old PASS must not make it pass.
    const wxFileOffset resultFileOffset = ResultCheck::fileSize(resultFile);
    const wxString workingDirectory = wxString(DataDir::exeDirectory().wstring());
    const long timeoutMs = 1000L * m_cmds[static_cast<size_t>(commandIndex)]->getTimeout();
    // Each command gets its own environment: the result file, the folder of the exe and its row number.
    const std::vector<std::pair<wxString, wxString>> environment = {
        { "PCR_RESULT_FILE", resultFile },
        { "PCR_APP_DIR", workingDirectory },
        { "PCR_CMD_ID", wxString::Format("%d", commandIndex + 1) },
    };

    std::thread([sink, command, expected, resultFile, resultFileOffset, workingDirectory, environment, timeoutMs, commandIndex, runTag]() {
        CommandOutcome outcome;
        outcome.index = commandIndex;
        outcome.runTag = runTag;
        outcome.command = command;
        outcome.expected = expected;
        const CommandRunner::Result run = CommandRunner::run(command, workingDirectory, environment, timeoutMs, commandIndex);
        outcome.output = run.output;
        outcome.exitCode = run.exitCode;
        outcome.durationMs = run.durationMs;
        wxString missingResultFile;
        outcome.pass = ResultCheck::evaluate(expected, outcome.output, run.exitCode, commandIndex + 1, resultFile, resultFileOffset,
                                             outcome.note, missingResultFile);
        if (run.timedOut || run.stopped) {
            outcome.pass = false; // a terminated command never passes
            outcome.note = run.timedOut ? wxString::Format("CMD %d terminated: time limit of %ld s expired", commandIndex + 1, timeoutMs / 1000)
                                        : wxString::Format("CMD %d terminated: stopped", commandIndex + 1);
        }

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

    ResultCsv::Record record;
    record.timestamp = get_current_timestamp();
    record.run = outcome.runTag;
    record.row = outcome.index + 1;
    record.command = outcome.command;
    record.expected = outcome.expected;
    record.pass = outcome.pass;
    record.exitCode = outcome.exitCode;
    record.durationMs = outcome.durationMs;
    record.note = outcome.note;
    record.output = outcome.output;
    m_records.push_back(record);
    if (outcome.runTag != 0 && outcome.runTag == m_runTag) { // counted in the summary of the current run
        if (outcome.pass) m_tagPass++;
        else {
            m_tagFail++;
            m_tagFailedRows.push_back(outcome.index);
        }
    }
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
    AddMessage(get_current_timestamp(), wxString::Format("Cmd %d %s (exit code %ld, %.1f s), result is: %s", outcome.index + 1, status,
                                                         outcome.exitCode, outcome.durationMs / 1000.0, output),
               colour);
    UnlockWhenIdle();
}
