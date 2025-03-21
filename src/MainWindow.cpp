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

    wxMenu* settingsMenu = new wxMenu;
    settingsMenu->Append(windowIDs::ID_ENABLE_EDIT, "&Enable Edit\tCtrl-E",
        "Enable edit of commands properties");
    settingsMenu->Append(windowIDs::ID_DISABLE_EDIT, "&Disable Edit\tCtrl-D",
        "Disable edit of commands properties");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile,       "&File");
    menuBar->Append(settingsMenu,   "&Settings");
    menuBar->Append(menuHelp,       "&Info");





    SetMenuBar(menuBar);

    CreateStatusBar();

    int ResultListWidth = 1950;
    resultList = new wxListCtrl(mainPanel, windowIDs::ID_COMMAND_LIST, wxDefaultPosition, wxSize(ResultListWidth, 550),
        wxLC_REPORT  | wxLC_HRULES | wxLC_VRULES | wxLC_AUTOARRANGE);
    Bind(wxEVT_MENU, &MainWindow::OnHello, this, windowIDs::ID_Hello);
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainWindow::OnEnable, this, windowIDs::ID_ENABLE_EDIT);
    Bind(wxEVT_MENU, &MainWindow::OnDisable, this, windowIDs::ID_DISABLE_EDIT);
    Bind(wxEVT_BUTTON, &MainWindow::onRunCommand, this, windowIDs::ID_RUN_COMMAND_BT);
    Bind(wxEVT_CHECKBOX, &MainWindow::OnCheckBoxEvent, this);
    Bind(wxEVT_THREAD_RESULT, &MainWindow::OnThreadResult, this);
    Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnClose, this);
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

    for (int i = 0; i < nrOfCMDs; i++) {
        arrayOfGuiCMDs[i] = new cmdgui(mainPanel, i);
        arrayOfGuiCMDs[i]->setCounters(wxString::Format("counters of CMD: %d", i+1));
        arrayOfGuiCMDs_sz[i] = arrayOfGuiCMDs[i]->getPointer();
        cmdsSizer->Add(arrayOfGuiCMDs_sz[i], 1, wxEXPAND | wxALL, 1);
    }
    
    wxIcon _frame_icon(wxICON(MAINICON));
    SetIcon(_frame_icon);

    resultsSizer->Add(resultList, 1, wxEXPAND | wxALL, 1);

    componentsSizer->Add(runBT, 1, wxEXPAND | wxALL, 1);
    componentsSizer->Add(cmdsSizer, 1, wxEXPAND | wxALL, 1);
    componentsSizer->Add(resultsSizer, 20, wxEXPAND | wxALL, 1);

    mainPanel->SetSizer(componentsSizer);

    mainSizer->Add(mainPanel);

    SetSizer(mainSizer);
    mainPanel->Show(true);
}

void MainWindow::OnExit(wxCommandEvent& event)
{
    Close(true);
}


void MainWindow::OnCheckBoxEvent(wxCommandEvent& event)
{

    for (int i = 0; i < nrOfCMDs; i++) {

        if (event.GetId() == arrayOfGuiCMDs[i]->getCurrId()) {
            if (arrayOfGuiCMDs[i]->Cmd_active_CB->GetValue()) {
                arrayOfGuiCMDs[i]->enable();
            }
            else {
                arrayOfGuiCMDs[i]->disable();
            }
        }else if(event.GetId() == (arrayOfGuiCMDs[i]->getCurrId() + arrayOfGuiCMDs[i]->SEQUENTIAL_ID_INDEX)) {
            if (arrayOfGuiCMDs[i]->Cmd_sequential_CB->GetValue()) {
                arrayOfGuiCMDs[i]->setSequential(true);
            }
            else {
                arrayOfGuiCMDs[i]->setSequential(false);
            }
        }
        else if (event.GetId() == (arrayOfGuiCMDs[i]->getCurrId() + arrayOfGuiCMDs[i]->VIEW_ID_INDEX)) {
            if (arrayOfGuiCMDs[i]->Cmd_view_CB->GetValue()) {
                arrayOfGuiCMDs[i]->setView(true);
            }
            else {
                arrayOfGuiCMDs[i]->setView(false);
            }

        }

    }
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


void MainWindow::OnEnable(wxCommandEvent& event)
{
    EnableCmds();
}

void MainWindow::EnableCmds()
{

    for (int i = 0; i < nrOfCMDs; i++) {
        arrayOfGuiCMDs[i]->enableEditables();
    }

}


void MainWindow::OnDisable(wxCommandEvent& event)
{
    DisableCmds();
}

void MainWindow::DisableCmds()
{
    for (int i = 0; i < nrOfCMDs; i++) {
        arrayOfGuiCMDs[i]->disableEditables();
    }

}

//Catching Ctrl+A and Ctrl+C for select all or copy 
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

//Doing the actual selection of all rows
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
        wxString fullText = resultList->GetItemText(itemIndex, 0) + "\n" + resultList->GetItemText(itemIndex, 1);  // Assume column 1
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


void MainWindow::OnClose(wxCloseEvent& event) {
    if (event.CanVeto())
    {
        if (wxMessageBox("Are you sure you want to quit, unfinished CMDs will be terminated!!!",
            "Request to quit application",
            wxICON_QUESTION | wxYES_NO) != wxYES)
        {
            event.Veto();
            return;
        }
    }
    blockingThread = false;
    AddMessage(get_current_timestamp(), "Quitting app");
    Destroy();
}

void MainWindow::onRunCommand(wxCommandEvent& event)
{
    DisableCmds();
    bool runCommands = true;
    bool foundAtLeasOne = false;

    for (int i = 0; i < nrOfCMDs; i++) {
        if (arrayOfGuiCMDs[i]->getRunning()) {
            foundAtLeasOne = true;
        }
        //Setting all gui CMDs to default true
        arrayOfGuiCMDs[i]->setResult(true);
    }

    if (foundAtLeasOne) {
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
        for (int i = 0; i < nrOfCMDs; i++) {
            if (!arrayOfGuiCMDs[i]->getRunning()) {//Checking if it is still running goin through only if not
                if (arrayOfGuiCMDs[i]->isActive()) {//Checking if it is an active CMD aka TO RUN
                    if (arrayOfGuiCMDs[i]->setRunning(true)) {//Setting its status to RUN
                        if (arrayOfGuiCMDs[i]->isSequential()) {//Checking if sequential to RUN on it's own
                            //with a message box giving the operator the possibility to wait previous command to finish in case it depends on them
                            wxMessageBox("Next command will be executed in single mode, the next ones will wait for this to complete\nMake sure it does not depend on previous commands and hit OK when ready",
                                "ATTENTION Blocking thread", wxOK | wxICON_INFORMATION);
                            //setting status block to wait if sequential
                            blockingThread = true;
                            //tracking wich command created the blocking thread to properly handle result
                            blockingCommandIndex = i;
                            //assuming no previous fails
                            bool PreviousCmdFailed = false;
                            //reading previous results only if greater than 1 ontherwise i-1 is array out of bound, we don't want to end like CrowdStrike, do we?
                            if (i > 0) {
                                if (!arrayOfGuiCMDs[i - 1]->getResukt()) {
                                    PreviousCmdFailed = true;
                                }
                            }
                            //getting decision if want's to preceed in case of previous fail
                            if (PreviousCmdFailed){
                                if (wxMessageBox("Found at least one previous executed CMD with fail result\nDo you still want to proceed?",
                                    "ATTENTION Previous Failure", wxYES_NO) == wxYES) {
                                    //confirmed to start command even if one previous cmd failed
                                    StartThread(arrayOfGuiCMDs[i]->getCmd(), i, true);
                                }
                                else {//Adding a message to track the decision to skip, releasing blocking status and setting to not running
                                    AddMessage(get_current_timestamp(), wxString::Format("Skipped CMD: %d", i));
                                    blockingThread = false;
                                    blockingCommandIndex = -1;
                                    arrayOfGuiCMDs[i]->setRunning(false);
                                }
                            }
                            else {//No previous fail so strating blocking thread 
                                StartThread(arrayOfGuiCMDs[i]->getCmd(), i, true);
                            }
                        }
                        else {//Found it to be NOT sequential so runnin in non blocking
                            StartThread(arrayOfGuiCMDs[i]->getCmd(), i, false);
                        }
                    }
                    else {//not able to seti it running never happened but might if gui components have any issue
                        AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD %d gui to BUSY", i + 1));
                    }
                }//Here nothing happens becaue it is the check if it is active, if not active no need to track, it is visibile from GUI
            }
            else {//Found that it is already runnging after confirmation of run commands even if at least one running 
                  //Here I will just add a message to trace it was skipped since already active
                AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD %d already active", i + 1));
            }
            //Cycling in loop untile the CMD that was run in sequential mode sets blockingThread in false
            //wxYield() makes GUI still responsive inside a loop that can potentially run infinitely in cas thread never ends
            //TODO add possibility to break it programmatically maby a menu item with chortcut
            while (blockingThread) {
                wxYield();
            }
        }
    }
}

void MainWindow::StartThread(const wxString& input, int CommandIndex, bool sequentialStatus) {
    // Start a background thread
    std::thread([this, input, &CommandIndex]() {
        std::string result = RunCommand(input.ToStdString(), CommandIndex);

        wxYield();
        // Create and post an event to the main thread
        wxCommandEvent event(wxEVT_THREAD_RESULT);
        event.SetString(result);
        event.SetInt(CommandIndex);
        wxQueueEvent(this, event.Clone());
        }).detach();  // Join to run it sequentially
    
    wxYield();
    #ifdef _DEBUG
        AddMessage(get_current_timestamp(), wxString::Format("Started thread for CMD %d", CommandIndex+1));
    #endif
}


// Function to run the command and capture output
std::string MainWindow::RunCommand(const std::string& command, int& commandIndex) {

    // Create pipes for capturing output
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
        return "Error: Failed to create pipe!";
    }

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };

    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;  // Hides the window
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;  // Redirect stderr to stdout


    int viewFlag = CREATE_NO_WINDOW;
    if (arrayOfGuiCMDs[commandIndex]->getView()) {
        
        // We will write to both the pipe and the console
        si.dwFlags = STARTF_USESTDHANDLES;
        // Duplicate the write handle to the console output (stdout)
        HANDLE hStdOutDup;
        DuplicateHandle(GetCurrentProcess(), hWrite, GetCurrentProcess(), &hStdOutDup, 0, TRUE, DUPLICATE_SAME_ACCESS);

        si.hStdOutput = hStdOutDup; // Output goes to both the pipe and the console
        si.hStdError = hStdOutDup;  // Error goes to both the pipe and the console
        CloseHandle(hStdOutDup);
        viewFlag = CREATE_NEW_CONSOLE;
    }
    // Create the process
    if (!CreateProcessA(NULL, (LPSTR)(("cmd /c \"" + command + "\"").c_str()), NULL, NULL, TRUE, viewFlag, NULL, NULL, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return "Error: Failed to execute command!";
    }

    // Close the write handle to allow reading from the pipe
    CloseHandle(hWrite);

    // Read the command output
    std::string result;
    char buffer[0xBFFFF];
    DWORD bytesRead;

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        wxYield(); 
        if (bytesRead >= sizeof(buffer)) {
            buffer[sizeof(buffer) - 1] = '\0';  // Prevent overflow
        }
        else {
            buffer[bytesRead] = '\0';  // Safe termination
        }
        result += buffer;
    }

    // Cleanup
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    wxYield();
    return result;
}


// Update UI when result is received
void MainWindow::OnThreadResult(wxCommandEvent& event) {
    for (int i = 0; i < nrOfCMDs; i++) {
        if (event.GetInt() == (i)) {
            if (arrayOfGuiCMDs[i]->getPostiveVal().Find(searchOnFIle_s) == wxNOT_FOUND) {
                if (event.GetString().Find(arrayOfGuiCMDs[i]->getPostiveVal()) == wxNOT_FOUND) {
                    arrayOfFailed[i]++;
                    arrayOfGuiCMDs[i]->setResult(false);
                }
                else {
                    arrayOfPassed[i]++;
                    arrayOfGuiCMDs[i]->setResult(true);
                }
                //AddMessage(get_current_timestamp(), event.GetString());
                wxString countersTrack = wxString::Format("  P= %03d || F= %03d",
                    arrayOfPassed[i], arrayOfFailed[i]);
                arrayOfGuiCMDs[i]->setCounters(countersTrack);
                ArrayOfresponses[i] = event.GetString();
                //wxLogMessage(ArrayOfresponses[i], "MessageReceived");
                arrayOfGuiCMDs[i]->setRunning(false);
                //Checking if there was an active blocking thread if yes then checking if the result came from the command who 
                //called the blocking thread, if yes than releasing the block
                if (blockingThread && (blockingCommandIndex == i)) {
                    blockingThread = false;
                }
                wxString MessageToAdd = wxString::Format("Cmd %d result is: " + event.GetString(), event.GetInt() + 1);
                AddMessage(get_current_timestamp(), MessageToAdd);
            }
            else {
                wxString fileNamePath = extractFileName(arrayOfGuiCMDs[i]->getPostiveVal());
                wxFile mFile;
                wxTextFile resultFile;
                if (mFile.Exists(fileNamePath)) {

                    wxString positiveResult = extractPositiveResult(arrayOfGuiCMDs[i]->getPostiveVal());
                    if (FindInFile(fileNamePath, positiveResult)) {
                        arrayOfPassed[i]++;
                        arrayOfGuiCMDs[i]->setResult(true);
                    }
                    else {
                        arrayOfFailed[i]++;
                        arrayOfGuiCMDs[i]->setResult(false);
                    }
                    //AddMessage(get_current_timestamp(), event.GetString());
                    wxString countersTrack = wxString::Format("  P= %03d || F= %03d",
                        arrayOfPassed[i], arrayOfFailed[i]);
                    arrayOfGuiCMDs[i]->setCounters(countersTrack);
                    ArrayOfresponses[i] = event.GetString();
                    //wxLogMessage(ArrayOfresponses[i], "MessageReceived");
                    arrayOfGuiCMDs[i]->setRunning(false);
                    //Checking if there was an active blocking thread if yes then checking if the result came from the command who 
                    //called the blocking thread, if yes than releasing the block
                    if (blockingThread && (blockingCommandIndex == i)) {
                        blockingThread = false;
                    }
                    wxString MessageToAdd = wxString::Format("Cmd %d result is: " + event.GetString(), event.GetInt() + 1);
                    AddMessage(get_current_timestamp(), MessageToAdd);
                }
                else {
                    arrayOfFailed[i]++;
                    arrayOfGuiCMDs[i]->setResult(false);
                    //AddMessage(get_current_timestamp(), event.GetString());
                    wxString countersTrack = wxString::Format("  P= %03d || F= %03d",
                        arrayOfPassed[i], arrayOfFailed[i]);
                    arrayOfGuiCMDs[i]->setCounters(countersTrack);
                    ArrayOfresponses[i] = event.GetString();
                    //wxLogMessage(ArrayOfresponses[i], "MessageReceived");
                    arrayOfGuiCMDs[i]->setRunning(false);
                    //Checking if there was an active blocking thread if yes then checking if the result came from the command who 
                    //called the blocking thread, if yes than releasing the block
                    if (blockingThread && (blockingCommandIndex == i)) {
                        blockingThread = false;
                    }
                    AddMessage(get_current_timestamp(), wxString::Format("File " + fileNamePath + " doesn't exist"));
                }
            }
        }
    }
}

wxString MainWindow::extractFileName(wxString completeString) {
    wxString toReturn = "";
    int indexOfStartFilename = completeString.Find(searchOnFIle_s) + searchOnFIle_s.length();
    int indexOfEndFileFilename = completeString.find_last_of(separator) + separator.length();
    toReturn = completeString.SubString(indexOfStartFilename, indexOfEndFileFilename);
    AddMessage(get_current_timestamp(), wxString::Format("Filename starts at index: \"%d\"", indexOfStartFilename));
    AddMessage(get_current_timestamp(), wxString::Format("Filename ends at index: \"%d\"", indexOfEndFileFilename));
    AddMessage(get_current_timestamp(), wxString::Format("Filename is: \"%s\"", toReturn));

    return toReturn;
}


wxString MainWindow::extractPositiveResult(wxString completeString) {
    wxString toReturn = "";

    return toReturn;

}

bool MainWindow::FindInFile(const wxString& filePath, const wxString& searchString)
{
    bool toRetrun = false;
    wxFileInputStream input(filePath);
    if (!input.IsOk())
    {
        AddMessage(get_current_timestamp(), wxString::Format("Failed to open file \"%s\" @SearchLargeFile()", filePath));
    }

    wxTextInputStream text(input);
    wxString line;
    bool found = false;
#ifdef _DEBUG
    AddMessage(get_current_timestamp(), wxString::Format("START Search"));
#endif
    while (input.IsOk() && !input.Eof() && !found) // Read line by line
    {
        line = text.ReadLine();
        if (line.Contains(searchString))
        {
#ifdef _DEBUG
            AddMessage(get_current_timestamp(), wxString::Format("Found \"%s\" at file \"%s\" @FindInFile()", searchString, filePath));
#endif
            toRetrun = true;
            found = true;
        }
        wxYield();
    }
#ifdef _DEBUG
    AddMessage(get_current_timestamp(), wxString::Format("END Search"));
#endif

    return toRetrun;
}