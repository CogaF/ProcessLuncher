/*!
 * \file cmdgui.cpp
 * \brief Implementation of cmdgui.h.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */

#include "cmdgui.h"

#include "Id.h"

cmdgui::cmdgui(wxPanel* parentPanel)
{
    createControls(parentPanel, 0);
}

cmdgui::cmdgui(wxPanel* parentPanel, int guiIndex)
{
    createControls(parentPanel, guiIndex);
}

cmdgui::cmdgui(wxPanel* parentPanel, int guiIndex, bool isActive, bool sequential, const wxString& cmdName,
               const wxString& positiveValue, const wxString& counters_s)
{
    _isActive = isActive;
    _isSequential = sequential;
    _cmdName = cmdName;
    _positiveVal = positiveValue;
    _counters = counters_s;
    createControls(parentPanel, guiIndex);
}

void cmdgui::createControls(wxPanel* parentPanel, int guiIndex)
{
    // The ids are given at construction time (they used to be patched afterwards with SetId()).
    _thisId = windowIDs::cmdRowBaseId(guiIndex);

    Cmd_sz = new wxBoxSizer(wxHORIZONTAL);
    Cmd_active_CB = new wxCheckBox(parentPanel, _thisId, "");
    Cmd_run_bt = new wxButton(parentPanel, _thisId + RUNBUTTON_ID_INDEX, "Run");
    Cmd_txt = new wxTextCtrl(parentPanel, _thisId + TEXT_ID_INDEX, "");
    Cmd_res = new wxTextCtrl(parentPanel, _thisId + RES_ID_INDEX, "");
    Cmd_counters = new wxStaticText(parentPanel, _thisId + COUNTERS_ID_INDEX, "");
    Cmd_sequential_CB = new wxCheckBox(parentPanel, _thisId + SEQUENTIAL_ID_INDEX, "");
    Cmd_view_CB = new wxCheckBox(parentPanel, _thisId + VIEW_ID_INDEX, "");
    Cmd_running_CB = new wxCheckBox(parentPanel, _thisId + RUNNING_ID_INDEX, "");

    // "View" is not implemented yet (see MainWindow::RunCommand) and "Busy" is only an indicator.
    Cmd_view_CB->Disable();
    Cmd_running_CB->Disable();

    Cmd_sz->Add(Cmd_active_CB, 1, wxEXPAND | wxALL, 1);
    Cmd_sz->Add(Cmd_run_bt, 1, wxEXPAND | wxALL, 1);
    Cmd_sz->Add(Cmd_txt, 7, wxEXPAND | wxALL, 1);
    Cmd_sz->Add(Cmd_res, 4, wxEXPAND | wxALL, 1);
    Cmd_sz->Add(Cmd_counters, 2, wxEXPAND | wxALL, 1);
    Cmd_sz->Add(Cmd_sequential_CB, 1, wxEXPAND | wxALL, 1);
    Cmd_sz->Add(Cmd_view_CB, 1, wxEXPAND | wxALL, 1);
    Cmd_sz->Add(Cmd_running_CB, 1, wxEXPAND | wxALL, 1);

    refreshWidgets();
}

void cmdgui::refreshWidgets()
{
    Cmd_txt->SetValue(_cmdName);
    Cmd_res->SetValue(_positiveVal);
    Cmd_counters->SetLabel(_counters);
    Cmd_active_CB->SetValue(_isActive);
    Cmd_sequential_CB->SetValue(_isSequential);
    Cmd_view_CB->SetValue(_isViewShow);
    Cmd_running_CB->SetValue(_isRunning);
    refreshLabels();
}

void cmdgui::refreshLabels()
{
    Cmd_active_CB->SetLabel(_isActive ? "ON" : "OFF");
    Cmd_sequential_CB->SetLabel(_isSequential ? "Single" : "Parallel");
    Cmd_view_CB->SetLabel(_isViewShow ? "Show" : "Hide");
    Cmd_running_CB->SetLabel(_isRunning ? "Busy" : "Ready");
    // The Run button is usable only for an active command that is not already running.
    Cmd_run_bt->Enable(_isActive && !_isRunning);
}

void cmdgui::setResult(bool result)
{
    _isPass = result;
}

bool cmdgui::getResult() const
{
    return _isPass;
}

void cmdgui::disable()
{
    setActive(false);
    Cmd_sequential_CB->Disable();
    Cmd_txt->Disable();
    Cmd_res->Disable();
    Cmd_counters->Disable();
    Cmd_view_CB->Disable();
}

void cmdgui::enable()
{
    setActive(true);
    Cmd_sequential_CB->Enable();
    Cmd_txt->Enable();
    Cmd_res->Enable();
    Cmd_counters->Enable();
    // Cmd_view_CB stays disabled until the "view" feature exists.
}

void cmdgui::disableEditables()
{
    Cmd_sequential_CB->Disable();
    Cmd_txt->Disable();
    Cmd_res->Disable();
    Cmd_view_CB->Disable();
}

void cmdgui::enableEditables()
{
    if (_isActive) {
        Cmd_sequential_CB->Enable();
        Cmd_txt->Enable();
        Cmd_res->Enable();
        // Cmd_view_CB stays disabled until the "view" feature exists.
    }
}

wxBoxSizer* cmdgui::getPointer()
{
    return Cmd_sz;
}

bool cmdgui::isActive()
{
    _isActive = Cmd_active_CB->GetValue();
    return _isActive;
}

bool cmdgui::isSequential()
{
    _isSequential = Cmd_sequential_CB->GetValue();
    return _isSequential;
}

wxString cmdgui::getCmd()
{
    _cmdName = Cmd_txt->GetValue();
    return _cmdName;
}

wxString cmdgui::getPositiveVal()
{
    _positiveVal = Cmd_res->GetValue();
    return _positiveVal;
}

bool cmdgui::setCounters(const wxString& countersString)
{
    _counters = countersString;
    if (Cmd_counters == nullptr) return false;
    Cmd_counters->SetLabel(_counters);
    return true;
}

bool cmdgui::setSequential(bool sequentialStatus)
{
    _isSequential = sequentialStatus;
    if (Cmd_sequential_CB == nullptr) return false;
    Cmd_sequential_CB->SetValue(_isSequential);
    refreshLabels();
    return true;
}

bool cmdgui::setRunning(bool runningStatus)
{
    _isRunning = runningStatus;
    if (Cmd_running_CB == nullptr) return false;
    Cmd_running_CB->SetValue(_isRunning);
    refreshLabels();
    return true;
}

bool cmdgui::getRunning() const
{
    return _isRunning;
}

bool cmdgui::setActive(bool activeStatus)
{
    _isActive = activeStatus;
    if (Cmd_active_CB == nullptr) return false;
    Cmd_active_CB->SetValue(_isActive);
    refreshLabels();
    return true;
}

bool cmdgui::getView() const
{
    return _isViewShow;
}

bool cmdgui::setView(bool viewStatus)
{
    _isViewShow = viewStatus;
    if (Cmd_view_CB == nullptr) return false;
    Cmd_view_CB->SetValue(_isViewShow);
    refreshLabels();
    return true;
}

bool cmdgui::setCmd(const wxString& cmdName)
{
    _cmdName = cmdName;
    if (Cmd_txt == nullptr) return false;
    Cmd_txt->SetValue(_cmdName);
    return true;
}

bool cmdgui::setPostVal(const wxString& positiveValue)
{
    _positiveVal = positiveValue;
    if (Cmd_res == nullptr) return false;
    Cmd_res->SetValue(_positiveVal);
    return true;
}

int cmdgui::getCurrId() const
{
    return _thisId;
}
