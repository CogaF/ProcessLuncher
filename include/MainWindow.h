/*!
 * \file MainWindow.h
 * \brief The main window of Process Launcher.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/button.h>
#include <wx/event.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/thread.h>
#include <wx/timer.h>

#include <array>
#include <memory>
#include <mutex>
#include <vector>

#include "Id.h"
#include "cmdgui.h"

/*! \brief Number of command rows shown in the main window. */
constexpr int kNrOfCmds = 11;

/*!
 * \brief Hard upper limit of command rows.
 *
 * More rows make the result list, which sits below them, too small on ordinary displays; above this
 * number consider putting the rows in a scrolled window. It is also the size of the id space
 * reserved for the rows, see windowIDs::cmdRowBaseId().
 */
constexpr int MaxNrOfCMDs = 25;

static_assert(kNrOfCmds <= MaxNrOfCMDs, "kNrOfCmds exceeds MaxNrOfCMDs");

/*!
 * \brief What a worker thread reports to the main window when its command has finished.
 *
 * It travels as the payload of a wxThreadEvent of type ::wxEVT_THREAD_RESULT.
 */
struct CommandOutcome
{
    int      index = -1;     /*!< zero based index of the command row. */
    bool     pass = false;   /*!< true if the expected result was found. */
    wxString output;         /*!< console output of the command. */
    wxString note;           /*!< extra information (for example "file ... doesn't exist"), may be empty. */
    long     exitCode = -1;  /*!< exit code of the command, -1 if unknown. */
    long     durationMs = 0; /*!< how long the command ran. */
};

wxDECLARE_EVENT(wxEVT_THREAD_RESULT, wxThreadEvent);

/*!
 * \brief Main frame: a table of commands, a "Run command(s)" button, the result file and the list of results.
 *
 * Every command runs in its own worker thread, so the window stays responsive. A command is
 * <em>parallel</em> (started at once) or <em>single</em> (started alone: "Run command(s)" waits for it
 * to finish before it starts the next rows). When a command ends its output is searched for the
 * expected text (or, for results written to a file, the file is searched, see ResultCheck.h); the
 * outcome is added to the list as PASS / FAIL and counted in the row.
 *
 * The <em>result file</em> (entry under the Run button, by default <tt>result.txt</tt> next to the
 * exe) is where batch files append their time stamped PASS / FAIL lines; its path is given to every
 * command in the environment variable PCR_RESULT_FILE. If it does not exist when a command needs
 * it, the operator is asked to create it there or to choose another place.
 *
 * Running commands, "single" commands and the batch file editor are features of the license
 * (LicensePolicy.h); without them a message offers the License window.
 *
 * All wxWidgets windows are touched only from the main thread: the workers send a
 * wxThreadEvent carrying a CommandOutcome.
 */
class MainWindow : public wxFrame
{
public:
    /*! \brief Builds the menus, the command rows, the result list and the layout. */
    MainWindow();

    /*! \brief Detaches the window from the worker threads: commands still running are abandoned and their results dropped. */
    ~MainWindow() override;

private:
    /*! \brief State shared with the worker threads so that they never post to a destroyed window. */
    struct EventSink
    {
        std::mutex   mutex;              /*!< protects \ref target. */
        MainWindow*  target = nullptr;   /*!< window that receives the results, null once it is closing. */
    };

    // --- menu / control handlers -------------------------------------------------------------
    /*! \brief Menu File > Exit. */
    void OnExit(wxCommandEvent& event);
    /*! \brief Menu Info > About: the structured About window. */
    void OnAbout(wxCommandEvent& event);
    /*! \brief Menu Info > License (Ctrl-K). */
    void OnLicense(wxCommandEvent& event);
    /*! \brief Menu File > New batch file: opens the editor with the PASS / FAIL skeleton. */
    void OnNewBatch(wxCommandEvent& event);
    /*! \brief Menu File > Open batch file: opens the editor on a chosen file. */
    void OnOpenBatch(wxCommandEvent& event);
    /*! \brief Menu File > Open examples folder. */
    void OnOpenExamples(wxCommandEvent& event);
    /*! \brief Menu Settings > Open data folder. */
    void OnOpenDataFolder(wxCommandEvent& event);
    /*! \brief Menu Settings > Open result file. */
    void OnOpenResultFile(wxCommandEvent& event);
    /*! \brief Button "..." next to the result file entry, or menu Settings > Select result file. */
    void OnSelectResultFile(wxCommandEvent& event);
    /*! \brief Enter in the result file entry: takes the typed path. */
    void OnResultFileEntered(wxCommandEvent& event);
    /*! \brief The result file entry lost the focus: takes the typed path. */
    void OnResultFileLostFocus(wxFocusEvent& event);
    /*! \brief Every minute: re-evaluates the license when the day has changed. */
    void OnLicenseTimer(wxTimerEvent& event);
    /*! \brief Menu Settings > Enable Edit: unlocks the editable fields of the rows. */
    void OnEnable(wxCommandEvent& event);
    /*! \brief Menu Settings > Disable Edit: locks the editable fields of the rows. */
    void OnDisable(wxCommandEvent& event);
    /*! \brief Menu Settings > Stop waiting: releases a "single" command that blocks "Run command(s)". */
    void OnStopWaiting(wxCommandEvent& event);
    /*! \brief Check boxes of the rows (ON/OFF, single/parallel, show/hide). */
    void OnGuiEvent(wxCommandEvent& event);
    /*! \brief Buttons: the general "Run command(s)" and the "Run" of every row. */
    void OnButtonEvent(wxCommandEvent& event);
    /*! \brief "Run command(s)": runs every active command, honouring single / parallel. */
    void onRunCommand(wxCommandEvent& event);
    /*! \brief Result of a worker thread (wxEVT_THREAD_RESULT). */
    void OnThreadResult(wxThreadEvent& event);
    /*!
     * \brief Closes the batch editors (they may ask to save), asks for confirmation if commands are
     * running, terminates them and closes the window.
     */
    void OnClose(wxCloseEvent& event);
    /*! \brief Ctrl+A selects every row of the list, Ctrl+C copies the selected rows. */
    void OnKeyDown(wxKeyEvent& event);
    /*! \brief Shows the whole text of the row under the mouse as tooltip. */
    void OnMouseMove(wxMouseEvent& event);
    /*! \brief Keeps the list columns proportional to the width of the list. */
    void OnListSize(wxSizeEvent& event);

    // --- helpers --------------------------------------------------------------------------------
    /*!
     * \brief Adds a line at the top of the result list.
     * \param timestamp text of the first column.
     * \param message text of the second column.
     * \param colour text colour of the row, wxNullColour for the default one.
     */
    void AddMessage(const wxString& timestamp, const wxString& message, const wxColour& colour = wxNullColour);
    /*! \brief \return the current local time as "YYYY-MM-DD hh:mm:ss". */
    wxString get_current_timestamp() const;
    /*! \brief Selects every row of the result list. */
    void SelectAllItems();
    /*! \brief Copies the selected rows of the result list (tab separated) to the clipboard. */
    void CopySelectedRow();
    /*! \brief Unlocks the editable fields of every row. */
    void EnableCmds();
    /*! \brief Locks the editable fields of every row. */
    void DisableCmds();
    /*!
     * \brief Unlocks the rows locked by "Run command(s)" once nothing runs any more, unless the
     * operator locked them (Settings > Disable Edit).
     */
    void UnlockWhenIdle();
    /*! \brief \return true if at least one command is running. */
    bool AnyCommandRunning() const;
    /*!
     * \brief Translates a widget id into the row it belongs to.
     * \param id event id.
     * \param[out] offset which widget of the row (one of the cmdgui::*_ID_INDEX values, 0 = ON/OFF).
     * \return the row index, or -1 if \p id does not belong to a command row.
     */
    int RowOfId(int id, int& offset) const;
    /*!
     * \brief Starts the worker thread of command \p commandIndex.
     *
     * The row must already be marked running. When the command ends the thread evaluates the result
     * and posts a ::wxEVT_THREAD_RESULT event to this window.
     * \param input the command line.
     * \param commandIndex row index.
     */
    void StartThread(const wxString& input, int commandIndex);
    /*!
     * \brief Starts a "single" command of "Run command(s)" after asking the operator.
     * \param commandIndex row index; the row is already marked running.
     * \return true if the command was started (the caller must then wait for it).
     */
    bool StartSingleCommand(int commandIndex);
    /*! \brief Keeps the GUI alive until the blocking "single" command ends, is stopped or the window closes. */
    void WaitWhileBlocking();

    // --- result file ------------------------------------------------------------------------
    /*! \brief \return the default result file: result.txt in the folder of the exe. */
    static wxString DefaultResultFile();
    /*! \brief \return the path in the result file entry (the default one if it is empty). */
    wxString ResultFilePath() const;
    /*!
     * \brief Takes \p path as the result file: shows it, saves it and gives it to the commands (PCR_RESULT_FILE).
     * \param path full path of the file (not created here).
     */
    void SetResultFile(const wxString& path);
    /*!
     * \brief Makes sure the result file exists.
     *
     * If it does not, asks whether to create it where it is expected ("Create here"), to choose
     * another place ("Choose location...") or to give up. A new file gets one time stamped line.
     * \return false if the operator gave up or the file could not be created.
     */
    bool EnsureResultFile();
    /*! \brief Creates \p path (and its folder) and appends a time stamped line. \return false on failure. */
    bool CreateResultFile(const wxString& path);
    /*! \brief \return true if the expected result of row \p commandIndex refers to the result file. */
    bool UsesResultFile(int commandIndex);

    // --- license ----------------------------------------------------------------------------
    /*!
     * \brief Checks that a feature is licensed; if not, says why and offers the License window.
     * \param feature a LicensePolicy::kFeature... key.
     * \param featureName translated name of the feature for the message.
     * \return true if the feature may be used.
     */
    bool RequireFeature(const char* feature, const wxString& featureName);
    /*! \brief Shows the license state in the status bar. */
    void UpdateLicenseStatus();
    /*! \brief Opens the License window and refreshes the status bar afterwards. */
    void ShowLicenseDialog();

    // --- data -----------------------------------------------------------------------------------
    bool  m_blocking = false;           /*!< a "single" command is being waited for. */
    int   m_blockingCommandIndex = -1;  /*!< row of that command. */
    bool  m_runAllInProgress = false;   /*!< guards against re-entering onRunCommand() while it waits. */
    bool  m_closing = false;            /*!< the window is being closed, stop waiting and starting. */
    bool  m_userEditLock = false;       /*!< Settings > Disable Edit: the rows stay locked after a run. */
    long  m_lastTipItem = -1;           /*!< list row whose tooltip is shown, -1 for none. */

    std::shared_ptr<EventSink> m_sink = std::make_shared<EventSink>(); /*!< shared with the workers. */

    bool  m_askingResultFile = false;   /*!< the "result file does not exist" question is open. */
    wxTimer m_licenseTimer;             /*!< re-checks the license dates. */

    wxTextCtrl* m_resultFileTxt = nullptr; /*!< shows (and edits) the full path of the result file. */
    wxPanel*    m_mainPanel = nullptr;    /*!< fills the frame. */
    wxButton*   m_runBT = nullptr;        /*!< "Run command(s)". */
    wxListCtrl* m_resultList = nullptr;   /*!< timestamp + information, newest on top. */
    wxBoxSizer* m_cmdsSizer = nullptr;    /*!< vertical sizer holding the rows. */

    std::vector<std::unique_ptr<cmdgui>> m_cmds;     /*!< the command rows. */
    std::array<int, kNrOfCmds> m_passed{};           /*!< PASS counter of every row. */
    std::array<int, kNrOfCmds> m_failed{};           /*!< FAIL counter of every row. */
};
