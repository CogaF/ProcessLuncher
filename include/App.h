/*!
 * \file App.h
 * \brief The wxWidgets application object of Process Launcher.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/app.h>
#include <wx/cmdline.h>

#include "MainWindow.h"

/*!
 * \brief Application object.
 *
 * OnInit() also opens the log (<data folder>/log.txt) and starts the license system.
 *
 * Command line (all optional):
 * \code
 * "Process Launcher.exe" [project.pcr] [--project <file>] [--run] [--repeat <n>] [--csv <file>]
 *                        [--screenshots <folder>]
 * \endcode
 * --run runs the rows at once without questions, exports the CSV if asked and closes; the exit code
 * is then 0 (every result PASS), 1 (at least one FAIL) or 2 (nothing could run: license, project,
 * no row ON). Use "start /wait" in a batch file to wait for it.
 *
 * Process Launcher is a small tool without a theme setting, so it always starts in dark mode:
 * OnInit() calls wxApp::SetAppearance() (wxWidgets 3.3+) before the first window exists, and
 * FilterEvent() colours every top-level window the first time it is shown and gives it the
 * application icon.
 */
class App : public wxApp
{
public:
    /*!
     * \brief Selects the dark appearance, then creates and shows the main window.
     * \return false to abort the start-up.
     */
    bool OnInit() override;

    /*! \brief Closes the log file. \return the exit code. */
    int OnExit() override;

    /*!
     * \brief Sees every event before the window it is meant for.
     *
     * Used to theme each top-level window (frame, dialog, message box) and set its icon when it is
     * shown for the first time.
     * \param event the event being dispatched.
     * \return Event_Skip (-1) so that normal processing continues.
     */
    int FilterEvent(wxEvent& event) override;

    /*! \brief Declares the command line options. */
    void OnInitCmdLine(wxCmdLineParser& parser) override;
    /*! \brief Reads the command line options into the start options. \return false to stop. */
    bool OnCmdLineParsed(wxCmdLineParser& parser) override;
    /*! \brief Runs the event loop. \return the exit code set with SetExitCode(), if any. */
    int OnRun() override;

    /*! \brief Exit code of the process (command line --run). */
    void SetExitCode(int code) { m_exitCode = code; }

private:
    StartOptions m_options;  /*!< what the command line asked for. */
    int m_exitCode = -1;     /*!< -1: the default exit code of wxWidgets. */
};

wxDECLARE_APP(App);
