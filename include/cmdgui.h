/*
	This is an application that was created by me Coga Fation but truth be told
	that without the help of ChatGpt it could have taken many many hours
	I'm taking for granted that I would have reached the same results which is a long shot
*/

#pragma once
#include <wx/wx.h>
#include <Id.h>
class cmdgui
{
private:
	wxBoxSizer*		Cmd_sz;
	wxCheckBox*		Cmd_running_CB;
	wxTextCtrl*		Cmd_txt;
	wxTextCtrl*		Cmd_res;
	wxStaticText*	Cmd_counters;
	wxButton*		Cmd_run_bt;

	wxString _cmdName = "ping google.com";
	wxString _positiveVal = ":File:C:\\temp\\result.txt::StringPASS_InFile";
	wxString _counters = "";
	bool _isActive = true;
	bool _isSequential = false;
	int _thisId = windowIDs::ID_GUI_CLASS;
	int _indexOfGui = 0;
	bool _guiExists = false;
	bool _isRunning = false;
	bool _isViewShow = false;
	bool _isPass = false;

public:

	int SEQUENTIAL_ID_INDEX = 1;
	int RUNBUTTON_ID_INDEX = 2;
	int TEXT_ID_INDEX = 3;
	int RES_ID_INDEX = 4;
	int COUNTERS_ID_INDEX = 5;
	int VIEW_ID_INDEX = 6;

	wxCheckBox* Cmd_active_CB;
	wxCheckBox* Cmd_sequential_CB;
	wxCheckBox* Cmd_view_CB;
	cmdgui(wxPanel* parentPanel);
	cmdgui(wxPanel* parentPanel, int guiIndex);
	cmdgui(wxPanel* parentPanel, int guiIndex, bool isActive, bool sequential, wxString cmdName, wxString positiveValue, wxString counters_s);
	~cmdgui();
	bool isActive();
	bool isSequential();
	wxString getCmd();
	wxString getPostiveVal();
	bool setCounters(wxString countersString);
	void buildDefault(int indexOfGuid);
	bool setSequential(bool sequentialStatus);
	bool setActive(bool activeStatus);
	bool setView(bool activeStatus);
	bool getView();
	bool setCmd(wxString cmdName);
	bool setPostVal(wxString positiveValue);
	bool setCurrIds(int currId);
	int getCurrId(); 
	bool update();
	bool setRunning(bool runningStatus);
	bool getRunning();
	void disable();
	void enable();
	void disableEditables();
	void enableEditables();
	void setResult(bool result);
	bool getResukt();
	wxBoxSizer* getPointer();
};

