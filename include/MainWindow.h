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
#include <cmdgui.h>
class MainWindow : public wxFrame
{
public:
    MainWindow();
    wxPanel*        mainPanel;
    cmdgui*         Cmd1;
    cmdgui*         Cmd2;
    cmdgui*         Cmd3;
    cmdgui*         Cmd4;
    cmdgui*         Cmd5;
    cmdgui*         Cmd6;
    //Main Sizer that will contain all components
    wxBoxSizer* mainSizer;
    wxBoxSizer* componentsSizer;
    //Sizer for commands for now contains only run button
    wxBoxSizer*     cmdsSizer;
    //sizer for the list
    wxBoxSizer*     resultsSizer;
    //The RUN button
    wxButton*       runBT;
    //The list (immagine this saying with the voice of John Rese in POI on espisode 1 of season 1 when he understands what Finch means, although this has nothing to do with that list)
    wxListCtrl*     resultList;
    //Tracking number of passes and fails for each command
    int passes_cmd1 = 0;
    int failed_cmd1 = 0;
    int passes_cmd2 = 0;
    int failed_cmd2 = 0;
    int passes_cmd3 = 0;
    int failed_cmd3 = 0;
    int passes_cmd4 = 0;
    int failed_cmd4 = 0;
    int passes_cmd5 = 0;
    int failed_cmd5 = 0;
    int passes_cmd6 = 0;
    int failed_cmd6 = 0; 
    //Value that will be used to keep track of threads that have finished their task
    //in order to prevend running newthread while previous ones aren't finished yet
    int finishedThreads = 0;
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