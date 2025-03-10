// Start of wxWidgets "Hello World" Program
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

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Info");

    SetMenuBar(menuBar);

    CreateStatusBar();

    Bind(wxEVT_MENU, &MainWindow::OnHello, this, windowIDs::ID_Hello);
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_BUTTON, &MainWindow::onRunCommand, this, windowIDs::ID_RUN_COMMAND_BT);
    Bind(wxEVT_THREAD_RESULT, &MainWindow::OnThreadResult, this);

    SetStatusText("Command Runner!");
    resultsSizer = new wxBoxSizer(wxVERTICAL);
    resultList = new wxListCtrl(this, windowIDs::ID_COMMAND_LIST, wxDefaultPosition, wxSize(950,350), wxLC_REPORT | wxLC_SINGLE_SEL);

    // Add columns for timestamp and message
    resultList->InsertColumn(0, "Timestamp");
    resultList->InsertColumn(1, "Message");

    runBT = new wxButton(this, windowIDs::ID_RUN_COMMAND_BT, "Run command", wxDefaultPosition, wxDefaultSize );
    // Add messages to the list (with timestamp and message content)
    //AddMessage(get_current_timestamp(), "Hello, this is the first message.");
    //AddMessage(get_current_timestamp(), "Second message appears here.");
    // Get the current frame size (including the border)
    int width, height;
    GetSize(&width, &height);

    // Set the proportions: Message is 5 times the width of Timestamp
    int timestampWidth = width / 6;   // 1 part of the total 6 parts (timestamp + 5 * message)
    int messageWidth = timestampWidth * 5; // 5 parts for message

    // Set the column widths
    resultList->SetColumnWidth(0, timestampWidth); // Timestamp column
    resultList->SetColumnWidth(1, messageWidth);   // Message column

    resultsSizer->Add(runBT, 1, wxEXPAND | wxALL, 3);
    resultsSizer->Add(resultList, 5, wxEXPAND | wxALL, 3);
    SetSizerAndFit(resultsSizer);
}

void MainWindow::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void MainWindow::OnAbout(wxCommandEvent& event)
{
    wxString ToPrint = "This APP can multiple \"*.bat\" commands in separated threads (aka simultaneously)\n";
    ToPrint += "Results of each execution will be shown as separated messages in the list\n\nDev. Coga F. (EGO Group S.r.L @ fation.coga@egogroup.eu)";
    wxMessageBox(ToPrint, "About PCR", wxOK | wxICON_INFORMATION);
}

void MainWindow::OnHello(wxCommandEvent& event)
{
    wxLogMessage("wxWidgets 3.2.6 Inside");
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

// Function to add message to the list
void MainWindow::AddMessage(const wxString& timestamp, const wxString& message) {
    // Insert the new message at the top
    long index = resultList->InsertItem(0, timestamp);
    resultList->SetItem(index, 1, message);
}

void MainWindow::onRunCommand(wxCommandEvent& event)
{
    //wxString seggerSoftwareLocation = "\"C:\\Program Files\\SEGGER\\JLink\\Cimbali\\MasterUpdate_fromSlave.bat\"";
    wxString MasterFromMaster = "\"C:\\Program Files\\SEGGER\\JLink\\Cimbali\\MasterUpdate_fromMaster.bat\"";
    wxString MasterFromSlave = "\"C:\\Program Files\\SEGGER\\JLink\\Cimbali\\MasterUpdate_fromSlave.bat\"";
    //AddMessage(get_current_timestamp(), MasterFromMaster);
    //AddMessage(get_current_timestamp(), MasterFromSlave);
    StartThread(MasterFromMaster);
    StartThread(MasterFromSlave);
}

void MainWindow::StartThread(const wxString& input) {
    // Start a background thread
    std::thread([this, input]() {
        std::string result = RunCommand(input.ToStdString());

        // Create and post an event to the main thread
        wxCommandEvent event(wxEVT_THREAD_RESULT);
        event.SetString(result);
        wxQueueEvent(this, event.Clone());
        }).detach();  // Detach so the thread runs independently
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
    //si.wShowWindow = SW_HIDE;  // Hides the window
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;  // Redirect stderr to stdout

    // Create the process
    if (!CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
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
    if (event.GetString().Find("PASSED Programming through MASTER with FW MASTER") != wxNOT_FOUND){
        PassedFromMaster++;
    }
    else if (event.GetString().Find("FAILED Programming through MASTER with FW MASTER") != wxNOT_FOUND) {
        FailedFromMaster++;
    }
    else if (event.GetString().Find("PASSED Programming through SLAVE with FW MASTER") != wxNOT_FOUND) {
        PassedFromSlave++;
    }
    else if (event.GetString().Find("FAILED Programming through SLAVE with FW MASTER") != wxNOT_FOUND) {
        FailedFromSlave++;
    }
    //AddMessage(get_current_timestamp(), event.GetString());
    wxString countersTrack = wxString::Format("PASSED  Master=%03d Slave=%03d ------ FAILED  Master=%03d Slave=%03d",
        PassedFromMaster, PassedFromSlave, FailedFromMaster, FailedFromSlave);
    AddMessage(get_current_timestamp(), countersTrack);
}