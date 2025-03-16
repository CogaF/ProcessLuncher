/*
    This is an application that was created by me Coga Fation but truth be told 
    that without the help of ChatGpt it could have taken many many hours 
    I'm taking for granted that I would have reached the same results which is a long shot
*/
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
#include <wx/clipbrd.h>

#define SEQUENTIAL true

class MainWindow : public wxFrame
{
public:
    MainWindow();
    bool stopAtSequential = false;
    wxPanel*        mainPanel;
    cmdgui*         Cmd1;
    cmdgui*         Cmd2;
    cmdgui*         Cmd3;
    cmdgui*         Cmd4;
    cmdgui*         Cmd5;
    cmdgui*         Cmd6;
    cmdgui*         Cmd7;
    cmdgui*         Cmd8;
    cmdgui*         Cmd9;
    cmdgui*         Cmd10;
    cmdgui*         Cmd11;
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
    int passes_cmd7 = 0;
    int failed_cmd7 = 0;
    int passes_cmd8 = 0;
    int failed_cmd8 = 0;
    int passes_cmd9 = 0;
    int failed_cmd9 = 0;
    int passes_cmd10 = 0;
    int failed_cmd10 = 0;
    int passes_cmd11 = 0;
    int failed_cmd11 = 0;
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
    void StartThread(const wxString& input, int commandIndex, bool sequentialStatus);
    void OnThreadResult(wxCommandEvent& event); 
    void OnKeyDown(wxKeyEvent& event);
    void CopySelectedRow(); 
    void OnMouseMove(wxMouseEvent& event);
    void SelectAllItems();
    void OnEnable(wxCommandEvent& event);
    void OnDisable(wxCommandEvent& event);
    void OnCheckBoxEvent(wxCommandEvent& event);
    void EnableCmds();
    void DisableCmds();
};

wxDEFINE_EVENT(wxEVT_THREAD_RESULT, wxCommandEvent);