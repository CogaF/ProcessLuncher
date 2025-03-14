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
	wxCheckBox*		Cmd_active_CB;
	wxCheckBox*		Cmd_Sequential_CB;
	wxTextCtrl*		Cmd_txt;
	wxTextCtrl*		Cmd_Res;
	wxStaticText*	Cmd_counters;

	wxString _cmdName = "ping google.com";
	wxString _positiveVal = "result for positive evaluation";
	wxString _counters = "";
	bool _isActive = true;
	bool _isSequential = false;
	int _thisId = windowIDs::ID_GUI_CLASS;
	bool _guiExists = false;

public:
	cmdgui(wxPanel* parentPanel);
	cmdgui(wxPanel* parentPanel, int currId);
	cmdgui(wxPanel* parentPanel, int currId, bool isActive, bool sequential, wxString cmdName, wxString positiveValue, wxString counters_s);
	~cmdgui();
	bool isActive();
	bool isSequential();
	wxString getCmd();
	wxString getPostiveVal();
	bool setCounters(wxString countersString);
	bool buildDefault();
	bool setSequential(bool sequentialStatus);
	bool setActive(bool activeStatus);
	bool setCmd(wxString cmdName);
	bool setPostVal(wxString positiveValue);
	bool setCurrIds(int currId);
	int getCurrId(); 
	bool update();
	wxBoxSizer* getPointer();
};

