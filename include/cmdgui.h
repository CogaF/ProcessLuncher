/*!
 * \file cmdgui.h
 * \brief One row of the command table: the widgets that describe a single command.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/wx.h>

/*!
 * \brief A row of controls that configures one command and shows its state.
 *
 * Layout, left to right: ON/OFF check box, "Run" button, command text, expected-result text,
 * pass/fail counters, single/parallel check box, show/hide check box and a read-only busy/ready
 * check box. The widgets are children of the parent panel and are owned by wxWidgets; the row only
 * keeps pointers to them. The horizontal sizer returned by getPointer() must be added to the
 * parent's layout by the caller.
 *
 * The identifiers of the widgets are <em>base + offset</em> (see the *_ID_INDEX members), where base
 * is getCurrId(), so the owner can tell which row and which widget raised an event.
 *
 * The "expected result" text has two forms:
 *  - plain text: the command passes when its console output contains the text;
 *  - <tt>:File:&lt;path&gt;::&lt;text&gt;</tt>: the command passes when the file contains the text (for
 *    commands that write their result to a file instead of the console). With an empty path,
 *    <tt>:File:::&lt;text&gt;</tt>, the program's result file is meant (see ResultCheck.h).
 */
class cmdgui
{
public:
    /*! \brief Offset (from getCurrId()) of the "single / parallel" check box id. */
    static constexpr int SEQUENTIAL_ID_INDEX = 1;
    /*! \brief Offset of the "Run" button id. */
    static constexpr int RUNBUTTON_ID_INDEX = 2;
    /*! \brief Offset of the command text id. */
    static constexpr int TEXT_ID_INDEX = 3;
    /*! \brief Offset of the expected-result text id. */
    static constexpr int RES_ID_INDEX = 4;
    /*! \brief Offset of the counters label id. */
    static constexpr int COUNTERS_ID_INDEX = 5;
    /*! \brief Offset of the "show / hide" check box id. */
    static constexpr int VIEW_ID_INDEX = 6;
    /*! \brief Offset of the "busy / ready" check box id. */
    static constexpr int RUNNING_ID_INDEX = 7;

    /*! \brief "ON / OFF" check box (id == getCurrId()): whether the command takes part in "Run command(s)". */
    wxCheckBox* Cmd_active_CB = nullptr;
    /*! \brief "Single / Parallel" check box: single commands run alone, the following ones wait for them. */
    wxCheckBox* Cmd_sequential_CB = nullptr;
    /*! \brief "Show / Hide" check box: reserved for showing the console of the command (currently disabled). */
    wxCheckBox* Cmd_view_CB = nullptr;

    /*!
     * \brief Creates row 0 with the default command.
     * \param parentPanel parent of all the widgets.
     */
    explicit cmdgui(wxPanel* parentPanel);

    /*!
     * \brief Creates the row number \p guiIndex with the default command.
     * \param parentPanel parent of all the widgets.
     * \param guiIndex zero based row index, it selects the block of ids (windowIDs::cmdRowBaseId()).
     */
    cmdgui(wxPanel* parentPanel, int guiIndex);

    /*!
     * \brief Creates the row number \p guiIndex with explicit contents.
     * \param parentPanel parent of all the widgets.
     * \param guiIndex zero based row index.
     * \param isActive initial state of the ON/OFF check box.
     * \param sequential initial state of the single/parallel check box.
     * \param cmdName the command line to run.
     * \param positiveValue the expected result (see the class description).
     * \param counters_s initial text of the counters label.
     */
    cmdgui(wxPanel* parentPanel, int guiIndex, bool isActive, bool sequential, const wxString& cmdName,
           const wxString& positiveValue, const wxString& counters_s);

    /*! \brief The widgets are owned by their parent window, nothing to release here. */
    ~cmdgui() = default;

    cmdgui(const cmdgui&) = delete;
    cmdgui& operator=(const cmdgui&) = delete;

    /*! \brief Reads the ON/OFF check box. \return true if the command must be run by "Run command(s)". */
    bool isActive();
    /*! \brief Reads the single/parallel check box. \return true if the command runs alone ("single"). */
    bool isSequential();
    /*! \brief Reads the command text. \return the command line typed by the user. */
    wxString getCmd();
    /*! \brief Reads the expected-result text. \return the text typed by the user. */
    wxString getPositiveVal();

    /*! \brief Shows \p countersString in the counters label. \return false if the widget does not exist. */
    bool setCounters(const wxString& countersString);
    /*! \brief Sets the single/parallel state and its label. \return false if the widget does not exist. */
    bool setSequential(bool sequentialStatus);
    /*! \brief Sets the ON/OFF state, its label and the Run button. \return false if the widget does not exist. */
    bool setActive(bool activeStatus);
    /*! \brief Sets the show/hide state and its label. \return false if the widget does not exist. */
    bool setView(bool viewStatus);
    /*! \brief \return the show/hide state. */
    bool getView() const;
    /*! \brief Sets the command text. \return false if the widget does not exist. */
    bool setCmd(const wxString& cmdName);
    /*! \brief Sets the expected-result text. \return false if the widget does not exist. */
    bool setPostVal(const wxString& positiveValue);
    /*!
     * \brief Marks the command as running (Busy, Run button disabled) or idle (Ready).
     * \return false if the widget does not exist.
     */
    bool setRunning(bool runningStatus);
    /*! \brief \return true while the command is running. */
    bool getRunning() const;

    /*! \brief \return the base id of the row; widget ids are this value plus one of the *_ID_INDEX offsets. */
    int getCurrId() const;

    /*! \brief Switches the row OFF and disables its editable widgets. */
    void disable();
    /*! \brief Switches the row ON and enables its editable widgets. */
    void enable();
    /*! \brief Locks the editable widgets (command, expected result, single/parallel, show/hide). */
    void disableEditables();
    /*! \brief Unlocks the editable widgets, only if the row is ON. */
    void enableEditables();

    /*! \brief Stores the outcome of the last execution (true = PASS). */
    void setResult(bool result);
    /*! \brief \return the outcome of the last execution (true = PASS). */
    bool getResult() const;

    /*! \brief \return the sizer that lays out the row, to be added to the parent's sizer. */
    wxBoxSizer* getPointer();

private:
    /*! \brief Creates the widgets with their final ids and puts them in the sizer. */
    void createControls(wxPanel* parentPanel, int guiIndex);
    /*! \brief Pushes the cached state (_isActive, _isSequential, ...) into the widgets. */
    void refreshWidgets();
    /*! \brief Updates the labels that depend on the active / sequential / view / running state. */
    void refreshLabels();

    wxBoxSizer*   Cmd_sz = nullptr;           /*!< horizontal sizer holding the widgets. */
    wxCheckBox*   Cmd_running_CB = nullptr;   /*!< read-only Busy / Ready indicator. */
    wxTextCtrl*   Cmd_txt = nullptr;          /*!< the command line. */
    wxTextCtrl*   Cmd_res = nullptr;          /*!< the expected result. */
    wxStaticText* Cmd_counters = nullptr;     /*!< "P= nnn || F= nnn" pass / fail counters. */
    wxButton*     Cmd_run_bt = nullptr;       /*!< runs only this command. */

    wxString _cmdName = "ping -n 1 127.0.0.1";                               /*!< default command. */
    wxString _positiveVal = "TTL=";                                           /*!< default expected result. */
    wxString _counters;                       /*!< text of the counters label. */
    bool _isActive = true;                    /*!< ON/OFF. */
    bool _isSequential = false;               /*!< single (true) or parallel (false). */
    int  _thisId = 0;                         /*!< base id of the row. */
    bool _isRunning = false;                  /*!< Busy / Ready. */
    bool _isViewShow = false;                 /*!< Show / Hide. */
    bool _isPass = true;                      /*!< outcome of the last execution. */
};
