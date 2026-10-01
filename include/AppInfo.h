/*!
 * \file AppInfo.h
 * \brief The application's identity - the ONE place its name lives in the code.
 *
 * The exe name comes from AppExeBaseName in the .vcxproj - keep both the same.
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 */
#pragma once

/*! \brief Names and texts that identify the application. */
namespace AppInfo {
	/*! \brief Shown in the About box and the status bar; also names the registry key of the trial state. */
	inline constexpr const char* kName = "Process Launcher";
	/*! \brief Short name used in the window title. */
	inline constexpr const char* kShortName = "PCR";
	/*! \brief One line description shown in the About box. */
	inline constexpr const char* kDescription = "Runs commands and batch files in parallel or in sequence, searches their result for a text and records PASS or FAIL.";
	/*! \brief Folder next to the exe holding every file the application reads or writes (see DataDir.h). */
	inline constexpr const char* kDataFolderName = "Process Launcher data";
	/*! \brief Folder next to the exe with the example batch files. */
	inline constexpr const char* kExamplesFolderName = "bat_examples";
	/*! \brief Default result file, next to the exe. */
	inline constexpr const char* kDefaultResultFileName = "result.txt";
	/*! \brief Copyright line shown in the About box. */
	inline constexpr const char* kCopyright = "Copyright (C) 2025-2026 Fation Coga";
	/*! \brief Organisation shown in the About box. */
	inline constexpr const char* kOrganisation = "EGO Group S.r.L";
	/*! \brief Contact e-mail shown in the About box. */
	inline constexpr const char* kContact = "fation.coga@egogroup.eu";
	/*! \brief Project page shown in the About box. */
	inline constexpr const char* kWebsite = "https://github.com/CogaF/ProcessLuncher";
	/*! \brief Licence line shown in the About box (the user's own license: Info > License). */
	inline constexpr const char* kLicense = "Use requires a valid license; a 14-day trial starts with the first run.";
}
