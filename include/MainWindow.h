#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <string>
class MainWindow : public wxFrame
{
public:
    MainWindow();
    wxPanel* mainPanel;
    wxBoxSizer* resultsSizer;
    wxButton* runBT;
    wxListCtrl* resultList;
private:

    HANDLE hOutputReadPipe;
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void onRunCommand(wxCommandEvent& event);
    int runCommand(const wxString& command);
    void AddMessage(const wxString& timestamp, const wxString& message); 
    wxString get_current_timestamp();  
    void RunCommand(const wxString& command);
    void ProcessOutput();
};

