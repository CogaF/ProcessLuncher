/*!
 * \file BatchEditorFrame.cpp
 * \brief Implementation of BatchEditorFrame.h.
 */

#include "BatchEditorFrame.h"

#include <algorithm>

#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/tooltip.h>

#include "AppInfo.h"
#include "AppSettings.h"
#include "BatHighlighter.h"
#include "DataDir.h"

namespace {

/*! \brief Identifiers of the toolbar buttons. */
enum
{
    ID_NEW = wxID_HIGHEST + 500, /*!< new file. */
    ID_OPEN,                     /*!< open file. */
    ID_SAVE,                     /*!< save. */
    ID_SAVEAS,                   /*!< save as. */
    ID_BIGGER,                   /*!< larger text. */
    ID_SMALLER,                  /*!< smaller text. */
    ID_HIGHLIGHT_TIMER,          /*!< timer of the delayed highlighting. */
    ID_FILTER                    /*!< filter text box. */
};

/*! \brief Smallest allowed text size (points). */
constexpr int kMinFontSize = 6;
/*! \brief Largest allowed text size (points). */
constexpr int kMaxFontSize = 48;

/*! \brief Colours of the syntax elements, light enough for the dark background. */
const wxColour kPlainColour(230, 230, 230);

/*!
 * \brief Text attributes of a syntax element.
 * \param kind the element.
 * \param fontSize size in points.
 * \return the attributes: colour, weight, slant, size and the Courier New face.
 */
wxTextAttr attributesFor(BatToken kind, int fontSize)
{
    wxTextAttr attr;
    attr.SetFontFaceName("Courier New");
    attr.SetFontFamily(wxFONTFAMILY_TELETYPE);
    attr.SetFontSize(fontSize);
    attr.SetFontWeight(wxFONTWEIGHT_NORMAL);
    attr.SetFontStyle(wxFONTSTYLE_NORMAL);
    switch (kind) {
    case BatToken::Command:  attr.SetTextColour(wxColour(100, 180, 255)); attr.SetFontWeight(wxFONTWEIGHT_BOLD); break;
    case BatToken::Flow:     attr.SetTextColour(wxColour(255, 170, 60));  attr.SetFontWeight(wxFONTWEIGHT_BOLD); break;
    case BatToken::Comment:  attr.SetTextColour(wxColour(110, 200, 110)); attr.SetFontStyle(wxFONTSTYLE_ITALIC); break;
    case BatToken::Label:    attr.SetTextColour(wxColour(235, 225, 100)); attr.SetFontWeight(wxFONTWEIGHT_BOLD); break;
    case BatToken::Variable: attr.SetTextColour(wxColour(80, 220, 220)); break;
    case BatToken::String:   attr.SetTextColour(wxColour(235, 150, 130)); break;
    case BatToken::Operator: attr.SetTextColour(wxColour(255, 110, 110)); break;
    case BatToken::Switch:   attr.SetTextColour(wxColour(190, 150, 255)); break;
    case BatToken::Pass:     attr.SetTextColour(wxColour(120, 230, 120)); attr.SetFontWeight(wxFONTWEIGHT_BOLD); break;
    case BatToken::Fail:     attr.SetTextColour(wxColour(255, 110, 110)); attr.SetFontWeight(wxFONTWEIGHT_BOLD); break;
    }
    return attr;
}

/*! \brief Attributes of plain text. */
wxTextAttr plainAttributes(int fontSize)
{
    wxTextAttr attr;
    attr.SetFontFaceName("Courier New");
    attr.SetFontFamily(wxFONTFAMILY_TELETYPE);
    attr.SetFontSize(fontSize);
    attr.SetFontWeight(wxFONTWEIGHT_NORMAL);
    attr.SetFontStyle(wxFONTSTYLE_NORMAL);
    attr.SetTextColour(kPlainColour);
    return attr;
}

/*! \brief Initial content of a new batch file: the skeleton that reports PASS / FAIL and logs to the result file. */
wxString newFileTemplate()
{
    return wxString::FromUTF8(
        "@echo off\n"
        "setlocal\n"
        "rem ---- result file given by Process Launcher (or result.txt next to this file when run by hand)\n"
        "if not defined PCR_RESULT_FILE set \"PCR_RESULT_FILE=%~dp0result.txt\"\n"
        "set \"RESULT_FILE=%PCR_RESULT_FILE%\"\n"
        "set \"TEST_NAME=%~n0\"\n"
        "\n"
        "rem ---- the test: put the command to check here\n"
        "ping -n 1 127.0.0.1 >nul\n"
        "\n"
        "if errorlevel 1 (\n"
        "    call :log FAIL ping returned %errorlevel%\n"
        "    exit /b 1\n"
        ")\n"
        "call :log PASS ping answered\n"
        "exit /b 0\n"
        "\n"
        ":log\n"
        ">>\"%RESULT_FILE%\" echo %date% %time:~0,8% [%TEST_NAME%] %*\n"
        "echo %*\n"
        "goto :eof\n");
}

} // namespace

BatchEditorFrame::BatchEditorFrame(wxWindow* parent, const wxString& path)
    : wxFrame(parent, wxID_ANY, "Batch file editor", wxDefaultPosition, FromDIP(wxSize(1150, 720))),
      m_timer(this, ID_HIGHLIGHT_TIMER)
{
    m_fontSize = std::clamp(AppSettings::getInt("editorFontSize", 11), kMinFontSize, kMaxFontSize);
    SetMinSize(FromDIP(wxSize(700, 400)));

    CreateStatusBar(3);
    const int widths[3] = { -1, FromDIP(140), FromDIP(120) };
    SetStatusWidths(3, widths);

    auto* root = new wxPanel(this);
    auto* rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(BuildToolbar(root), 0, wxEXPAND | wxALL, FromDIP(2));

    auto* splitter = new wxSplitterWindow(root, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
    splitter->SetMinimumPaneSize(FromDIP(180));

    m_text = new wxTextCtrl(splitter, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize,
                            wxTE_MULTILINE | wxTE_RICH2 | wxHSCROLL | wxTE_PROCESS_TAB | wxTE_NOHIDESEL);
    wxWindow* pane = BuildPane(splitter);
    splitter->SplitVertically(m_text, pane, -FromDIP(360)); // the pane keeps its width when the window is resized
    splitter->SetSashGravity(1.0);
    rootSizer->Add(splitter, 1, wxEXPAND);
    root->SetSizer(rootSizer);

    // --- events
    Bind(wxEVT_BUTTON, &BatchEditorFrame::OnNew, this, ID_NEW);
    Bind(wxEVT_BUTTON, &BatchEditorFrame::OnOpen, this, ID_OPEN);
    Bind(wxEVT_BUTTON, &BatchEditorFrame::OnSave, this, ID_SAVE);
    Bind(wxEVT_BUTTON, &BatchEditorFrame::OnSaveAs, this, ID_SAVEAS);
    Bind(wxEVT_BUTTON, &BatchEditorFrame::OnBigger, this, ID_BIGGER);
    Bind(wxEVT_BUTTON, &BatchEditorFrame::OnSmaller, this, ID_SMALLER);
    Bind(wxEVT_CLOSE_WINDOW, &BatchEditorFrame::OnClose, this);
    Bind(wxEVT_TIMER, &BatchEditorFrame::OnHighlightTimer, this, ID_HIGHLIGHT_TIMER);
    m_text->Bind(wxEVT_TEXT, &BatchEditorFrame::OnTextChanged, this);
    m_text->Bind(wxEVT_KEY_UP, &BatchEditorFrame::OnCaretMoved, this);
    m_text->Bind(wxEVT_LEFT_UP, &BatchEditorFrame::OnCaretMoved, this);
    m_text->Bind(wxEVT_MOUSEWHEEL, &BatchEditorFrame::OnMouseWheel, this);

    // Keyboard shortcuts.
    wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_CTRL, 'S', ID_SAVE),
        wxAcceleratorEntry(wxACCEL_CTRL, 'O', ID_OPEN),
        wxAcceleratorEntry(wxACCEL_CTRL, 'N', ID_NEW),
        wxAcceleratorEntry(wxACCEL_CTRL, '+', ID_BIGGER),
        wxAcceleratorEntry(wxACCEL_CTRL, WXK_NUMPAD_ADD, ID_BIGGER),
        wxAcceleratorEntry(wxACCEL_CTRL, '-', ID_SMALLER),
        wxAcceleratorEntry(wxACCEL_CTRL, WXK_NUMPAD_SUBTRACT, ID_SMALLER),
    };
    SetAcceleratorTable(wxAcceleratorTable(static_cast<int>(sizeof(entries) / sizeof(entries[0])), entries));
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) {
        wxCommandEvent forward(wxEVT_BUTTON, e.GetId());
        ProcessWindowEvent(forward);
    });

    if (!path.empty()) {
        if (!LoadFile(path)) m_text->SetValue(wxString());
    }
    else {
        m_busy = true;
        m_text->SetValue(newFileTemplate());
        m_busy = false;
        m_dirty = false;
    }
    ApplyBaseStyle();
    Rehighlight();
    m_text->SetInsertionPoint(0);
    m_text->ShowPosition(0);
    UpdateTitle();
    UpdatePosition();
    CentreOnParent();
}

wxWindow* BatchEditorFrame::BuildToolbar(wxWindow* parent)
{
    auto* bar = new wxPanel(parent);
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    auto add = [&](int id, const wxString& label, const wxString& hint) {
        auto* b = new wxButton(bar, id, label, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        b->SetToolTip(hint);
        sizer->Add(b, 0, wxALL, FromDIP(2));
        return b;
    };
    add(ID_NEW, "New", "New batch file from the PASS / FAIL skeleton (Ctrl+N)");
    add(ID_OPEN, "Open...", "Open a batch file (Ctrl+O)");
    add(ID_SAVE, "Save", "Save (Ctrl+S)");
    add(ID_SAVEAS, "Save as...", "Save with another name");
    sizer->AddSpacer(FromDIP(16));
    add(ID_SMALLER, "A-", "Smaller text (Ctrl+-, or Ctrl + mouse wheel)");
    add(ID_BIGGER, "A+", "Larger text (Ctrl++, or Ctrl + mouse wheel)");
    m_sizeLabel = new wxStaticText(bar, wxID_ANY, wxString::Format("%d pt", m_fontSize));
    sizer->Add(m_sizeLabel, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(6));
    sizer->AddStretchSpacer();
    auto* hint = new wxStaticText(bar, wxID_ANY, "Click a command to insert it - hover for the explanation - right click for details and an example");
    sizer->Add(hint, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(6));
    bar->SetSizer(sizer);
    return bar;
}

wxWindow* BatchEditorFrame::BuildPane(wxWindow* parent)
{
    auto* container = new wxPanel(parent);
    auto* containerSizer = new wxBoxSizer(wxVERTICAL);

    m_filter = new wxTextCtrl(container, ID_FILTER, wxString(), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_filter->SetHint("Filter the commands...");
    m_filter->Bind(wxEVT_TEXT, [this](wxCommandEvent&) { ApplyFilter(); });
    containerSizer->Add(m_filter, 0, wxEXPAND | wxALL, FromDIP(3));

    m_pane = new wxScrolledWindow(container, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    m_pane->SetScrollRate(0, FromDIP(12));
    m_paneSizer = new wxBoxSizer(wxVERTICAL);

    m_categoryNames = BatCommands::categories();
    size_t currentCategory = static_cast<size_t>(-1);
    for (const BatCommand& command : BatCommands::all()) {
        const wxString category = wxString::FromUTF8(command.category);
        const size_t categoryIndex = static_cast<size_t>(
            std::find(m_categoryNames.begin(), m_categoryNames.end(), category) - m_categoryNames.begin());
        if (categoryIndex != currentCategory) {
            currentCategory = categoryIndex;
            auto* header = new wxStaticText(m_pane, wxID_ANY, category);
            header->SetFont(header->GetFont().Bold().Larger());
            m_paneSizer->Add(header, 0, wxLEFT | wxRIGHT | wxTOP, FromDIP(6));
            m_items.push_back({ header, true, wxString(), categoryIndex });
        }
        auto* button = new wxButton(m_pane, wxID_ANY, wxControl::EscapeMnemonics(wxString::FromUTF8(command.name)),
                                    wxDefaultPosition, wxDefaultSize, wxBU_LEFT);
        WireButton(button, command);
        m_paneSizer->Add(button, 0, wxEXPAND | wxALL, FromDIP(1));
        m_items.push_back({ button, false,
                            (wxString::FromUTF8(command.name) + " " + wxString::FromUTF8(command.summary)).Lower(), categoryIndex });
    }
    m_pane->SetSizer(m_paneSizer);
    m_pane->FitInside();
    containerSizer->Add(m_pane, 1, wxEXPAND);
    container->SetSizer(containerSizer);
    return container;
}

void BatchEditorFrame::WireButton(wxButton* button, const BatCommand& command)
{
    const wxString summary = wxString::FromUTF8(command.summary);
    button->SetToolTip(summary);

    // Left click: insert the command at the cursor.
    button->Bind(wxEVT_BUTTON, [this, &command](wxCommandEvent&) {
        InsertCommand(wxString::FromUTF8(command.insertText));
    });
    // The explanation is long for a tooltip alone: the status bar shows it while the mouse is over the button.
    button->Bind(wxEVT_ENTER_WINDOW, [this, summary](wxMouseEvent& e) {
        SetStatusText(summary, 0);
        e.Skip();
    });
    button->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& e) {
        UpdateTitle();
        e.Skip();
    });
    // Right click: details and example.
    button->Bind(wxEVT_RIGHT_UP, [this, &command](wxMouseEvent&) {
        wxMessageBox(BatCommands::infoText(command), wxString::FromUTF8(command.name), wxOK | wxICON_INFORMATION, this);
    });
}

void BatchEditorFrame::ApplyFilter()
{
    const wxString needle = m_filter->GetValue().Lower().Trim().Trim(false);
    std::vector<bool> categoryHasMatch(m_categoryNames.size(), false);
    for (const PaneItem& item : m_items) {
        if (item.isHeader) continue;
        const bool match = needle.empty() || item.haystack.Contains(needle);
        m_paneSizer->Show(item.window, match);
        if (match) categoryHasMatch[item.category] = true;
    }
    for (const PaneItem& item : m_items) {
        if (item.isHeader) m_paneSizer->Show(item.window, categoryHasMatch[item.category]);
    }
    m_pane->Layout();
    m_pane->FitInside();
    m_pane->Scroll(0, 0);
}

void BatchEditorFrame::InsertCommand(const wxString& text)
{
    m_text->WriteText(text); // replaces the selection, leaves the cursor after the text
    m_text->SetFocus();
}

void BatchEditorFrame::ApplyBaseStyle()
{
    m_busy = true;
    m_text->Freeze();
    wxFont font(wxFontInfo(m_fontSize).Family(wxFONTFAMILY_TELETYPE).FaceName("Courier New"));
    m_text->SetFont(font);
    m_text->SetDefaultStyle(plainAttributes(m_fontSize));
    m_text->SetStyle(0, m_text->GetLastPosition(), plainAttributes(m_fontSize));
    m_text->Thaw();
    m_busy = false;
    m_sizeLabel->SetLabel(wxString::Format("%d pt", m_fontSize));
}

void BatchEditorFrame::Rehighlight()
{
    m_busy = true;
    long from = 0, to = 0;
    m_text->GetSelection(&from, &to);
    m_text->Freeze();
    m_text->SetStyle(0, m_text->GetLastPosition(), plainAttributes(m_fontSize));
    for (const BatSpan& span : HighlightBatch(m_text->GetValue())) {
        m_text->SetStyle(static_cast<long>(span.start), static_cast<long>(span.start + span.length), attributesFor(span.kind, m_fontSize));
    }
    m_text->SetSelection(from, to);
    // What is typed next is plain text, not a continuation of the last coloured word.
    m_text->SetDefaultStyle(plainAttributes(m_fontSize));
    m_text->Thaw();
    m_busy = false;
}

void BatchEditorFrame::ChangeFontSize(int delta)
{
    const int size = std::clamp(m_fontSize + delta, kMinFontSize, kMaxFontSize);
    if (size == m_fontSize) return;
    m_fontSize = size;
    AppSettings::setInt("editorFontSize", m_fontSize);
    ApplyBaseStyle();
    Rehighlight();
}

void BatchEditorFrame::OnBigger(wxCommandEvent&) { ChangeFontSize(1); }
void BatchEditorFrame::OnSmaller(wxCommandEvent&) { ChangeFontSize(-1); }

void BatchEditorFrame::OnMouseWheel(wxMouseEvent& event)
{
    if (event.ControlDown()) ChangeFontSize(event.GetWheelRotation() > 0 ? 1 : -1);
    else event.Skip();
}

void BatchEditorFrame::OnTextChanged(wxCommandEvent& event)
{
    event.Skip();
    if (m_busy) return;
    if (!m_dirty) {
        m_dirty = true;
        UpdateTitle();
    }
    m_timer.StartOnce(300); // highlight once the typing pauses
    UpdatePosition();
}

void BatchEditorFrame::OnHighlightTimer(wxTimerEvent&)
{
    Rehighlight();
}

void BatchEditorFrame::OnCaretMoved(wxEvent& event)
{
    UpdatePosition();
    event.Skip();
}

void BatchEditorFrame::UpdatePosition()
{
    long column = 0, line = 0;
    m_text->PositionToXY(m_text->GetInsertionPoint(), &column, &line);
    SetStatusText(wxString::Format("Ln %ld, Col %ld", line + 1, column + 1), 1);
}

void BatchEditorFrame::UpdateTitle()
{
    const wxString name = m_path.empty() ? wxString("(new file)") : wxFileName(m_path).GetFullName();
    SetTitle(wxString::Format("Batch file editor - %s%s", name, m_dirty ? " *" : ""));
    SetStatusText(m_path.empty() ? wxString("Unsaved file") : m_path, 0);
    SetStatusText(m_dirty ? wxString("Modified") : wxString("Saved"), 2);
}

bool BatchEditorFrame::ConfirmDiscard()
{
    if (!m_dirty) return true;
    const int answer = wxMessageBox("Save the changes to the batch file?", "Batch file editor",
                                    wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
    if (answer == wxCANCEL) return false;
    if (answer == wxYES) return Save();
    return true;
}

bool BatchEditorFrame::LoadFile(const wxString& path)
{
    if (!ConfirmDiscard()) return false;
    wxFile file(path);
    if (!file.IsOpened()) {
        wxMessageBox(wxString::Format("Cannot open %s", path), "Batch file editor", wxOK | wxICON_ERROR, this);
        return false;
    }
    const wxFileOffset length = file.Length();
    std::string bytes(static_cast<size_t>(length), '\0');
    if (length > 0 && file.Read(bytes.data(), static_cast<size_t>(length)) != length) {
        wxMessageBox(wxString::Format("Cannot read %s", path), "Batch file editor", wxOK | wxICON_ERROR, this);
        return false;
    }
    // Batch files are usually ANSI / OEM, sometimes UTF-8: try UTF-8 first, then the local code page.
    wxString text = wxString::FromUTF8(bytes.data(), bytes.size());
    if (text.empty() && !bytes.empty()) text = wxString(bytes.data(), wxConvLocal, bytes.size());
    if (text.empty() && !bytes.empty()) text = wxString::From8BitData(bytes.data(), bytes.size());
    text.Replace("\r\n", "\n"); // the control works with \n, WriteFile() puts \r\n back

    m_busy = true;
    m_text->SetValue(text);
    m_busy = false;
    m_path = path;
    m_dirty = false;
    ApplyBaseStyle();
    Rehighlight();
    m_text->SetInsertionPoint(0);
    m_text->ShowPosition(0); // start at the top of the file
    UpdateTitle();
    UpdatePosition();
    return true;
}

bool BatchEditorFrame::WriteFile(const wxString& path)
{
    wxString text = m_text->GetValue();
    text.Replace("\r\n", "\n"); // never double a CR
    text.Replace("\n", "\r\n"); // cmd.exe needs CRLF: labels and goto can fail with bare LF
    const wxScopedCharBuffer utf8 = text.utf8_str();
    wxFile file(path, wxFile::write);
    return file.IsOpened() && file.Write(utf8.data(), utf8.length()) == utf8.length();
}

bool BatchEditorFrame::Save()
{
    if (m_path.empty()) return SaveAs();
    if (!WriteFile(m_path)) {
        wxMessageBox(wxString::Format("Cannot write %s", m_path), "Batch file editor", wxOK | wxICON_ERROR, this);
        return false;
    }
    m_dirty = false;
    UpdateTitle();
    return true;
}

bool BatchEditorFrame::SaveAs()
{
    wxString dir = m_path.empty() ? wxString(DataDir::exeDirectory().wstring()) : wxFileName(m_path).GetPath();
    wxFileDialog dialog(this, "Save batch file", dir, m_path.empty() ? wxString("test.bat") : wxFileName(m_path).GetFullName(),
                        "Batch files (*.bat;*.cmd)|*.bat;*.cmd|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK) return false;
    wxString path = dialog.GetPath();
    if (wxFileName(path).GetExt().empty()) path += ".bat";
    if (!WriteFile(path)) {
        wxMessageBox(wxString::Format("Cannot write %s", path), "Batch file editor", wxOK | wxICON_ERROR, this);
        return false;
    }
    m_path = path;
    m_dirty = false;
    UpdateTitle();
    return true;
}

void BatchEditorFrame::OnNew(wxCommandEvent&)
{
    if (!ConfirmDiscard()) return;
    m_busy = true;
    m_text->SetValue(newFileTemplate());
    m_busy = false;
    m_path.clear();
    m_dirty = false;
    ApplyBaseStyle();
    Rehighlight();
    UpdateTitle();
}

void BatchEditorFrame::OnOpen(wxCommandEvent&)
{
    if (!ConfirmDiscard()) return;
    wxString dir = m_path.empty() ? wxString(DataDir::exeDirectory().wstring()) : wxFileName(m_path).GetPath();
    const wxString examples = wxString(DataDir::exeDirectory().wstring()) + wxFileName::GetPathSeparator() + AppInfo::kExamplesFolderName;
    if (m_path.empty() && wxDirExists(examples)) dir = examples;
    wxFileDialog dialog(this, "Open batch file", dir, wxString(),
                        "Batch files (*.bat;*.cmd)|*.bat;*.cmd|All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK) return;
    m_dirty = false; // already confirmed above
    LoadFile(dialog.GetPath());
}

void BatchEditorFrame::OnSave(wxCommandEvent&) { Save(); }
void BatchEditorFrame::OnSaveAs(wxCommandEvent&) { SaveAs(); }

void BatchEditorFrame::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto() && !ConfirmDiscard()) {
        event.Veto();
        return;
    }
    m_timer.Stop();
    Destroy();
}
