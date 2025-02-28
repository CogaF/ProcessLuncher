// Start of wxWidgets "Hello World" Program
#include <wx/wx.h>
#include "MainWindow.h"
#include "App.h"
#include "Id.h"

using namespace std;
using namespace windowIDs;



MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "Hello World")
{
    wxSize frameSize(950, 550);
    wxFrame::SetSize(frameSize);
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();

    Bind(wxEVT_MENU, &MainWindow::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MainWindow::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainWindow::OnExit, this, wxID_EXIT);
    Bind(wxEVT_BUTTON, &MainWindow::onRunCommand, this, ID_RUN_COMMAND_BT);

    SetStatusText("Welcome to wxWidgets!");
    resultsSizer = new wxBoxSizer(wxVERTICAL);
    resultList = new wxListCtrl(this, ID_COMMAND_LIST, wxDefaultPosition, wxSize(950,350), wxLC_REPORT | wxLC_SINGLE_SEL);

    // Add columns for timestamp and message
    resultList->InsertColumn(0, "Timestamp");
    resultList->InsertColumn(1, "Message");

    runBT = new wxButton(this, ID_RUN_COMMAND_BT, "Run command", wxDefaultPosition, wxDefaultSize );
        // Add messages to the list (with timestamp and message content)
    AddMessage(get_current_timestamp(), "Hello, this is the first message.");
    AddMessage(get_current_timestamp(), "Second message appears here.");
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
void MainWindow::onRunCommand(wxCommandEvent& event)
{
   RunCommand("ping google.com");;
}

void MainWindow::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello World", wxOK | wxICON_INFORMATION);
}

void MainWindow::OnHello(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}  
// Function to add message to the list
void MainWindow::AddMessage(const wxString& timestamp, const wxString& message) {
    // Insert the new message at the top
    long index = resultList->InsertItem(0, timestamp);
    resultList->SetItem(index, 1, message);
}

// Function to run the command and capture output
void MainWindow::RunCommand(const wxString& command) {
    // Setup for CreateProcess
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;  // Use show window flags
    si.wShowWindow = SW_HIDE;  // Hide the window

    // Redirect output (stdout) to a pipe
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hOutputReadPipe, &si.hStdOutput, &sa, 0)) {
        wxLogError("Failed to create pipe.");
        return;
    }

    si.hStdError = si.hStdOutput;

    // Launch the process (e.g., cmd.exe /c ping google.com)
    std::wstring cmd = L"cmd.exe /c " + std::wstring(command.begin(), command.end());
    if (CreateProcess(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(si.hStdOutput);  // Close the write end of the pipe

        // Create a thread to handle reading the process output
        CreateThread(NULL, 0,
            [](LPVOID lpParam) -> DWORD {
                MainWindow* frame = (MainWindow*)lpParam;
                frame->ProcessOutput();
                return 0;
            },
            (LPVOID)this, 0, NULL);

        // Wait for the process to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Cleanup
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        wxLogError("Failed to launch process.");
    }
}


// Non-static function to handle output from the process
void MainWindow::ProcessOutput() {
    char buffer[4096];
    DWORD bytesRead;

    // Read the output from the pipe
    while (ReadFile(hOutputReadPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';  // Null terminate the buffer

        // Insert output into wxListCtrl (in main thread)
        wxGetApp().GetTopWindow()->CallAfter([buffer]() {
            MainWindow* frame = dynamic_cast<MainWindow*>(wxGetApp().GetTopWindow());
            if (frame) {
                frame->resultList->InsertItem(frame->resultList->GetItemCount(), wxString(buffer, wxConvUTF8));
            }
            });
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