/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * This file is part of Process Launcher.
 */

#pragma once

#include <string>
#include <wx/dialog.h>

#include "License.h"

class wxPanel;

/*!
 * \file LicenseDialog.h
 * \brief Help > License: the license in use (or the trial), what it unlocks, this PC's UID, and
 * the ways to get and install a license - request file, Load / Paste, drag and drop.
 */
/*!
 * \brief The License window (see the file comment).
 */
class LicenseDialog : public wxDialog {
public:
	/*! \brief Builds the window from Licensing's current state. */
	explicit LicenseDialog(wxWindow* parent);

	/*! \brief true if a license was installed or removed while the window was open. */
	bool changed() const { return changed_; }

	/*! \brief One translated sentence saying why a license is not usable (empty for Valid). */
	static wxString problemText(const License::Evaluation& evaluation);
	/*! \brief The translated display name of a feature key (LicensePolicy::kFeature...). */
	static wxString featureName(const std::string& feature);
	/*! \brief Why a feature is not available now: the license problem, or "not included". */
	static wxString featureBlockedText();
	/*! \brief Status bar text for the current license state. */
	static wxString statusBarText();

	/*! \brief Checks and installs a license text; reports the outcome in message boxes. */
	bool installText(const std::string& text);
	/*! \brief installText() with the content of a file. */
	bool installFile(const wxString& path);

private:
	void rebuild();
	void onSaveRequest();
	void onLoad();
	void onPaste();
	void onRemove();

	wxPanel* content_ = nullptr;
	bool changed_ = false;
};
