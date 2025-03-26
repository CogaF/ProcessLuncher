/*
	This is an application that was created by me Coga Fation but truth be told
	that without the help of ChatGpt it could have taken many many hours
	I'm taking for granted that I would have reached the same results which is a long shot
*/

#include "cmdgui.h"

cmdgui::cmdgui(wxPanel* parentPanel) {
	if (!_guiExists) {
		Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
		Cmd_run_bt = new wxButton(parentPanel, wxID_ANY, "Run");
		Cmd_run_bt->Enable();
		Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_res = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_counters = new wxStaticText(parentPanel, wxID_ANY, "");
		Cmd_sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_view_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_view_CB->Disable();
		Cmd_running_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_running_CB->Disable();
		Cmd_running_CB->SetValue(false);
	}
	buildDefault(0);
}

cmdgui::cmdgui(wxPanel* parentPanel, int guiIndex) {
	if (!_guiExists) {
		Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
		Cmd_run_bt = new wxButton(parentPanel, wxID_ANY, "Run");
		Cmd_run_bt->Enable();
		Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_res = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_counters = new wxStaticText(parentPanel, wxID_ANY, "");
		Cmd_sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_view_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_view_CB->Disable();
		Cmd_running_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_running_CB->Disable();
		Cmd_running_CB->SetValue(false);
	}
	//_thisId = guiIndex;
	buildDefault(guiIndex);
	//update();
}


void cmdgui::setResult(bool result) {
	_isPass = result;
}
bool cmdgui::getResukt() {
	return _isPass;
}

void cmdgui::disable() {
	Cmd_sequential_CB->Disable();
	Cmd_txt->Disable();
	Cmd_res->Disable();
	Cmd_counters->Disable();
	Cmd_view_CB->Disable();
	setActive(false);
}

void cmdgui::enable() {
	Cmd_sequential_CB->Enable();
	Cmd_txt->Enable();
	Cmd_res->Enable();
	Cmd_counters->Enable();
	//Cmd_view_CB->Enable();
	setActive(true);
}

void cmdgui::disableEditables() {
	Cmd_sequential_CB->Disable();
	Cmd_txt->Disable();
	Cmd_res->Disable();
	Cmd_view_CB->Disable();
}

void cmdgui::enableEditables() {
	if (_isActive) {
		Cmd_sequential_CB->Enable();
		Cmd_txt->Enable();
		Cmd_res->Enable();
		//Cmd_view_CB->Enable();
	}
}

cmdgui::cmdgui(wxPanel* parentPanel, int currId, bool isActive, bool sequential, wxString cmdName, wxString positiveValue, wxString counters_s) {
	if (!_guiExists) {

		Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
		Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_run_bt = new wxButton(parentPanel, wxID_ANY, "Run");
		Cmd_run_bt->Enable();
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

void cmdgui::buildDefault(int guiIndex) {

	Cmd_active_CB->SetValue(true);
	Cmd_active_CB->SetLabel("ON");
	Cmd_run_bt->Enable();
	Cmd_txt->SetValue(_cmdName);
	Cmd_res->SetValue(_positiveVal);
	Cmd_counters->SetLabel(_counters);
	Cmd_sequential_CB->SetValue(false);
	Cmd_sequential_CB->SetLabel("Parallel");
	Cmd_view_CB->SetValue(false);
	Cmd_view_CB->SetLabel("Hide ");
	Cmd_running_CB->SetValue(false);
	Cmd_running_CB->SetLabel("Ready");

	if (!setCurrIds(guiIndex)) {
		_guiExists = false;
	}

	if (!_guiExists) {
		Cmd_sz->Add(Cmd_active_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_run_bt, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_txt, 5, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_res, 3, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_counters, 3, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_sequential_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_view_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_running_CB, 1, wxEXPAND | wxALL, 1);
	}
	_guiExists = true;
}

bool cmdgui::update() {

	Cmd_active_CB->SetValue(_isActive);
	if (_isActive) {
		Cmd_active_CB->SetLabel("ON");
	}
	else {
		Cmd_active_CB->SetLabel("OFF");
	}
	Cmd_sequential_CB->SetValue(_isSequential);
	if (_isSequential) {
		Cmd_sequential_CB->SetLabel("Single");
	}
	else {
		Cmd_sequential_CB->SetLabel("Multip.");
	}
	Cmd_txt->SetValue(_cmdName);
	Cmd_res->SetValue(_positiveVal);

	Cmd_running_CB->SetValue(_isRunning);
	if (_isRunning) {
		Cmd_running_CB->SetLabel("Busy");
		Cmd_run_bt->Disable();
	}
	else {
		Cmd_running_CB->SetLabel("Ready");
		Cmd_run_bt->Enable();
	}

	Cmd_active_CB->SetValue(_isViewShow);
	if (_isViewShow) {
		Cmd_view_CB->SetLabel("Show");
	}
	else {
		Cmd_view_CB->SetLabel("Hide ");
	}

	Cmd_counters->SetLabel(_counters);
	if (!setCurrIds(_thisId)) {
		_guiExists = false;
		return false;
	}
	if (!_guiExists) {
		Cmd_sz->Add(Cmd_active_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_run_bt, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_txt, 7, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_res, 4, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_counters, 2, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_sequential_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_view_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_running_CB, 1, wxEXPAND | wxALL, 1);
	}
	_guiExists = true;
	return true;
}

bool cmdgui::setSequential(bool sequentialStatus) {
	_isSequential = sequentialStatus;
	if (Cmd_sequential_CB != nullptr) {
		Cmd_sequential_CB->SetValue(_isSequential);
		if (_isSequential) {
			Cmd_sequential_CB->SetLabel("Single");
		}
		else {
			Cmd_sequential_CB->SetLabel("Parallel");
		}
		return true;
	}
	return false;
}

bool cmdgui::setRunning(bool runningStatus) {
	_isRunning = runningStatus;
	if (Cmd_running_CB != nullptr) {
		Cmd_running_CB->SetValue(_isRunning);
		if (_isRunning) {
			Cmd_running_CB->SetLabel("Busy");
			Cmd_run_bt->Disable();
		}
		else {
			Cmd_running_CB->SetLabel("Ready");
			Cmd_run_bt->Enable();
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
		if (_isActive) { 
			Cmd_active_CB->SetLabel("ON");
			Cmd_run_bt->Enable();
		}
		else {
			Cmd_active_CB->SetLabel("OFF");
			Cmd_run_bt->Disable();
		}
		return true;
	}
	return false;
}

bool cmdgui::getView() {
	return _isViewShow;
}

bool cmdgui::setView(bool viewStatus) {
	_isViewShow = viewStatus;
	if (Cmd_view_CB != nullptr) {
		Cmd_view_CB->SetValue(_isViewShow);
		if (_isViewShow) {
			Cmd_view_CB->SetLabel("Show");
		}
		else {
			Cmd_view_CB->SetLabel("Hide ");
		}
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

bool cmdgui::setCurrIds(int guiIndex) {
	_thisId += guiIndex * 10;
	if (Cmd_active_CB != nullptr &&
		Cmd_sequential_CB != nullptr &&
		Cmd_txt != nullptr &&
		Cmd_res != nullptr &&
		Cmd_counters != nullptr) {

		Cmd_active_CB->SetId(_thisId );
		Cmd_sequential_CB->SetId(_thisId + SEQUENTIAL_ID_INDEX);
		Cmd_run_bt->SetId(_thisId + RUNBUTTON_ID_INDEX);
		Cmd_txt->SetId(_thisId + TEXT_ID_INDEX);
		Cmd_res->SetId(_thisId + RES_ID_INDEX);
		Cmd_counters->SetId(_thisId + COUNTERS_ID_INDEX);
		Cmd_view_CB->SetId(_thisId + VIEW_ID_INDEX);
		return true;
	}
	return false;
}

int cmdgui::getCurrId() {
	return _thisId;
}
