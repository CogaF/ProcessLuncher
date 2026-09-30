/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * This file is part of Process Launcher.
 */

#pragma once

/*!
 * \file LicensePolicy.h
 * \brief What a license unlocks in THIS application - the one place to adapt the license system to
 * a new product (see LICENSING.md).
 *
 * The license generator (LicGen, generic for every product) READS this file as text: keep the
 * "inline constexpr" form of kProduct, the kFeature... constants, kAllFeatures and kEditions.
 *
 * Without a license (and after the trial) the application still starts: the window, the About box
 * and the License window stay available. Running commands, the "single" mode and the batch file
 * editor need a license.
 */
namespace LicensePolicy {
	/*!
	 * \brief Product code written in every license and license request, and mixed into the machine UID.
	 * A license for another product (even one signed with the same key) is refused. Changing it
	 * invalidates every license already issued.
	 */
	inline constexpr const char* kProduct = "ProcessLuncher";

	/*! \brief Days of full use from the first start on a PC without a license; 0 = no trial. */
	inline constexpr int kTrialDays = 14;
	/*! \brief A license or trial ending within this many days is announced in the status bar and log. */
	inline constexpr int kExpiryWarningDays = 14;
	/*!
	 * \brief How far (days) the PC clock may go back before time-limited licenses and the trial are
	 * suspended as "clock set back" (a perpetual license is never affected).
	 */
	inline constexpr int kClockToleranceDays = 2;

	// --- Features: the keys written in FEATURES= of a license ---
	/*! \brief Running commands (the "Run" buttons and "Run command(s)"). */
	inline constexpr const char* kFeatureRun = "run";
	/*! \brief "Single" commands: started alone while the following ones wait for them. */
	inline constexpr const char* kFeatureSequential = "sequential";
	/*! \brief The batch file editor (File > Open batch file) with its command reference. */
	inline constexpr const char* kFeatureEditor = "editor";
	/*! \brief Every feature this version knows (a license with FEATURES=* also gets future ones). */
	inline constexpr const char* kAllFeatures[] = { kFeatureRun, kFeatureSequential, kFeatureEditor };

	/*! \brief A named set of features the generator can issue (--edition). */
	struct Edition {
		const char* key;      /*!< written in EDITION= */
		const char* features; /*!< comma-separated feature keys, or "*" for all */
	};
	/*! \brief The editions offered; the first is the generator's default. */
	inline constexpr Edition kEditions[] = {
		{ "professional", "*" },
		{ "standard", "run" },
	};
}
