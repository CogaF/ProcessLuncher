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
    wxPanel*        mainPanel;
    //definition of six sizers in order to keep all window comoponents of each command properties organized
    wxBoxSizer*     _1_Cmd_sz;
    wxBoxSizer*     _2_Cmd_sz;
    wxBoxSizer*     _3_Cmd_sz;
    wxBoxSizer*     _4_Cmd_sz;
    wxBoxSizer*     _5_Cmd_sz;
    wxBoxSizer*     _6_Cmd_sz;
    //Definition of checkboxes in order to select it one command should be activated or not
    wxCheckBox*     _1_Cmd_CB;
    wxCheckBox*     _2_Cmd_CB;
    wxCheckBox*     _3_Cmd_CB;
    wxCheckBox*     _4_Cmd_CB;
    wxCheckBox*     _5_Cmd_CB;
    wxCheckBox*     _6_Cmd_CB;
    //definition of textcontrols where the exact name of the command to run or the address of the *.bat file
    wxTextCtrl*     _1_Cmd_txt;
    wxTextCtrl*     _2_Cmd_txt;
    wxTextCtrl*     _3_Cmd_txt;
    wxTextCtrl*     _4_Cmd_txt;
    wxTextCtrl*     _5_Cmd_txt;
    wxTextCtrl*     _6_Cmd_txt;
    //definition of textcontrols where the app shall consider as success from the result returned from the command/bat 
    wxTextCtrl*     _1_Cmd_Res;
    wxTextCtrl*     _2_Cmd_Res;
    wxTextCtrl*     _3_Cmd_Res;
    wxTextCtrl*     _4_Cmd_Res;
    wxTextCtrl*     _5_Cmd_Res;
    wxTextCtrl*     _6_Cmd_Res;
    //definition os static text where the user can read the overall results for each command (aka passes and fails)
    wxStaticText*   _1_Cmd_counters;
    wxStaticText*   _2_Cmd_counters;
    wxStaticText*   _3_Cmd_counters;
    wxStaticText*   _4_Cmd_counters;
    wxStaticText*   _5_Cmd_counters;
    wxStaticText*   _6_Cmd_counters;
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