/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "License.h"

/*!
 * \file LicenseManager.h
 * \brief The license state of the running application: which license is in use, the trial, and
 * whether a feature may be used. Call Licensing::initialize() once at startup (App::OnInit).
 *
 * - The license is "<data folder>/license.lic", or "license.lic" next to the exe (for an installer
 *   that ships one); see License.h for the format and LicensePolicy.h for the features.
 * - Without a valid license, a trial of LicensePolicy::kTrialDays unlocks every feature. Its start
 *   date is kept twice - in the data folder and (Windows) in HKCU\Software\<app name>\License - each
 *   copy protected by a check value, so deleting or editing one copy does not restart it.
 * - The newest date ever seen is kept with it: a clock set back by more than
 *   LicensePolicy::kClockToleranceDays suspends the trial and time-limited licenses (not
 *   perpetual ones) until the clock is right again.
 * - Without a valid license, "<data folder>/license-request.txt" is written with this PC's UID,
 *   ready to send to the license issuer.
 *
 * This is protection against casual misuse and honest mistakes, not against a determined
 * attacker with a debugger - no purely local check is. What it guarantees is that nobody without
 * the private signing key can make a license (see LICENSING.md).
 */
namespace Licensing {
	/*! \brief What the application currently runs under. */
	enum class Mode {
		Licensed,  /*!< a valid license (features as it lists) */
		Trial,     /*!< no valid license, trial days left: every feature */
		Unlicensed /*!< neither: only what needs no license */
	};

	/*! \brief Reads the machine UID, the trial state and the license; logs the outcome. */
	void initialize();
	/*! \brief Reads the license file again (e.g. after it was copied into the folder). */
	void refresh();
	/*!
	 * \brief Re-evaluates the dates when the day has changed since the last check (call it from a
	 * timer). \return true if the mode or the allowed features may have changed.
	 */
	bool refreshIfDayChanged();

	/*! \brief The current mode. */
	Mode mode();
	/*! \brief The last evaluation of the license file (status Missing if there is none). */
	const License::Evaluation& evaluation();
	/*! \brief true if feature (LicensePolicy::kFeature...) may be used now. */
	bool allows(std::string_view feature);

	/*! \brief Days of trial left (0 when over, disabled, or suspended). */
	int trialDaysLeft();
	/*! \brief true if the trial state was found edited (the trial is then over). */
	bool trialStateTampered();
	/*! \brief true if the PC clock was found set back (see the file comment). */
	bool clockRolledBack();
	/*! \brief Days until a time-limited license expires (0 = last day); -1 if it does not expire or is not valid. */
	int licenseDaysLeft();

	/*! \brief This PC's UID (canonical, no dashes), empty if it could not be read. */
	const std::string& machineUid();
	/*! \brief The license file checked last (it may not exist). */
	std::filesystem::path licenseFile();
	/*! \brief Where install() puts a license: "<data folder>/license.lic". */
	std::filesystem::path installedLicenseFile();
	/*! \brief "<data folder>/license-request.txt". */
	std::filesystem::path requestFile();

	/*! \brief Evaluates a license text against this PC and date, without installing it. */
	License::Evaluation check(std::string_view licenseText);
	/*!
	 * \brief Installs licenseText as the data folder's license.lic if it is valid for this PC (the
	 * previous one is kept as license.lic.bak). \param result the evaluation of licenseText.
	 * \return false if it is not valid (see result.status) or could not be written (error set).
	 */
	bool install(std::string_view licenseText, License::Evaluation& result, std::string& error);
	/*! \brief Removes the data folder's license.lic (kept as license.lic.bak). */
	bool uninstall(std::string& error);
	/*! \brief The text of a license request for this PC. */
	std::string requestText();
	/*! \brief Writes requestFile(). */
	bool writeRequest(std::string& error);
}
