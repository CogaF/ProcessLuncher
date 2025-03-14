/*
    This is an application that was created by me Coga Fation but truth be told
    that without the help of ChatGpt it could have taken many many hours
    I'm taking for granted that I would have reached the same results which is a long shot
*/

#include "MainWindow.h"


MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "Parallel Command Runner - PCR")
{
    wxSize frameSize(950, 550);
    wxFrame::SetSize(frameSize);
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(windowIDs::ID_Hello, "&Build greeting...\tCtrl-H",
        "Greeting from application");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    mainPanel = new wxPanel(this, windowIDs::ID_MAIN_PANEL);
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Info");

    SetMenuBar(menuBar);

    CreateStatusBar();

    int ResultListWidth = 1950;
    resultList = new wxListCtrl(mainPanel, windowIDs::ID_COMMAND_LIST, wxDefaultPosition, wxSize(ResultListWidth, 550),
        wxLC_REPORT  | wxLC_HRULES | wxLC_VRULES | wxLC_AUTOARRANGE);
    Bind(wxEVT_MENU, &MainWindow::OnHello, this, windowIDs::ID_Hello);
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_BUTTON, &MainWindow::onRunCommand, this, windowIDs::ID_RUN_COMMAND_BT);
    Bind(wxEVT_THREAD_RESULT, &MainWindow::OnThreadResult, this);
    resultList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnKeyDown, this);
    resultList->Bind(wxEVT_MOTION, &MainWindow::OnMouseMove, this);

    SetStatusText("Command Runner!");
    resultsSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer = new wxBoxSizer(wxVERTICAL);
    componentsSizer = new wxBoxSizer(wxVERTICAL); 
    cmdsSizer = new wxBoxSizer(wxVERTICAL);

    // Add columns for timestamp and message
    resultList->InsertColumn(0, "Timestamp");
    resultList->InsertColumn(1, "Information");

    runBT = new wxButton(mainPanel, windowIDs::ID_RUN_COMMAND_BT, "Run command(s)", wxDefaultPosition, wxSize(-1, 40));
    // Add messages to the list (with timestamp and message content)
    //AddMessage(get_current_timestamp(), "Hello, this is the first message.");
    //AddMessage(get_current_timestamp(), "Second message appears here.");
    // Get the current frame size (including the border)

    // Set the proportions: Message is 8 times the width of Timestamp
    int timestampWidth = ResultListWidth / 9;   // 1 part of the total 9 parts (timestamp + 8 * message)
    int messageWidth = timestampWidth * 8; // 8 parts for message
    // Set the column widths
    resultList->SetColumnWidth(0, timestampWidth); // Timestamp column
    resultList->SetColumnWidth(1, messageWidth);   // Message column
    Cmd1 = new cmdgui(mainPanel);
    Cmd2 = new cmdgui(mainPanel);
    Cmd3 = new cmdgui(mainPanel);
    Cmd4 = new cmdgui(mainPanel);
    Cmd5 = new cmdgui(mainPanel);
    Cmd6 = new cmdgui(mainPanel);
    Cmd7 = new cmdgui(mainPanel);
    Cmd8 = new cmdgui(mainPanel);
    Cmd9 = new cmdgui(mainPanel);
    Cmd10 = new cmdgui(mainPanel);
    Cmd11 = new cmdgui(mainPanel);

    Cmd1->setCounters("CMD counters 1");
    Cmd2->setCounters("CMD counters 2");
    Cmd3->setCounters("CMD counters 3");
    Cmd4->setCounters("CMD counters 4");
    Cmd5->setCounters("CMD counters 5");
    Cmd6->setCounters("CMD counters 6");
    Cmd7->setCounters("CMD counters 7");
    Cmd8->setCounters("CMD counters 8");
    Cmd9->setCounters("CMD counters 9");
    Cmd10->setCounters("CMD counters 10");
    Cmd11->setCounters("CMD counters 11");

    wxBoxSizer* cmd1_sz = Cmd1->getPointer();
    wxBoxSizer* cmd2_sz = Cmd2->getPointer();
    wxBoxSizer* cmd3_sz = Cmd3->getPointer();
    wxBoxSizer* cmd4_sz = Cmd4->getPointer();
    wxBoxSizer* cmd5_sz = Cmd5->getPointer();
    wxBoxSizer* cmd6_sz = Cmd6->getPointer();
    wxBoxSizer* cmd7_sz = Cmd7->getPointer();
    wxBoxSizer* cmd8_sz = Cmd8->getPointer();
    wxBoxSizer* cmd9_sz = Cmd9->getPointer();
    wxBoxSizer* cmd10_sz = Cmd10->getPointer();
    wxBoxSizer* cmd11_sz = Cmd11->getPointer();

    cmdsSizer->Add(cmd1_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd2_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd3_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd4_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd5_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd6_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd7_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd8_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd9_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd10_sz, 1, wxEXPAND | wxALL, 1);
    cmdsSizer->Add(cmd11_sz, 1, wxEXPAND | wxALL, 1);

    wxIcon _frame_icon(wxICON(IDI_ICON1));
    //SetIcon(_frame_icon);

    wxIcon icon("ALBSOLEL_ico_256x256.ico", wxBITMAP_TYPE_ICO);

    // Set the icon for the frame
    SetIcon(icon);
    resultsSizer->Add(resultList, 1, wxEXPAND | wxALL, 1);

    componentsSizer->Add(runBT, 1, wxEXPAND | wxALL, 1);
    componentsSizer->Add(cmdsSizer, 1, wxEXPAND | wxALL, 1);
    componentsSizer->Add(resultsSizer, 20, wxEXPAND | wxALL, 1);

    mainPanel->SetSizer(componentsSizer);

    mainSizer->Add(mainPanel);

    SetSizer(mainSizer);
    mainPanel->Show(true);
    //mainSizer->Add(mainPanel);
}

void MainWindow::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void MainWindow::OnAbout(wxCommandEvent& event)
{
    wxString ToPrint = "This APP can run multiple \"*.bat\" file or CMD like commands in separated threads (aka simultaneously)\n";
    ToPrint += "Results of each execution will be shown as separated messages in the list\n\nDev. Coga F. (EGO Group S.r.L @ fation.coga@egogroup.eu)";
    wxMessageBox(ToPrint, "About PCR", wxOK | wxICON_INFORMATION);
}

void MainWindow::OnHello(wxCommandEvent& event)
{
    wxLogMessage("wxWidgets 3.2.6 Inside");
}

void MainWindow::OnKeyDown(wxKeyEvent& event) {
    if (event.ControlDown() && event.GetKeyCode() == 'A') {  // Detect Ctrl+A
        SelectAllItems();
    }else if (event.ControlDown() && event.GetKeyCode() == 'C') {  // Detect Ctrl+C
        CopySelectedRow();
    }
    else {
        event.Skip();  // Let other key events process normally
    }
}
void MainWindow::SelectAllItems() {
    long itemIndex = -1;
    while ((itemIndex = resultList->GetNextItem(itemIndex, wxLIST_NEXT_ALL)) != wxNOT_FOUND) {
        // Select the item by setting its state to selected
        resultList->SetItemState(itemIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}
wxString MainWindow::get_current_timestamp() {
    using namespace std::chrono;

    // Get current time as a time_point
    system_clock::time_point now = system_clock::now();

    // Convert to system time
    std::time_t now_time = system_clock::to_time_t(now);

    // Convert to string
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));

    return wxString(buffer);
} 

void MainWindow::OnMouseMove(wxMouseEvent& event) {
    int flags;  // Placeholder for hit-test flags
    long itemIndex = resultList->HitTest(event.GetPosition(), flags);

    if (itemIndex >= 0) {  // If a valid row is found
        wxString fullText = resultList->GetItemText(itemIndex, 1);  // Assume column 1
        resultList->SetToolTip(fullText);  // Show full text as tooltip
    }
    else {
        resultList->SetToolTip("");  // Clear tooltip when not hovering over an item
    }

    event.Skip();  // Allow further event processing
}

void MainWindow::CopySelectedRow() {
    wxString allSelectedRows;

    // Start iterating through all selected rows
    long itemIndex = -1;
    while ((itemIndex = resultList->GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != wxNOT_FOUND) {
        wxString rowText;

        // Get the text for each column in the row
        for (int col = 0; col < resultList->GetColumnCount(); col++) {
            rowText += resultList->GetItemText(itemIndex, col);
            if (col < resultList->GetColumnCount() - 1)
                rowText += "\t";  // Tab-separated for easy pasting into spreadsheets
        }

        allSelectedRows += rowText + "\n";  // Add the row to the final result with a newline
    }

    // If no rows are selected, return early
    if (allSelectedRows.IsEmpty())
        return;

    // Copy to clipboard
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(allSelectedRows));
        wxTheClipboard->Close();
    }
}
// Function to add message to the list
void MainWindow::AddMessage(const wxString& timestamp, const wxString& message) {
    // Insert the new message at the top
    long index = resultList->InsertItem(0, timestamp);
    resultList->SetItem(index, 1, message);
}

void MainWindow::onRunCommand(wxCommandEvent& event)
{
    bool runCommands = true;
    if (Cmd1->getRunning() || Cmd2->getRunning() || Cmd3->getRunning() || Cmd4->getRunning() ||
        Cmd5->getRunning() || Cmd6->getRunning() || Cmd7->getRunning() || Cmd8->getRunning() ||
        Cmd9->getRunning() || Cmd10->getRunning() || Cmd11->getRunning()) {

        if (wxMessageBox("Found at leas one thread still active:\nDo you want to launch all the not running commands?",
            " PCR - Confirmation Request", wxYES_NO) == wxYES) {
            runCommands = true;
        }
        else {
            runCommands = false;
        }

    }
    //if permission granted to run 
    //then each command will be checked if not running, if yes then will add message that it was found running
    //if not running then it will be checked if active, if yes it will be launched after setting it to running, if not the will be silently skipped
    // setting to running return bool true for setted ok otherwise false, case gui has nullptr, in this case a message that informs about this will be added to list
    if (runCommands) {

        if (!Cmd1->getRunning()) {
            if (Cmd1->isActive()) {
                if (Cmd1->setRunning(true)) {
                    StartThread(Cmd1->getCmd(), 1);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 1 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 1 already active"));
        }

        if (!Cmd2->getRunning()) {
            if (Cmd2->isActive()) {
                if (Cmd2->setRunning(true)) {
                    StartThread(Cmd2->getCmd(), 2);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 2 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 2 already active"));
        }

        if (!Cmd3->getRunning()) {
            if (Cmd3->isActive()) {
                if (Cmd3->setRunning(true)) {
                    StartThread(Cmd3->getCmd(), 3);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 3 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 3 already active"));
        }

        if (!Cmd4->getRunning()) {
            if (Cmd4->isActive()) {
                if (Cmd4->setRunning(true)) {
                    StartThread(Cmd4->getCmd(), 4);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 4 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 4 already active"));
        }

        if (!Cmd5->getRunning()) {
            if (Cmd5->isActive()) {
                if (Cmd5->setRunning(true)) {
                    StartThread(Cmd5->getCmd(), 5);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 5 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 5 already active"));
        }

        if (!Cmd6->getRunning()) {
            if (Cmd6->isActive()) {
                if (Cmd6->setRunning(true)) {
                    StartThread(Cmd6->getCmd(), 6);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 6 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 6 already active"));
        }

        if (!Cmd7->getRunning()) {
            if (Cmd7->isActive()) {
                if (Cmd7->setRunning(true)) {
                    StartThread(Cmd7->getCmd(), 7);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 7 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 7 already active"));
        }

        if (!Cmd8->getRunning()) {
            if (Cmd8->isActive()) {
                if (Cmd8->setRunning(true)) {
                    StartThread(Cmd8->getCmd(), 8);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 8 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 8 already active"));
        }

        if (!Cmd9->getRunning()) {
            if (Cmd9->isActive()) {
                if (Cmd9->setRunning(true)) {
                    StartThread(Cmd9->getCmd(), 9);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 9 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 9 already active"));
        }

        if (!Cmd10->getRunning()) {
            if (Cmd10->isActive()) {
                if (Cmd10->setRunning(true)) {
                    StartThread(Cmd10->getCmd(), 10);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 10 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 10 already active"));
        }

        if (!Cmd11->getRunning()) {
            if (Cmd11->isActive()) {
                if (Cmd11->setRunning(true)) {
                    StartThread(Cmd11->getCmd(), 11);
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 11 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 11 already active"));
        }
    }

}

void MainWindow::StartThread(const wxString& input, int CommandIndex) {
    // Start a background thread
    std::thread([this, input, CommandIndex]() {
        std::string result = RunCommand(input.ToStdString());

        // Create and post an event to the main thread
        wxCommandEvent event(wxEVT_THREAD_RESULT);
        event.SetString(result);
        event.SetInt(CommandIndex);
        wxQueueEvent(this, event.Clone());
        }).detach();  // Detach so the thread runs independently

#ifdef _DEBUG
    AddMessage(get_current_timestamp(), wxString::Format("Started thread for CMD %d", CommandIndex));
#endif
}

// Function to run the command and capture output
std::string MainWindow::RunCommand(const std::string& command) {

    // Create pipes for capturing output
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
        return "Error: Failed to create pipe!";
    }

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };

    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_SHOWNORMAL;  // Hides the window
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;  // Redirect stderr to stdout

    // Create the process
    if (!CreateProcessA(NULL, (LPSTR)(("cmd /c \"" + command + "\"").c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return "Error: Failed to execute command!";
    }

    // Close the write handle to allow reading from the pipe
    CloseHandle(hWrite);

    // Read the command output
    std::string result;
    char buffer[128];
    DWORD bytesRead;

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    // Cleanup
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return result;
}


void MainWindow::OnThreadResult(wxCommandEvent& event) {
    // Update UI when result is received
    if (event.GetInt() == 1) {
        if (event.GetString().Find(Cmd1->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd1++;
        }
        else {
            failed_cmd1++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd1, failed_cmd1);
        Cmd1->setCounters(countersTrack);
        Cmd1->setRunning(false);
    }
    if (event.GetInt() == 2) {
        if (event.GetString().Find(Cmd2->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd2++;
        }
        else {
            failed_cmd2++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd2, failed_cmd2);
        Cmd2->setCounters(countersTrack);
        Cmd2->setRunning(false);
    }
    if (event.GetInt() == 3) {
        if (event.GetString().Find(Cmd3->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd3++;
        }
        else {
            failed_cmd3++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd3, failed_cmd3);
        Cmd3->setCounters(countersTrack);
        Cmd3->setRunning(false);
    }
    if (event.GetInt() == 4) {
        if (event.GetString().Find(Cmd4->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd4++;
        }
        else {
            failed_cmd4++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd4, failed_cmd4);
        Cmd4->setCounters(countersTrack);
        Cmd4->setRunning(false);
    }
    if (event.GetInt() == 5) {
        if (event.GetString().Find(Cmd5->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd5++;
        }
        else {
            failed_cmd5++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd5, failed_cmd5);
        Cmd5->setCounters(countersTrack);
        Cmd5->setRunning(false);
    }
    if (event.GetInt() == 6) {
        if (event.GetString().Find(Cmd6->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd6++;
        }
        else {
            failed_cmd6++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd6, failed_cmd6);
        Cmd6->setCounters(countersTrack);
        Cmd6->setRunning(false);
    }
    if (event.GetInt() == 7) {
        if (event.GetString().Find(Cmd7->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd7++;
        }
        else {
            failed_cmd7++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd7, failed_cmd7);
        Cmd7->setCounters(countersTrack);
        Cmd7->setRunning(false);
    }
    if (event.GetInt() == 8) {
        if (event.GetString().Find(Cmd8->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd8++;
        }
        else {
            failed_cmd8++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd8, failed_cmd8);
        Cmd8->setCounters(countersTrack);
        Cmd8->setRunning(false);
    }
    if (event.GetInt() == 9) {
        if (event.GetString().Find(Cmd9->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd9++;
        }
        else {
            failed_cmd9++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd9, failed_cmd9);
        Cmd9->setCounters(countersTrack);
        Cmd9->setRunning(false);
    }
    if (event.GetInt() == 10) {
        if (event.GetString().Find(Cmd10->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd10++;
        }
        else {
            failed_cmd10++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd10, failed_cmd10);
        Cmd10->setCounters(countersTrack);
        Cmd10->setRunning(false);
    }
    if (event.GetInt() == 11) {
        if (event.GetString().Find(Cmd11->getPostiveVal()) != wxNOT_FOUND) {
            passes_cmd11++;
        }
        else {
            failed_cmd11++;
        }
        //AddMessage(get_current_timestamp(), event.GetString());
        wxString countersTrack = wxString::Format("P = %03d  ||  F= %03d",
            passes_cmd11, failed_cmd11);
        Cmd11->setCounters(countersTrack);
        Cmd11->setRunning(false);
    }
    wxString MessageToAdd = wxString::Format("Cmd %d result is: " + event.GetString(), event.GetInt());
    AddMessage(get_current_timestamp(), MessageToAdd);
}