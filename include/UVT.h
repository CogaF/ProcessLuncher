/*!
 * \file UVT.h
 * \brief User-Visible Texts of the license system (English) - shown through tr().
 *
 * Only the texts of the shared license code live here; the rest of the application writes its
 * texts in place. Format strings (%s, %d) must keep their placeholders.
 */
#pragma once

#include <wx/string.h>

/*! \brief Texts of the License window, its messages and the status bar. */
namespace UVT {
	/*! \brief Button: close. */
	inline const wxString CLOSE_BTN = "Close";
	/*! \brief Window title: license. */
	inline const wxString LICENSE_TITLE = "License";
	/*! \brief License window header: licensed. */
	inline const wxString LICENSE_HEADER_LICENSED = "Licensed";
	/*! \brief License window header: trial (%d: days left). */
	inline const wxString LICENSE_HEADER_TRIAL_FMT = "Trial version - %d day(s) left";
	/*! \brief License window header: no license. */
	inline const wxString LICENSE_HEADER_UNLICENSED = "Not licensed";
	/*! \brief Text: what works without a license. */
	inline const wxString LICENSE_UNLICENSED_EXPLAIN = "The window, the About box and the License window stay available; running commands, the single mode and the batch file editor need a license.";
	/*! \brief Text: the trial. */
	inline const wxString LICENSE_TRIAL_EXPLAIN = "Every feature is available during the trial.";
	/*! \brief Text: trial over. */
	inline const wxString LICENSE_TRIAL_OVER = "The trial period is over.";
	/*! \brief Text: trial state edited. */
	inline const wxString LICENSE_TRIAL_TAMPERED = "The trial information was changed - the trial is over.";
	/*! \brief License problem: none installed. */
	inline const wxString LICENSE_STATUS_MISSING = "No license is installed.";
	/*! \brief License problem: unreadable. */
	inline const wxString LICENSE_STATUS_UNREADABLE = "The license file could not be read.";
	/*! \brief License problem: damaged (%s: field). */
	inline const wxString LICENSE_STATUS_MALFORMED_FMT = "The license file is damaged or is not a license (%s).";
	/*! \brief License problem: other product. */
	inline const wxString LICENSE_STATUS_WRONG_PRODUCT = "The license is for a different product.";
	/*! \brief License problem: build without key. */
	inline const wxString LICENSE_STATUS_NO_KEYS = "This build contains no license verification key, so no license can be accepted (developer: see LICENSING.md).";
	/*! \brief License problem: unknown key. */
	inline const wxString LICENSE_STATUS_UNKNOWN_KEY = "The license was signed with a key this version does not know - ask the issuer for a license for this version.";
	/*! \brief License problem: bad signature. */
	inline const wxString LICENSE_STATUS_BAD_SIGNATURE = "The license was not issued by the license owner, or it was changed after it was issued.";
	/*! \brief License problem: revoked. */
	inline const wxString LICENSE_STATUS_REVOKED = "This license has been revoked.";
	/*! \brief License problem: no machine id. */
	inline const wxString LICENSE_STATUS_NO_MACHINE_ID = "This PC's unique ID could not be read.";
	/*! \brief License problem: other PC. */
	inline const wxString LICENSE_STATUS_OTHER_MACHINE = "The license was issued for a different PC.";
	/*! \brief License problem: clock set back. */
	inline const wxString LICENSE_STATUS_CLOCK = "The PC's date is earlier than a date already seen: the trial and time-limited licenses are suspended until the date is correct.";
	/*! \brief License problem: expired (%s: date). */
	inline const wxString LICENSE_STATUS_EXPIRED_FMT = "The license expired on %s.";
	/*! \brief License problem: version not covered (%s: covered until, %s: release date). */
	inline const wxString LICENSE_STATUS_UPDATES_FMT = "The license covers versions released until %s; this version was released on %s. Keep using an older version or renew the license.";
	/*! \brief Field: licensee. */
	inline const wxString LICENSE_LICENSED_TO_LABEL = "Licensed to:";
	/*! \brief Field: e-mail. */
	inline const wxString LICENSE_EMAIL_LABEL = "E-mail:";
	/*! \brief Field: edition. */
	inline const wxString LICENSE_EDITION_LABEL = "Edition:";
	/*! \brief Field: license id. */
	inline const wxString LICENSE_ID_LABEL = "License ID:";
	/*! \brief Field: machines. */
	inline const wxString LICENSE_MACHINES_LABEL = "Valid on:";
	/*! \brief Value: this PC only. */
	inline const wxString LICENSE_THIS_PC = "this PC";
	/*! \brief Value: several PCs (%d: count). */
	inline const wxString LICENSE_PCS_FMT = "%d PCs, this one included";
	/*! \brief Value: site license. */
	inline const wxString LICENSE_ANY_PC = "any PC (site license)";
	/*! \brief Field: issued. */
	inline const wxString LICENSE_ISSUED_LABEL = "Issued:";
	/*! \brief Field: expires. */
	inline const wxString LICENSE_EXPIRES_LABEL = "Expires:";
	/*! \brief Value: perpetual. */
	inline const wxString LICENSE_NEVER_EXPIRES = "never (perpetual)";
	/*! \brief Value: date and days left. */
	inline const wxString LICENSE_DATE_DAYS_LEFT_FMT = "%s (%d day(s) left)";
	/*! \brief Field: updates until. */
	inline const wxString LICENSE_UPDATES_LABEL = "Updates until:";
	/*! \brief Value: all versions. */
	inline const wxString LICENSE_ALL_UPDATES = "all versions";
	/*! \brief Field: notes. */
	inline const wxString LICENSE_NOTES_LABEL = "Notes:";
	/*! \brief Field: file. */
	inline const wxString LICENSE_FILE_LABEL = "File:";
	/*! \brief Field: features. */
	inline const wxString LICENSE_FEATURES_LABEL = "Features:";
	/*! \brief Feature state: available. */
	inline const wxString LICENSE_FEATURE_ON = "available";
	/*! \brief Feature state: not available. */
	inline const wxString LICENSE_FEATURE_OFF = "not included";
	/*! \brief Label: machine UID. */
	inline const wxString LICENSE_UID_LABEL = "This PC's unique ID (UID):";
	/*! \brief Button: copy UID. */
	inline const wxString LICENSE_COPY_BTN = "Copy";
	/*! \brief Status bar: UID copied. */
	inline const wxString LICENSE_UID_COPIED = "UID copied to the clipboard.";
	/*! \brief Text: how to get a license (%s: request file). */
	inline const wxString LICENSE_HOW_TO_FMT = "To get a license, send the UID above - or the file %s from the data folder (Open Folder) - to the license issuer. Install the license you receive with Load License..., with Paste License, or by dropping the file on this window.";
	/*! \brief Button: save request. */
	inline const wxString LICENSE_SAVE_REQUEST_BTN = "Save Request...";
	/*! \brief Button: load license. */
	inline const wxString LICENSE_LOAD_BTN = "Load License...";
	/*! \brief Button: paste license. */
	inline const wxString LICENSE_PASTE_BTN = "Paste License";
	/*! \brief Button: remove license. */
	inline const wxString LICENSE_REMOVE_BTN = "Remove License";
	/*! \brief Button: open folder. */
	inline const wxString LICENSE_OPEN_FOLDER_BTN = "Open Folder";
	/*! \brief File dialog title: load license. */
	inline const wxString LICENSE_LOAD_TITLE = "Select the license file";
	/*! \brief File dialog filter: licenses. */
	inline const wxString LICENSE_FILES_FILTER = "License files (*.lic)|*.lic|All files (*.*)|*.*";
	/*! \brief File dialog title: save request. */
	inline const wxString LICENSE_SAVE_REQUEST_TITLE = "Save the license request";
	/*! \brief File dialog filter: requests. */
	inline const wxString LICENSE_REQUEST_FILTER = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
	/*! \brief Message: request saved (%s: file). */
	inline const wxString LICENSE_REQUEST_SAVED_FMT = "License request saved to %s.";
	/*! \brief Message: cannot write (%s: file). */
	inline const wxString LICENSE_SAVE_FAILED_FMT = "Cannot write %s.";
	/*! \brief Message: license refused (%s: reason). */
	inline const wxString LICENSE_NOT_VALID_FMT = "This license cannot be used on this PC:\n%s";
	/*! \brief Message: license installed (%s: licensee). */
	inline const wxString LICENSE_INSTALLED_FMT = "License installed - licensed to %s.";
	/*! \brief Message: install failed (%s: reason). */
	inline const wxString LICENSE_INSTALL_FAILED_FMT = "The license could not be installed: %s";
	/*! \brief Message: nothing to paste. */
	inline const wxString LICENSE_CLIPBOARD_EMPTY = "The clipboard contains no text.";
	/*! \brief Question: remove the license. */
	inline const wxString LICENSE_REMOVE_CONFIRM = "Remove the installed license? It is kept as license.lic.bak in the data folder.";
	/*! \brief Question: feature needs a license (%s: feature, %s: reason). */
	inline const wxString LICENSE_FEATURE_NEEDED_FMT = "%s needs a license.\n\n%s\n\nOpen the License window?";
	/*! \brief Text: licensed, feature not included (%s: edition). */
	inline const wxString LICENSE_FEATURE_NOT_INCLUDED_FMT = "Your license (edition: %s) does not include it.";
	/*! \brief Status bar: licensed (%s: licensee). */
	inline const wxString STATUSBAR_LICENSED_FMT = "Licensed to %s";
	/*! \brief Status bar: license expiring (%s: licensee, %d: days). */
	inline const wxString STATUSBAR_LICENSE_EXPIRES_FMT = "Licensed to %s - expires in %d day(s)";
	/*! \brief Status bar: trial (%d: days left). */
	inline const wxString STATUSBAR_TRIAL_FMT = "Trial: %d day(s) left";
	/*! \brief Status bar: no license. */
	inline const wxString STATUSBAR_UNLICENSED = "Not licensed: running commands disabled (Info > License)";

	/*! \brief Name of the feature "run". */
	inline const wxString FEATURE_RUN_NAME = "Running commands";
	/*! \brief Name of the feature "sequential". */
	inline const wxString FEATURE_SEQUENTIAL_NAME = "Single (sequential) commands";
	/*! \brief Name of the feature "editor". */
	inline const wxString FEATURE_EDITOR_NAME = "Batch file editor";
}
