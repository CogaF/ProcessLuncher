/*
    This is an application that was created by me, Coga Fation but truth be told 
    that without the help of ChatGpt it could have taken many, many hours 
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
#include "../resource.h"
#include <fstream>
#include <wx/filename.h> 
#include <wx/filename.h> 
#include <wx/stdpaths.h>
#include <wx/valnum.h>
#include <wx/textfile.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/file.h>

#define SEQUENTIAL true

//more than 25 means that there will be needed a display with many pixels/resolution in the y direction since the 
//result list is on the bottom of the commands
#define MaxNrOfCMDs 25

class MainWindow : public wxFrame
{
public:
    MainWindow();
    wxString        searchOnFIle_s = ":File:";
    wxString        separator = "::";
    int             nrOfCMDs = 11;
    bool            stopAtSequential = false;
    bool            blockingThread = false;
    int             blockingCommandIndex = -1;
    wxPanel*        mainPanel;
    cmdgui*         arrayOfGuiCMDs[MaxNrOfCMDs];
    wxBoxSizer*     arrayOfGuiCMDs_sz[MaxNrOfCMDs];
    int             arrayOfPassed[MaxNrOfCMDs];
    int             arrayOfFailed[MaxNrOfCMDs];
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
    wxString ArrayOfresponses[MaxNrOfCMDs];
    
private:

    HANDLE hOutputReadPipe;
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void onRunCommand(wxCommandEvent& event);
    void AddMessage(const wxString& timestamp, const wxString& message);
    wxString get_current_timestamp();
    std::string RunCommand(const std::string& command, int commandIndex);
    std::string RunCommandTest(const std::string& command, int commandIndex);
    void StartThread(const wxString& input, int commandIndex, bool sequentialStatus);
    void OnThreadResult(wxCommandEvent& event); 
    void OnKeyDown(wxKeyEvent& event);
    void CopySelectedRow(); 
    void OnMouseMove(wxMouseEvent& event);
    void SelectAllItems();
    void OnEnable(wxCommandEvent& event);
    void OnDisable(wxCommandEvent& event);
    void OnGuiEvent(wxCommandEvent& event);
    void OnButtonEvent(wxCommandEvent& event);
    void EnableCmds();
    void DisableCmds();
    void OnClose(wxCloseEvent& event);
    wxString extractFileName(wxString completeString);
    wxString extractPositiveResult(wxString completeString);
    bool FindInFile(const wxString& filePath, const wxString& searchString);
};

wxDEFINE_EVENT(wxEVT_THREAD_RESULT, wxCommandEvent);