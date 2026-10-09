/*!
 * \file Version.h
 * \brief Application version (semantic versioning) and its history - the ONE place both are kept.
 *
 *  - MAJOR: old files or habits stop working. Resets MINOR and PATCH.
 *  - MINOR: new features, everything old keeps working. Resets PATCH.
 *  - PATCH: bug fixes only.
 *  - PRERELEASE: "rc.1", "rc.2", ... while testing; "" for the final release.
 *
 * The license system compares the date of the top entry of kAppVersionHistory with a license's
 * UPDATES_UNTIL, so add an entry (newest first) with the release date of every release.
 */
#pragma once

/*! \brief MAJOR: incompatible changes. */
#define APP_VERSION_MAJOR 0
/*! \brief MINOR: new features, compatible. */
#define APP_VERSION_MINOR 2
/*! \brief PATCH: bug fixes only. */
#define APP_VERSION_PATCH 0
/*! \brief "rc.N" while testing, "" for the final release. */
#define APP_VERSION_PRERELEASE "rc.2"

/*! \brief One released (or release-candidate) version; notes: one item per line, "Added:/Changed:/Fixed:/Note:". */
struct AppVersionHistoryEntry {
	const char* version; /*!< "0.2.0-rc.1" */
	const char* date;    /*!< release date, YYYY-MM-DD */
	const char* notes;   /*!< one item per line */
};

/*! \brief Newest first. */
inline constexpr AppVersionHistoryEntry kAppVersionHistory[] = {
	{ "0.2.0-rc.2", "2026-10-09",
		"Fixed: closing the window while waiting for a single command could crash.\n"
		"Fixed: closing the window terminates the unfinished commands with everything they started.\n"
		"Fixed: batch editors ask to save before the main window closes.\n"
		"Fixed: rows unlocked when the last command ends; previous-failure check covers every earlier row.\n"
		"Fixed: batch editor saves a file in the encoding it was read in (OEM code page or UTF-8).\n"
		"Added: time limit per row, Stop button, PCR_CMD_ID, exit code and duration in the result list.\n"
		"Added: expected result :Exit:<codes> (exit code check) and {id} (row number) in the expected result.\n"
		"Added: project files (.pcr): the command rows are saved on close and at every run and loaded at the next start.\n"
		"Changed: contact e-mail coga.fation@gmail.com." },
	{ "0.2.0-rc.1", "2026-09-30",
		"Added: wxWidgets 3.3 and dark mode.\n"
		"Added: result file next to the exe: path shown and editable, created on request, searched only from where it ended when the command started.\n"
		"Added: bat_examples folder with batch files that report PASS / FAIL and append a time stamped line to the result file.\n"
		"Added: File > Open batch file: editor with highlighting, font size buttons and a reference of the batch commands.\n"
		"Added: per-PC licenses issued with LicGen (product ProcessLuncher, features run, sequential and editor), 14-day trial.\n"
		"Added: structured About window.\n"
		"Fixed: duplicate event symbol, uninitialised counters, worker threads calling the GUI, result text used as a format string, broken :File: parsing." },
	{ "0.1.0", "2025-02-28",
		"Note: first version: parallel and single commands, result found in the output or in a file." },
};
