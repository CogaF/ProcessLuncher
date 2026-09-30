/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * This file is part of Process Launcher.
 */

/*!
 * \file LicenseDialog.cpp
 * \brief Implementation of LicenseDialog.h.
 */

#include "LicenseDialog.h"
#include "I18n.h"
#include "LicenseManager.h"
#include "LicensePolicy.h"
#include "StatusLed.h"
#include "TextUtils.h"
#include "UVT.h"
#include "Version.h"

#include <functional>

#include <wx/button.h>
#include <wx/clipbrd.h>
#include <wx/dnd.h>
#include <wx/filedlg.h>
#include <wx/frame.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

namespace {
	wxString utf8(const std::string& s) { return wxString::FromUTF8(s); }
	wxString pathText(const std::filesystem::path& p) { return wxString(p.wstring()); }
	wxString dateText(int64_t day) { return utf8(License::formatDate(day)); }

	/*! \brief Installs a .lic dropped on the window. */
	class LicenseDropTarget : public wxFileDropTarget {
	public:
		explicit LicenseDropTarget(LicenseDialog* dialog) : dialog_(dialog) {}
		bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& files) override {
			if (files.empty()) return false;
			const wxString path = files[0];
			// After the drop has finished, so the message boxes do not block the drag source.
			dialog_->CallAfter([d = dialog_, path] { d->installFile(path); });
			return true;
		}
	private:
		LicenseDialog* dialog_;
	};
}

LicenseDialog::LicenseDialog(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, tr(UVT::LICENSE_TITLE), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
	SetDropTarget(new LicenseDropTarget(this));
	auto* top = new wxBoxSizer(wxVERTICAL);
	content_ = new wxPanel(this);
	top->Add(content_, 1, wxEXPAND);
	SetSizer(top);
	SetEscapeId(wxID_CLOSE);
	Licensing::refresh();
	rebuild();
	CentreOnParent();
}

// ------------------------------------------------------------------------------------------------
// Texts shared with MainWindow
// ------------------------------------------------------------------------------------------------

wxString LicenseDialog::problemText(const License::Evaluation& e) {
	using S = License::Status;
	switch (e.status) {
	case S::Valid:             return {};
	case S::Missing:           return tr(UVT::LICENSE_STATUS_MISSING);
	case S::Unreadable:        return tr(UVT::LICENSE_STATUS_UNREADABLE);
	case S::Malformed:         return wxString::Format(tr(UVT::LICENSE_STATUS_MALFORMED_FMT), utf8(e.detail));
	case S::WrongProduct:      return tr(UVT::LICENSE_STATUS_WRONG_PRODUCT);
	case S::NoKeys:            return tr(UVT::LICENSE_STATUS_NO_KEYS);
	case S::UnknownKey:        return tr(UVT::LICENSE_STATUS_UNKNOWN_KEY);
	case S::BadSignature:      return tr(UVT::LICENSE_STATUS_BAD_SIGNATURE);
	case S::Revoked:           return tr(UVT::LICENSE_STATUS_REVOKED);
	case S::NoMachineId:       return tr(UVT::LICENSE_STATUS_NO_MACHINE_ID);
	case S::OtherMachine:      return tr(UVT::LICENSE_STATUS_OTHER_MACHINE);
	case S::ClockRolledBack:   return tr(UVT::LICENSE_STATUS_CLOCK);
	case S::Expired:           return wxString::Format(tr(UVT::LICENSE_STATUS_EXPIRED_FMT), e.info.expires ? dateText(*e.info.expires) : wxString());
	case S::UpdatesNotCovered: return wxString::Format(tr(UVT::LICENSE_STATUS_UPDATES_FMT),
		e.info.updatesUntil ? dateText(*e.info.updatesUntil) : wxString(), wxString::FromUTF8(kAppVersionHistory[0].date));
	}
	return {};
}

wxString LicenseDialog::featureName(const std::string& feature) {
	if (feature == LicensePolicy::kFeatureRun) return tr(UVT::FEATURE_RUN_NAME);
	if (feature == LicensePolicy::kFeatureSequential) return tr(UVT::FEATURE_SEQUENTIAL_NAME);
	if (feature == LicensePolicy::kFeatureEditor) return tr(UVT::FEATURE_EDITOR_NAME);
	return utf8(feature);
}

wxString LicenseDialog::featureBlockedText() {
	const License::Evaluation& e = Licensing::evaluation();
	if (Licensing::mode() == Licensing::Mode::Licensed)
		return wxString::Format(tr(UVT::LICENSE_FEATURE_NOT_INCLUDED_FMT), utf8(e.info.edition.empty() ? std::string("-") : e.info.edition));
	wxString text = problemText(e);
	if (Licensing::clockRolledBack() && e.status != License::Status::ClockRolledBack) text << "\n" << tr(UVT::LICENSE_STATUS_CLOCK);
	else if (Licensing::trialStateTampered()) text << "\n" << tr(UVT::LICENSE_TRIAL_TAMPERED);
	else if (LicensePolicy::kTrialDays > 0) text << "\n" << tr(UVT::LICENSE_TRIAL_OVER);
	return text;
}

wxString LicenseDialog::statusBarText() {
	switch (Licensing::mode()) {
	case Licensing::Mode::Licensed: {
		const wxString who = utf8(Licensing::evaluation().info.licensee);
		const int left = Licensing::licenseDaysLeft();
		if (left >= 0 && left <= LicensePolicy::kExpiryWarningDays) return wxString::Format(tr(UVT::STATUSBAR_LICENSE_EXPIRES_FMT), who, left);
		return wxString::Format(tr(UVT::STATUSBAR_LICENSED_FMT), who);
	}
	case Licensing::Mode::Trial:
		return wxString::Format(tr(UVT::STATUSBAR_TRIAL_FMT), Licensing::trialDaysLeft());
	default:
		return tr(UVT::STATUSBAR_UNLICENSED);
	}
}

// ------------------------------------------------------------------------------------------------
// Content
// ------------------------------------------------------------------------------------------------

void LicenseDialog::rebuild() {
	content_->DestroyChildren();
	auto* sizer = new wxBoxSizer(wxVERTICAL);
	const int gap = FromDIP(10);
	const int wrap = FromDIP(560);
	auto text = [&](wxSizer* into, const wxString& s, bool bold = false) {
		auto* label = new wxStaticText(content_, wxID_ANY, s);
		if (bold) label->SetFont(label->GetFont().Bold().Larger());
		label->Wrap(wrap);
		into->Add(label, 0, wxLEFT | wxRIGHT | wxTOP, gap);
		return label;
	};

	const Licensing::Mode mode = Licensing::mode();
	const License::Evaluation& e = Licensing::evaluation();

	// Header: a coloured LED and the state.
	auto* header = new wxBoxSizer(wxHORIZONTAL);
	auto* led = new StatusLed(content_, wxID_ANY, StatusLed::State::Off, wxSize(18, 18));
	wxString title;
	switch (mode) {
	case Licensing::Mode::Licensed:
		led->SetState(Licensing::licenseDaysLeft() >= 0 && Licensing::licenseDaysLeft() <= LicensePolicy::kExpiryWarningDays
			? StatusLed::State::Yellow : StatusLed::State::Green);
		title = tr(UVT::LICENSE_HEADER_LICENSED);
		break;
	case Licensing::Mode::Trial:
		led->SetState(StatusLed::State::Blue);
		title = wxString::Format(tr(UVT::LICENSE_HEADER_TRIAL_FMT), Licensing::trialDaysLeft());
		break;
	default:
		led->SetState(StatusLed::State::Red);
		title = tr(UVT::LICENSE_HEADER_UNLICENSED);
		break;
	}
	header->Add(led, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(8));
	auto* titleText = new wxStaticText(content_, wxID_ANY, title);
	titleText->SetFont(titleText->GetFont().Bold().Scaled(1.4f));
	header->Add(titleText, 0, wxALIGN_CENTER_VERTICAL);
	sizer->Add(header, 0, wxLEFT | wxRIGHT | wxTOP, gap);

	if (mode != Licensing::Mode::Licensed) {
		// A license that is present but refused says why, even during the trial.
		if (e.status != License::Status::Missing || mode == Licensing::Mode::Unlicensed) text(sizer, problemText(e));
		if (mode == Licensing::Mode::Trial) text(sizer, tr(UVT::LICENSE_TRIAL_EXPLAIN));
		else {
			if (Licensing::clockRolledBack() && e.status != License::Status::ClockRolledBack) text(sizer, tr(UVT::LICENSE_STATUS_CLOCK));
			else if (Licensing::trialStateTampered()) text(sizer, tr(UVT::LICENSE_TRIAL_TAMPERED));
			else if (LicensePolicy::kTrialDays > 0) text(sizer, tr(UVT::LICENSE_TRIAL_OVER));
			text(sizer, tr(UVT::LICENSE_UNLICENSED_EXPLAIN));
		}
	}

	// Details of an authentic license (also of an expired or other-PC one: it helps to see which).
	auto* grid = new wxFlexGridSizer(2, FromDIP(4), FromDIP(12));
	grid->AddGrowableCol(1);
	auto row = [&](const wxString& label, const wxString& value) {
		auto* l = new wxStaticText(content_, wxID_ANY, label);
		l->SetFont(l->GetFont().Bold());
		grid->Add(l, 0, wxALIGN_TOP);
		auto* v = new wxStaticText(content_, wxID_ANY, value);
		v->Wrap(FromDIP(420));
		grid->Add(v, 1, wxEXPAND);
	};
	if (e.authentic) {
		const License::Info& i = e.info;
		row(tr(UVT::LICENSE_LICENSED_TO_LABEL), utf8(i.licensee));
		if (!i.email.empty()) row(tr(UVT::LICENSE_EMAIL_LABEL), utf8(i.email));
		if (!i.edition.empty()) row(tr(UVT::LICENSE_EDITION_LABEL), utf8(i.edition));
		row(tr(UVT::LICENSE_ID_LABEL), utf8(i.id));
		row(tr(UVT::LICENSE_MACHINES_LABEL), i.anyMachine ? tr(UVT::LICENSE_ANY_PC)
			: i.machines.size() == 1 ? tr(UVT::LICENSE_THIS_PC)
			: wxString::Format(tr(UVT::LICENSE_PCS_FMT), static_cast<int>(i.machines.size())));
		row(tr(UVT::LICENSE_ISSUED_LABEL), dateText(i.issued));
		const int left = Licensing::licenseDaysLeft();
		row(tr(UVT::LICENSE_EXPIRES_LABEL), !i.expires ? tr(UVT::LICENSE_NEVER_EXPIRES)
			: left >= 0 ? wxString::Format(tr(UVT::LICENSE_DATE_DAYS_LEFT_FMT), dateText(*i.expires), left) : dateText(*i.expires));
		row(tr(UVT::LICENSE_UPDATES_LABEL), i.updatesUntil ? dateText(*i.updatesUntil) : tr(UVT::LICENSE_ALL_UPDATES));
		if (!i.notes.empty()) row(tr(UVT::LICENSE_NOTES_LABEL), utf8(Utils::Str::join(i.notes, "\n")));
		row(tr(UVT::LICENSE_FILE_LABEL), pathText(Licensing::licenseFile()));
	}
	// What is available now.
	wxString features;
	for (const char* f : LicensePolicy::kAllFeatures) {
		if (!features.empty()) features << "\n";
		features << (Licensing::allows(f) ? wxString::FromUTF8("\xE2\x9C\x94 ") : wxString::FromUTF8("\xE2\x9C\x98 "))
			<< featureName(f) << " - " << (Licensing::allows(f) ? tr(UVT::LICENSE_FEATURE_ON) : tr(UVT::LICENSE_FEATURE_OFF));
	}
	row(tr(UVT::LICENSE_FEATURES_LABEL), features);
	sizer->Add(grid, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, gap);

	// This PC's UID.
	const std::string& uid = Licensing::machineUid();
	if (!uid.empty()) {
		text(sizer, tr(UVT::LICENSE_UID_LABEL));
		auto* uidRow = new wxBoxSizer(wxHORIZONTAL);
		const wxString shown = utf8(License::displayUid(uid));
		auto* uidText = new wxTextCtrl(content_, wxID_ANY, shown, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		uidText->SetFont(wxFont(wxFontInfo(uidText->GetFont().GetPointSize() + 1).Family(wxFONTFAMILY_TELETYPE)));
		uidText->SetMinSize(wxSize(uidText->GetTextExtent(shown).x + FromDIP(20), -1));
		uidRow->Add(uidText, 1, wxEXPAND | wxRIGHT, FromDIP(6));
		auto* copy = new wxButton(content_, wxID_ANY, tr(UVT::LICENSE_COPY_BTN));
		copy->Bind(wxEVT_BUTTON, [this, shown](wxCommandEvent&) {
			if (wxTheClipboard->Open()) {
				wxTheClipboard->SetData(new wxTextDataObject(shown));
				wxTheClipboard->Close();
				if (auto* frame = wxDynamicCast(GetParent(), wxFrame)) frame->SetStatusText(tr(UVT::LICENSE_UID_COPIED), 0);
			}
		});
		uidRow->Add(copy, 0);
		sizer->Add(uidRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, gap);
	}
	if (mode != Licensing::Mode::Licensed) text(sizer, wxString::Format(tr(UVT::LICENSE_HOW_TO_FMT), pathText(Licensing::requestFile().filename())));

	// Buttons.
	auto* buttons = new wxBoxSizer(wxHORIZONTAL);
	auto button = [&](const wxString& label, std::function<void()> action, bool enabled = true) {
		auto* b = new wxButton(content_, wxID_ANY, label);
		b->Bind(wxEVT_BUTTON, [action](wxCommandEvent&) { action(); });
		b->Enable(enabled);
		buttons->Add(b, 0, wxRIGHT, FromDIP(6));
	};
	button(tr(UVT::LICENSE_SAVE_REQUEST_BTN), [this] { onSaveRequest(); }, !uid.empty());
	button(tr(UVT::LICENSE_LOAD_BTN), [this] { onLoad(); });
	button(tr(UVT::LICENSE_PASTE_BTN), [this] { onPaste(); });
	std::error_code ec;
	button(tr(UVT::LICENSE_REMOVE_BTN), [this] { onRemove(); }, std::filesystem::exists(Licensing::installedLicenseFile(), ec));
	button(tr(UVT::LICENSE_OPEN_FOLDER_BTN), [] {
		const std::filesystem::path file = Licensing::licenseFile();
		std::error_code e2;
		const std::filesystem::path select = std::filesystem::exists(file, e2) ? file : Licensing::requestFile();
#ifdef _WIN32
		if (std::filesystem::exists(select, e2)) { wxExecute("explorer /select,\"" + pathText(select) + "\""); return; }
#endif
		wxLaunchDefaultApplication(pathText(select.parent_path()));
	});
	buttons->AddStretchSpacer();
	auto* close = new wxButton(content_, wxID_CLOSE, tr(UVT::CLOSE_BTN));
	close->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxID_CLOSE); });
	close->SetDefault();
	buttons->Add(close, 0);
	sizer->Add(buttons, 0, wxEXPAND | wxALL, gap);

	content_->SetSizer(sizer, true);
	GetSizer()->SetSizeHints(this);
	Layout();
}

// ------------------------------------------------------------------------------------------------
// Actions
// ------------------------------------------------------------------------------------------------

bool LicenseDialog::installText(const std::string& text) {
	License::Evaluation result;
	std::string error;
	if (!Licensing::install(text, result, error)) {
		if (!error.empty())
			wxMessageBox(wxString::Format(tr(UVT::LICENSE_INSTALL_FAILED_FMT), utf8(error)), tr(UVT::LICENSE_TITLE), wxOK | wxICON_ERROR, this);
		else
			wxMessageBox(wxString::Format(tr(UVT::LICENSE_NOT_VALID_FMT), problemText(result)), tr(UVT::LICENSE_TITLE), wxOK | wxICON_ERROR, this);
		return false;
	}
	changed_ = true;
	CallAfter([this] { rebuild(); }); // not now: the button that got here is one of the controls rebuild() replaces
	wxMessageBox(wxString::Format(tr(UVT::LICENSE_INSTALLED_FMT), utf8(result.info.licensee)), tr(UVT::LICENSE_TITLE), wxOK | wxICON_INFORMATION, this);
	return true;
}

bool LicenseDialog::installFile(const wxString& path) {
	const auto text = Utils::Files::readText(std::filesystem::path(path.ToStdWstring()));
	if (!text) {
		wxMessageBox(wxString::Format(tr(UVT::LICENSE_NOT_VALID_FMT), tr(UVT::LICENSE_STATUS_UNREADABLE)), tr(UVT::LICENSE_TITLE), wxOK | wxICON_ERROR, this);
		return false;
	}
	return installText(*text);
}

void LicenseDialog::onSaveRequest() {
	// Written to the data folder anyway, then offered wherever the user wants it (e.g. to attach to an e-mail).
	std::string error;
	Licensing::writeRequest(error);
	const std::filesystem::path defaultFile = Licensing::requestFile();
	wxFileDialog dlg(this, tr(UVT::LICENSE_SAVE_REQUEST_TITLE), pathText(defaultFile.parent_path()), pathText(defaultFile.filename()),
		tr(UVT::LICENSE_REQUEST_FILTER), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK) return;
	if (!Utils::Files::writeTextAtomic(std::filesystem::path(dlg.GetPath().ToStdWstring()), Licensing::requestText())) {
		wxMessageBox(wxString::Format(tr(UVT::LICENSE_SAVE_FAILED_FMT), dlg.GetPath()), tr(UVT::LICENSE_TITLE), wxOK | wxICON_ERROR, this);
		return;
	}
	wxMessageBox(wxString::Format(tr(UVT::LICENSE_REQUEST_SAVED_FMT), dlg.GetPath()), tr(UVT::LICENSE_TITLE), wxOK | wxICON_INFORMATION, this);
}

void LicenseDialog::onLoad() {
	wxFileDialog dlg(this, tr(UVT::LICENSE_LOAD_TITLE), wxString(), wxString(), tr(UVT::LICENSE_FILES_FILTER), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK) installFile(dlg.GetPath());
}

void LicenseDialog::onPaste() {
	wxString text;
	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported(wxDF_UNICODETEXT) || wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject data;
			if (wxTheClipboard->GetData(data)) text = data.GetText();
		}
		wxTheClipboard->Close();
	}
	if (text.Strip(wxString::both).empty()) {
		wxMessageBox(tr(UVT::LICENSE_CLIPBOARD_EMPTY), tr(UVT::LICENSE_TITLE), wxOK | wxICON_INFORMATION, this);
		return;
	}
	installText(text.utf8_string());
}

void LicenseDialog::onRemove() {
	if (wxMessageBox(tr(UVT::LICENSE_REMOVE_CONFIRM), tr(UVT::LICENSE_TITLE), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) != wxYES) return;
	std::string error;
	if (!Licensing::uninstall(error))
		wxMessageBox(wxString::Format(tr(UVT::LICENSE_INSTALL_FAILED_FMT), utf8(error)), tr(UVT::LICENSE_TITLE), wxOK | wxICON_ERROR, this);
	changed_ = true;
	CallAfter([this] { rebuild(); });
}
