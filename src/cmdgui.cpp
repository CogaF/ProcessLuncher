#include "cmdgui.h"

cmdgui::cmdgui(wxPanel* parentPanel) {
	Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
	Cmd_active_CB = new wxCheckBox();
	Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
	Cmd_Sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
	Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
	Cmd_Res = new wxTextCtrl(parentPanel, wxID_ANY, "");
	Cmd_counters = new wxStaticText(parentPanel, wxID_ANY, "");
	buildDefault();
}

cmdgui::cmdgui(wxPanel* parentPanel, int currId) {
	if (!_guiExists) {

		Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
		Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY,"");
		Cmd_Sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_Res = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_counters = new wxStaticText(parentPanel,wxID_ANY,"");
	}
	_thisId = currId;
	update();
}

cmdgui::cmdgui(wxPanel* parentPanel, int currId, bool isActive, bool sequential, wxString cmdName, wxString positiveValue, wxString counters_s) {
	if (!_guiExists) {

		Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
		Cmd_active_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_Sequential_CB = new wxCheckBox(parentPanel, wxID_ANY, "");
		Cmd_txt = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_Res = new wxTextCtrl(parentPanel, wxID_ANY, "");
		Cmd_counters = new wxStaticText(parentPanel, wxID_ANY, "");
	}
	_thisId = currId;
	_isActive = isActive;
	_isSequential = sequential;
	_cmdName = cmdName;
	_positiveVal = positiveValue;
	_counters = counters_s;
	update();

}

wxBoxSizer* cmdgui::getPointer() {
	return *&Cmd_sz;
}

bool cmdgui::isActive() {
	return _isActive;
}

bool cmdgui::isSequential() {
	return _isSequential;
}

wxString cmdgui::getCmd() {
	return _cmdName;
}

wxString cmdgui::getPostiveVal() {
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
	Cmd_Sequential_CB->SetValue(false);
	Cmd_Sequential_CB->SetLabel("Sequential?");
	Cmd_txt->SetValue(_cmdName);
	Cmd_Res->SetValue(_positiveVal);
	Cmd_counters->SetLabel(_counters);
	if (!setCurrIds(_thisId)) {
		_guiExists = false;
		return false;
	}
	if (!_guiExists) {
		Cmd_sz->Add(Cmd_active_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_Sequential_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_txt, 7, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_Res, 4, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_counters, 2, wxEXPAND | wxALL, 1);
	}
	_guiExists = true;
	return true;
}


bool cmdgui::update() {

	Cmd_active_CB->SetValue(_isActive);
	Cmd_active_CB->SetLabel("Active?");
	Cmd_Sequential_CB->SetValue(_isSequential);
	Cmd_Sequential_CB->SetLabel("Sequential?");
	Cmd_txt->SetValue(_cmdName);
	Cmd_Res->SetValue(_positiveVal);
	Cmd_counters->SetLabel(_counters);
	if (!setCurrIds(_thisId)) {
		_guiExists = false;
		return false;
	}
	if (!_guiExists) {
		Cmd_sz->Add(Cmd_active_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_Sequential_CB, 1, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_txt, 7, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_Res, 4, wxEXPAND | wxALL, 1);
		Cmd_sz->Add(Cmd_counters, 2, wxEXPAND | wxALL, 1);
	}
	_guiExists = true;
	return true;
}

bool cmdgui::setSequential(bool sequentialStatus) {
	_isSequential = sequentialStatus;
	if (Cmd_Sequential_CB != nullptr) {
		Cmd_Sequential_CB->SetValue(_isSequential);
		return true;
	}
	return false;
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
	if (Cmd_Res != nullptr) {
		Cmd_Res->SetValue(positiveValue);
		return true;
	}
	return false;

}

bool cmdgui::setCurrIds(int currId) {
	_thisId = currId;
	if (Cmd_active_CB != nullptr &&
		Cmd_Sequential_CB != nullptr &&
		Cmd_txt != nullptr &&
		Cmd_Res != nullptr &&
		Cmd_counters != nullptr) {
		Cmd_active_CB->SetId(_thisId );
		Cmd_Sequential_CB->SetId(_thisId + 1);
		Cmd_txt->SetId(_thisId + 2);
		Cmd_Res->SetId(_thisId + 3);
		Cmd_counters->SetId(_thisId + 4);
		return true;
	}
	return false;
}

int cmdgui::getCurrId() {
	return _thisId;
}
