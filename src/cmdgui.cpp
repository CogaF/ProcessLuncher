/*
	This is an application that was created by me Coga Fation but truth be told
	that without the help of ChatGpt it could have taken many many hours
	I'm taking for granted that I would have reached the same results which is a long shot
*/

#include "cmdgui.h"

cmdgui::cmdgui(wxPanel* parentPanel) {
	Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
	Cmd_active_CB = new wxCheckBox();
	Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
	Cmd_sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
	Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
	Cmd_res = new wxTextCtrl(parentPanel, wxID_ANY, "");
	Cmd_running_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
	Cmd_running_CB->Disable();
	Cmd_running_CB->SetValue(false);
	Cmd_counters = new wxStaticText(parentPanel, wxID_ANY, "");
	buildDefault();
}

cmdgui::cmdgui(wxPanel* parentPanel, int currId) {
	if (!_guiExists) {

		Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
		Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY,"");
		Cmd_running_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_res = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_running_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_running_CB->Disable();
		Cmd_running_CB->SetValue(false);
		Cmd_counters = new wxStaticText(parentPanel,wxID_ANY,"");
	}
	_thisId = currId;
	update();
}

cmdgui::cmdgui(wxPanel* parentPanel, int currId, bool isActive, bool sequential, wxString cmdName, wxString positiveValue, wxString counters_s) {
	if (!_guiExists) {

		Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
		Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_res = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_running_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_running_CB->Disable();
		Cmd_running_CB->SetValue(false);
		Cmd_counters = new wxStaticText(parentPanel, wxID_ANY, "");
	}
	_thisId = currId;
	_isActive = isActive;
	_isSequential = sequential;
	_cmdName = cmdName;
	_positiveVal = positiveValue;
	_counters = counters_s;
	_isRunning = false;
	update();

}

wxBoxSizer* cmdgui::getPointer() {
	return *&Cmd_sz;
}

bool cmdgui::isActive() {
	_isActive = Cmd_active_CB->GetValue();
	return _isActive;
}

bool cmdgui::isSequential() {
	_isSequential = Cmd_sequential_CB->GetValue();
	return _isSequential;
}

wxString cmdgui::getCmd() {
	_cmdName = Cmd_txt->GetValue();
	return _cmdName;
}

wxString cmdgui::getPostiveVal() {
	_positiveVal = Cmd_res->GetValue();
	return _positiveVal;
}

bool cmdgui::setCounters(wxString countersString) {
	_counters = countersString;
	if (Cmd_counters != nullptr) {
		Cmd_counters->SetLabel(_counters);
		return true;
	}
	return false;
}

bool cmdgui::buildDefault() {

	Cmd_active_CB->SetValue(true);
	Cmd_active_CB->SetLabel("Active?");
	Cmd_sequential_CB->SetValue(false);
	Cmd_sequential_CB->SetLabel("Sequential?");
	Cmd_txt->SetValue(_cmdName);
	Cmd_res->SetValue(_positiveVal);
	Cmd_running_CB->SetLabel("Ready");
	Cmd_running_CB->SetValue(false);
	Cmd_counters->SetLabel(_counters);
	if (!setCurrIds(_thisId)) {
		_guiExists = false;
		return false;
	}
	if (!_guiExists) {
		Cmd_sz->Add(Cmd_active_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_sequential_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_txt, 7, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_res, 4, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_running_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_counters, 2, wxEXPAND | wxALL, 1);
	}
	_guiExists = true;
	return true;
}


bool cmdgui::update() {

	Cmd_active_CB->SetValue(_isActive);
	Cmd_active_CB->SetLabel("Active?");
	Cmd_sequential_CB->SetValue(_isSequential);
	Cmd_sequential_CB->SetLabel("Sequential?");
	Cmd_txt->SetValue(_cmdName);
	Cmd_res->SetValue(_positiveVal);

	Cmd_running_CB->SetValue(_isRunning);
	if (_isRunning) {
		Cmd_running_CB->SetLabel("Busy");
	}
	else {
		Cmd_running_CB->SetLabel("Ready");
	}

	Cmd_counters->SetLabel(_counters);
	if (!setCurrIds(_thisId)) {
		_guiExists = false;
		return false;
	}
	if (!_guiExists) {
		Cmd_sz->Add(Cmd_active_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_sequential_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_txt, 7, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_res, 4, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_running_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_counters, 2, wxEXPAND | wxALL, 1);
	}
	_guiExists = true;
	return true;
}

bool cmdgui::setSequential(bool sequentialStatus) {
	_isSequential = sequentialStatus;
	if (Cmd_sequential_CB != nullptr) {
		Cmd_sequential_CB->SetValue(_isSequential);
		return true;
	}
	return false;
}

bool cmdgui::setRunning(bool runninglStatus) {
	_isRunning = runninglStatus;
	if (Cmd_running_CB != nullptr) {
		Cmd_running_CB->SetValue(_isRunning);
		if (_isRunning) {
			Cmd_running_CB->SetLabel("Busy");
		}
		else {
			Cmd_running_CB->SetLabel("Ready");
		}
		return true;
	}
	return false;
}

bool cmdgui::getRunning() {
	return _isRunning;
}

bool cmdgui::setActive(bool activeStatus) {
	_isActive = activeStatus;
	if (Cmd_active_CB != nullptr) {
		Cmd_active_CB->SetValue(_isActive);
		return true;
	}
	return false;
}

bool cmdgui::setCmd(wxString cmdName) {
	_cmdName = cmdName;
	if (Cmd_txt != nullptr) {
		Cmd_txt->SetValue(_cmdName);
		return true;
	}
	return false;
}

bool cmdgui::setPostVal(wxString positiveValue) {
	_positiveVal = positiveValue;
	if (Cmd_res != nullptr) {
		Cmd_res->SetValue(positiveValue);
		return true;
	}
	return false;

}

bool cmdgui::setCurrIds(int currId) {
	_thisId = currId;
	if (Cmd_active_CB != nullptr &&
		Cmd_sequential_CB != nullptr &&
		Cmd_txt != nullptr &&
		Cmd_res != nullptr &&
		Cmd_counters != nullptr) {
		Cmd_active_CB->SetId(_thisId );
		Cmd_sequential_CB->SetId(_thisId + 1);
		Cmd_txt->SetId(_thisId + 2);
		Cmd_res->SetId(_thisId + 3);
		Cmd_counters->SetId(_thisId + 4);
		return true;
	}
	return false;
}

int cmdgui::getCurrId() {
	return _thisId;
}
