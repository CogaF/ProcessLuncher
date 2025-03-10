#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <string>
#include <wx/event.h>
#include <thread>
#include <future>
#include "App.h"
#include "Id.h"
#include <cstdio>
#include <memory>
class MainWindow : public wxFrame
{
public:
    MainWindow();
    wxPanel* mainPanel;
    wxBoxSizer* resultsSizer;
    wxButton* runBT;
    wxListCtrl* resultList;

    int PassedFromMaster = 0;
    int PassedFromSlave = 0;
    int FailedFromMaster = 0;
    int FailedFromSlave = 0;
private:

    HANDLE hOutputReadPipe;
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void onRunCommand(wxCommandEvent& event);
    void AddMessage(const wxString& timestamp, const wxString& message);
    wxString get_current_timestamp();
    std::string RunCommand(const std::string& command);
    void StartThread(const wxString& input);
    void OnThreadResult(wxCommandEvent& event);

};

wxDEFINE_EVENT(wxEVT_THREAD_RESULT, wxCommandEvent);