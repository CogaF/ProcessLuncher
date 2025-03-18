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


    //Cmd1 = new cmdgui(mainPanel);
    //Cmd2 = new cmdgui(mainPanel);
    //Cmd3 = new cmdgui(mainPanel);
    //Cmd4 = new cmdgui(mainPanel);
    //Cmd5 = new cmdgui(mainPanel);
    //Cmd6 = new cmdgui(mainPanel);
    //Cmd7 = new cmdgui(mainPanel);
    //Cmd8 = new cmdgui(mainPanel);
    //Cmd9 = new cmdgui(mainPanel);
    //Cmd10 = new cmdgui(mainPanel);
    //Cmd11 = new cmdgui(mainPanel);

    for (int i = 0; i < nrOfCMDs; i++) {
        arrayOfGuiCMDs[i] = new cmdgui(mainPanel, i);
        arrayOfGuiCMDs[i]->setCounters(wxString::Format("counters of CMD: %d", i+1));
        arrayOfGuiCMDs_sz[i] = arrayOfGuiCMDs[i]->getPointer();
        cmdsSizer->Add(arrayOfGuiCMDs_sz[i], 1, wxEXPAND | wxALL, 1);
    }
    
    //Cmd1 = new cmdgui(mainPanel, 0);
    //Cmd2 = new cmdgui(mainPanel, 1);
    //Cmd3 = new cmdgui(mainPanel, 2);
    //Cmd4 = new cmdgui(mainPanel, 3);
    //Cmd5 = new cmdgui(mainPanel, 4);
    //Cmd6 = new cmdgui(mainPanel, 5);
    //Cmd7 = new cmdgui(mainPanel, 6);
    //Cmd8 = new cmdgui(mainPanel, 7);
    //Cmd9 = new cmdgui(mainPanel, 8);
    //Cmd10 = new cmdgui(mainPanel, 9);
    //Cmd11 = new cmdgui(mainPanel, 10);

    //Cmd1->setCounters("CMD counters 1");
    //Cmd2->setCounters("CMD counters 2");
    //Cmd3->setCounters("CMD counters 3");
    //Cmd4->setCounters("CMD counters 4");
    //Cmd5->setCounters("CMD counters 5");
    //Cmd6->setCounters("CMD counters 6");
    //Cmd7->setCounters("CMD counters 7");
    //Cmd8->setCounters("CMD counters 8");
    //Cmd9->setCounters("CMD counters 9");
    //Cmd10->setCounters("CMD counters 10");
    //Cmd11->setCounters("CMD counters 11");

    //wxBoxSizer* cmd1_sz = Cmd1->getPointer();
    //wxBoxSizer* cmd2_sz = Cmd2->getPointer();
    //wxBoxSizer* cmd3_sz = Cmd3->getPointer();
    //wxBoxSizer* cmd4_sz = Cmd4->getPointer();
    //wxBoxSizer* cmd5_sz = Cmd5->getPointer();
    //wxBoxSizer* cmd6_sz = Cmd6->getPointer();
    //wxBoxSizer* cmd7_sz = Cmd7->getPointer();
    //wxBoxSizer* cmd8_sz = Cmd8->getPointer();
    //wxBoxSizer* cmd9_sz = Cmd9->getPointer();
    //wxBoxSizer* cmd10_sz = Cmd10->getPointer();
    //wxBoxSizer* cmd11_sz = Cmd11->getPointer();

    //cmdsSizer->Add(cmd1_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd2_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd3_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd4_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd5_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd6_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd7_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd8_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd9_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd10_sz, 1, wxEXPAND | wxALL, 1);
    //cmdsSizer->Add(cmd11_sz, 1, wxEXPAND | wxALL, 1);

    wxIcon _frame_icon(wxICON(MAINICON));
    SetIcon(_frame_icon);

    //wxIcon icon("ALBSOLEL_ico_256x256.ico", wxBITMAP_TYPE_ICO);

    // Set the icon for the frame
    //SetIcon(icon);
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
    /*

    //if (event.GetId() == Cmd1->getCurrId()) {
    //    if (Cmd1->Cmd_active_CB->GetValue()) {
    //        Cmd1->enable();
    //    }
    //    else {
    //        Cmd1->disable();
    //    }
    //}
    //if (event.GetId() == Cmd1->getCurrId()+1) {
    //    if (Cmd1->Cmd_sequential_CB->GetValue()) {
    //        Cmd1->setSequential(true);
    //    }
    //    else {
    //        Cmd1->setSequential(false);
    //    }
    //}
    //

    //if (event.GetId() == Cmd2->getCurrId()) {
    //    if (Cmd2->Cmd_active_CB->GetValue()) {
    //        Cmd2->enable();
    //    }
    //    else {
    //        Cmd2->disable();
    //    }
    //}
    //if (event.GetId() == Cmd2->getCurrId()+1) {
    //    if (Cmd2->Cmd_sequential_CB->GetValue()) {
    //        Cmd2->setSequential(true);
    //    }
    //    else {
    //        Cmd2->setSequential(false);
    //    }
    //}


    //if (event.GetId() == Cmd3->getCurrId()) {
    //    if (Cmd3->Cmd_active_CB->GetValue()) {
    //        Cmd3->enable();
    //    }
    //    else {
    //        Cmd3->disable();
    //    }
    //}
    //if (event.GetId() == Cmd3->getCurrId()+1) {
    //    if (Cmd3->Cmd_sequential_CB->GetValue()) {
    //        Cmd3->setSequential(true);
    //    }
    //    else {
    //        Cmd3->setSequential(false);
    //    }
    //}


    //if (event.GetId() == Cmd4->getCurrId()) {
    //    if (Cmd4->Cmd_active_CB->GetValue()) {
    //        Cmd4->enable();
    //    }
    //    else {
    //        Cmd4->disable();
    //    }
    //}
    //if (event.GetId() == Cmd4->getCurrId()+1) {
    //    if (Cmd4->Cmd_sequential_CB->GetValue()) {
    //        Cmd4->setSequential(true);
    //    }
    //    else {
    //        Cmd4->setSequential(false);
    //    }
    //}


    //if (event.GetId() == Cmd5->getCurrId()) {
    //    if (Cmd5->Cmd_active_CB->GetValue()) {
    //        Cmd5->enable();
    //    }
    //    else {
    //        Cmd5->disable();
    //    }
    //}
    //if (event.GetId() == Cmd5->getCurrId()+1) {
    //    if (Cmd5->Cmd_sequential_CB->GetValue()) {
    //        Cmd5->setSequential(true);
    //    }
    //    else {
    //        Cmd5->setSequential(false);
    //    }
    //}


    //if (event.GetId() == Cmd6->getCurrId()) {
    //    if (Cmd6->Cmd_active_CB->GetValue()) {
    //        Cmd6->enable();
    //    }
    //    else {
    //        Cmd6->disable();
    //    }
    //}
    //if (event.GetId() == Cmd6->getCurrId()+1) {
    //    if (Cmd6->Cmd_sequential_CB->GetValue()) {
    //        Cmd6->setSequential(true);
    //    }
    //    else {
    //        Cmd6->setSequential(false);
    //    }
    //}



    //if (event.GetId() == Cmd7->getCurrId()) {
    //    if (Cmd7->Cmd_active_CB->GetValue()) {
    //        Cmd7->enable();
    //    }
    //    else {
    //        Cmd7->disable();
    //    }
    //}
    //if (event.GetId() == Cmd7->getCurrId()+1) {
    //    if (Cmd7->Cmd_sequential_CB->GetValue()) {
    //        Cmd7->setSequential(true);
    //    }
    //    else {
    //        Cmd7->setSequential(false);
    //    }
    //}



    //if (event.GetId() == Cmd8->getCurrId()) {
    //    if (Cmd8->Cmd_active_CB->GetValue()) {
    //        Cmd8->enable();
    //    }
    //    else {
    //        Cmd8->disable();
    //    }
    //}
    //if (event.GetId() == Cmd8->getCurrId()+1) {
    //    if (Cmd8->Cmd_sequential_CB->GetValue()) {
    //        Cmd8->setSequential(true);
    //    }
    //    else {
    //        Cmd8->setSequential(false);
    //    }
    //}



    //if (event.GetId() == Cmd9->getCurrId()) {
    //    if (Cmd9->Cmd_active_CB->GetValue()) {
    //        Cmd9->enable();
    //    }
    //    else {
    //        Cmd9->disable();
    //    }
    //}
    //if (event.GetId() == Cmd9->getCurrId()+1) {
    //    if (Cmd9->Cmd_sequential_CB->GetValue()) {
    //        Cmd9->setSequential(true);
    //    }
    //    else {
    //        Cmd9->setSequential(false);
    //    }
    //}



    //if (event.GetId() == Cmd10->getCurrId()) {
    //    if (Cmd10->Cmd_active_CB->GetValue()) {
    //        Cmd10->enable();
    //    }
    //    else {
    //        Cmd10->disable();
    //    }
    //}
    //if (event.GetId() == Cmd10->getCurrId()+1) {
    //    if (Cmd10->Cmd_sequential_CB->GetValue()) {
    //        Cmd10->setSequential(true);
    //    }
    //    else {
    //        Cmd10->setSequential(false);
    //    }
    //}


    //if (event.GetId() == Cmd11->getCurrId()) {
    //    if (Cmd11->Cmd_active_CB->GetValue()) {
    //        Cmd11->enable();
    //    }
    //    else {
    //        Cmd11->disable();
    //    }
    //}
    //if (event.GetId() == Cmd11->getCurrId()+1) {
    //    if (Cmd11->Cmd_sequential_CB->GetValue()) {
    //        Cmd11->setSequential(true);
    //    }
    //    else {
    //        Cmd11->setSequential(false);
    //    }
    //}
    */
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

    //Cmd1->enableEditables();
    //Cmd2->enableEditables();
    //Cmd3->enableEditables();
    //Cmd4->enableEditables();
    //Cmd5->enableEditables();
    //Cmd6->enableEditables();
    //Cmd7->enableEditables();
    //Cmd8->enableEditables();
    //Cmd9->enableEditables();
    //Cmd10->enableEditables();
    //Cmd11->enableEditables();
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

    //Cmd1->disableEditables();
    //Cmd2->disableEditables();
    //Cmd3->disableEditables();
    //Cmd4->disableEditables();
    //Cmd5->disableEditables();
    //Cmd6->disableEditables();
    //Cmd7->disableEditables();
    //Cmd8->disableEditables();
    //Cmd9->disableEditables();
    //Cmd10->disableEditables();
    //Cmd11->disableEditables();
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
    /*
        if (!Cmd1->getRunning()) {
            if (Cmd1->isActive()) {
                if (Cmd1->setRunning(true)) {
                    if (Cmd1->isSequential()) {
                        StartThread(Cmd1->getCmd(), 1, true);
                    }
                    else {
                        StartThread(Cmd1->getCmd(), 1, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 1 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 1 already active"));
        }
        wxYield();

        if (!Cmd2->getRunning()) {
            if (Cmd2->isActive()) {
                if (Cmd2->setRunning(true)) {
                    if (Cmd2->isSequential()) {
                        StartThread(Cmd2->getCmd(), 2, true);
                    }
                    else {
                        StartThread(Cmd2->getCmd(), 2, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 2 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 2 already active"));
        }
        wxYield();

        if (!Cmd3->getRunning()) {
            if (Cmd3->isActive()) {
                if (Cmd3->setRunning(true)) {
                    if (Cmd3->isSequential()) {
                        StartThread(Cmd3->getCmd(), 3, true);
                    }
                    else {
                        StartThread(Cmd3->getCmd(), 3, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 3 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 3 already active"));
        }
        wxYield();

        if (!Cmd4->getRunning()) {
            if (Cmd4->isActive()) {
                if (Cmd4->setRunning(true)) {
                    if (Cmd4->isSequential()) {
                        StartThread(Cmd4->getCmd(), 4, true);
                    }
                    else {
                        StartThread(Cmd4->getCmd(), 4, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 4 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 4 already active"));
        }
        wxYield();

        if (!Cmd5->getRunning()) {
            if (Cmd5->isActive()) {
                if (Cmd5->setRunning(true)) {
                    if (Cmd5->isSequential()) {
                        StartThread(Cmd5->getCmd(), 5, true);
                    }
                    else {
                        StartThread(Cmd5->getCmd(), 5, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 5 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 5 already active"));
        }
        wxYield();

        if (!Cmd6->getRunning()) {
            if (Cmd6->isActive()) {
                if (Cmd6->setRunning(true)) {
                    if (Cmd6->isSequential()) {
                        StartThread(Cmd6->getCmd(), 6, true);
                    }
                    else {
                        StartThread(Cmd6->getCmd(), 6, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 6 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 6 already active"));
        }
        wxYield();

        if (!Cmd7->getRunning()) {
            if (Cmd7->isActive()) {
                if (Cmd7->setRunning(true)) {
                    if (Cmd7->isSequential()) {
                        StartThread(Cmd7->getCmd(), 7, true);
                    }
                    else {
                        StartThread(Cmd7->getCmd(), 7, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 7 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 7 already active"));
        }
        wxYield();

        if (!Cmd8->getRunning()) {
            if (Cmd8->isActive()) {
                if (Cmd8->setRunning(true)) {
                    if (Cmd8->isSequential()) {
                        StartThread(Cmd8->getCmd(), 8, true);
                    }
                    else {
                        StartThread(Cmd8->getCmd(), 8, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 8 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 8 already active"));
        }
        wxYield();

        if (!Cmd9->getRunning()) {
            if (Cmd9->isActive()) {
                if (Cmd9->setRunning(true)) {
                    if (Cmd9->isSequential()) {
                        StartThread(Cmd9->getCmd(), 9, true);
                    }
                    else {
                        StartThread(Cmd9->getCmd(), 9, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 9 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 9 already active"));
        }
        wxYield();

        if (!Cmd10->getRunning()) {
            if (Cmd10->isActive()) {
                if (Cmd10->setRunning(true)) {
                    if (Cmd10->isSequential()) {
                        StartThread(Cmd10->getCmd(), 10, true);
                    }
                    else {
                        StartThread(Cmd10->getCmd(), 10, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 10 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 10 already active"));
        }
        wxYield();

        if (!Cmd11->getRunning()) {
            if (Cmd11->isActive()) {
                if (Cmd11->setRunning(true)) {
                    if (Cmd11->isSequential()) {
                        StartThread(Cmd11->getCmd(), 11, true);
                    }
                    else {
                        StartThread(Cmd11->getCmd(), 11, false);
                    }
                }
                else {
                    AddMessage(get_current_timestamp(), wxString::Format("Couldn't set the CMD 11 gui to BUSY"));
                }
            }
        }
        else {
            AddMessage(get_current_timestamp(), wxString::Format("Thread for CMD 11 already active"));
        }
        wxYield();
    }
    */
}

void MainWindow::StartThread(const wxString& input, int CommandIndex, bool sequentialStatus) {
    // Start a background thread
    /* Changed to no blocking and cycling into a while not finished loop with wxYield() inside (check calling function)
    if (sequentialStatus != SEQUENTIAL) {
        std::thread([this, input, CommandIndex]() {
            std::string result = RunCommand(input.ToStdString());

            wxYield();
            // Create and post an event to the main thread
            wxCommandEvent event(wxEVT_THREAD_RESULT);
            event.SetString(result);
            event.SetInt(CommandIndex);
            wxQueueEvent(this, event.Clone());
            }).detach();  // Detach so the thread runs independently
    }else{
    */
    std::thread([this, input, CommandIndex]() {
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
std::string MainWindow::RunCommand(const std::string& command, int commandIndex) {

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
    char buffer[512];
    DWORD bytesRead;

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        wxYield();
        buffer[bytesRead] = '\0';
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
            arrayOfGuiCMDs[i]->setRunning(false);
            //Checking if there was an active blocking thread if yes then checking if the result came from the command who 
            //called the blocking thread, if yes than releasing the block
            if (blockingThread && (blockingCommandIndex == i)) {
                blockingThread = false;
            }
            wxString MessageToAdd = wxString::Format("Cmd %d result is: " + event.GetString(), event.GetInt()+1);
            AddMessage(get_current_timestamp(), MessageToAdd);
        }
    }


    /*
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
    */
}