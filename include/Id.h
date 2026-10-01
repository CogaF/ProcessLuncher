/*!
 * \file Id.h
 * \brief Window / menu / control identifiers of Process Launcher.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/defs.h>

/*!
 * \brief Identifiers used by the main window.
 *
 * Every command row (see cmdgui) owns a block of ::windowIDs::kIdsPerCmdRow consecutive ids
 * starting at ::windowIDs::ID_GUI_CLASS. The ids of row <i>i</i> are
 * ID_GUI_CLASS + i * kIdsPerCmdRow + (0 .. kIdsPerCmdRow - 1); use cmdRowBaseId() to compute the base.
 */
namespace windowIDs {

/*! \brief Ids of the fixed controls and menu items. */
enum id {
    ID_MAIN_PANEL = wxID_HIGHEST + 1, /*!< The panel that fills the main frame. */
    ID_RUN_COMMAND_BT,                /*!< The "Run command(s)" button (runs every active command). */
    ID_COMMAND_LIST,                  /*!< The result list. */
    ID_DISABLE_EDIT,                  /*!< Menu: lock the editable fields of the command rows. */
    ID_ENABLE_EDIT,                   /*!< Menu: unlock the editable fields of the command rows. */
    ID_STOP_WAITING,                  /*!< Menu: stop waiting for a "single" (sequential) command. */
    ID_NEW_BATCH,                     /*!< Menu File: new batch file in the editor. */
    ID_OPEN_BATCH,                    /*!< Menu File: open a batch file in the editor. */
    ID_OPEN_EXAMPLES,                 /*!< Menu File: open the folder with the example batch files. */
    ID_LICENSE,                       /*!< Menu Info: the License window. */
    ID_OPEN_DATA_FOLDER,              /*!< Menu Settings: open the data folder. */
    ID_OPEN_RESULT_FILE,              /*!< Menu Settings: open the result file with its default program. */
    ID_SELECT_RESULT_FILE,            /*!< Menu Settings: choose the result file. */
    ID_RESULT_FILE_TXT,               /*!< The text entry that shows the full path of the result file. */
    ID_RESULT_BROWSE_BT,              /*!< The button next to it that chooses another file. */
    ID_LICENSE_TIMER,                 /*!< Timer that re-checks the license dates. */

    ID_GUI_CLASS = wxID_HIGHEST + 251 /*!< First id of the command rows (see cmdRowBaseId()). */
};

/*! \brief Number of consecutive ids reserved for every command row. */
constexpr int kIdsPerCmdRow = 10;

/*!
 * \brief First id of the block reserved for the command row \p rowIndex.
 * \param rowIndex zero based index of the row.
 */
constexpr int cmdRowBaseId(int rowIndex) { return ID_GUI_CLASS + rowIndex * kIdsPerCmdRow; }

} // namespace windowIDs
