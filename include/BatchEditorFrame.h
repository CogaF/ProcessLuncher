/*!
 * \file BatchEditorFrame.h
 * \brief Window to view and edit a batch file, with syntax highlighting and a pane of buttons for
 * every batch command.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 */
#pragma once

#include <vector>

#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

#include "BatCommands.h"

/*!
 * \brief The batch file editor.
 *
 * - Left: the text, in a monospaced font (Courier New), editable and selectable. Commands and flow
 *   words are bold, every kind of word has its colour (see BatToken); PASS and FAIL stand out.
 *   The highlighting follows the typing a moment after the last key.
 * - Toolbar: New, Open, Save, Save As, smaller / larger text.
 * - Right: a scrolling pane with a button for every command of BatCommands. A click inserts the
 *   command at the cursor; while the mouse is over a button its explanation is in the tooltip and
 *   the status bar; a right click opens a message box with the details and an example. The box
 *   above the pane filters the buttons.
 */
class BatchEditorFrame : public wxFrame
{
public:
    /*!
     * \brief Opens the editor.
     * \param parent the main window (may be null).
     * \param path batch file to load; empty for a new, unnamed file.
     */
    explicit BatchEditorFrame(wxWindow* parent, const wxString& path = wxString());

    /*! \brief Loads \p path in the editor (asks to save the current text first). \return false on failure. */
    bool LoadFile(const wxString& path);

private:
    /*! \brief One button of the pane, kept to filter it. */
    struct PaneItem
    {
        wxWindow*   window;    /*!< the button or the category label. */
        bool        isHeader;  /*!< true for a category label. */
        wxString    haystack;  /*!< lower case text searched by the filter (name + summary). */
        size_t      category;  /*!< index of the category it belongs to. */
    };

    /*! \brief Builds the toolbar. */
    wxWindow* BuildToolbar(wxWindow* parent);
    /*! \brief Builds the pane of command buttons. */
    wxWindow* BuildPane(wxWindow* parent);
    /*! \brief Re-applies the colours and styles (after editing or a change of size). */
    void Rehighlight();
    /*! \brief Applies font, size and colours to the whole text. */
    void ApplyBaseStyle();
    /*! \brief Changes the text size by \p delta points (limited to 6..48) and saves it. */
    void ChangeFontSize(int delta);
    /*! \brief Asks to save if the text was changed. \return false if the user cancelled. */
    bool ConfirmDiscard();
    /*! \brief Saves to the current file, asking for one if there is none. \return true if saved. */
    bool Save();
    /*! \brief Asks for a file name and saves. \return true if saved. */
    bool SaveAs();
    /*!
     * \brief Writes the text to \p path (Windows line ends, as cmd.exe expects) in the encoding it was
     * read in; text the OEM code page cannot hold is written in UTF-8 (the operator is told). \return true on success.
     */
    bool WriteFile(const wxString& path);
    /*! \brief Updates the window title (file name, * when modified) and the status bar. */
    void UpdateTitle();
    /*! \brief Updates the line / column field of the status bar. */
    void UpdatePosition();
    /*! \brief Inserts \p text at the cursor, replacing the selection. */
    void InsertCommand(const wxString& text);
    /*! \brief Shows or hides the pane buttons that match the filter text. */
    void ApplyFilter();
    /*! \brief Hooks hover / right click behaviour to a pane button. */
    void WireButton(wxButton* button, const BatCommand& command);

    void OnNew(wxCommandEvent& event);            /*!< toolbar / menu: new file. */
    void OnOpen(wxCommandEvent& event);           /*!< toolbar / menu: open file. */
    void OnSave(wxCommandEvent& event);           /*!< toolbar / menu: save. */
    void OnSaveAs(wxCommandEvent& event);         /*!< toolbar / menu: save as. */
    void OnBigger(wxCommandEvent& event);         /*!< toolbar: larger text. */
    void OnSmaller(wxCommandEvent& event);        /*!< toolbar: smaller text. */
    void OnClose(wxCloseEvent& event);            /*!< asks to save before closing. */
    void OnTextChanged(wxCommandEvent& event);    /*!< marks modified, schedules the highlighting. */
    void OnHighlightTimer(wxTimerEvent& event);   /*!< the highlighting delay has passed. */
    void OnCaretMoved(wxEvent& event);            /*!< key or mouse: update line / column. */
    void OnMouseWheel(wxMouseEvent& event);       /*!< Ctrl + wheel changes the text size. */

    wxTextCtrl*       m_text = nullptr;       /*!< the batch text. */
    wxTextCtrl*       m_filter = nullptr;     /*!< filter box of the pane. */
    wxScrolledWindow* m_pane = nullptr;       /*!< scrolling pane of command buttons. */
    wxBoxSizer*       m_paneSizer = nullptr;  /*!< sizer of the pane. */
    wxStaticText*     m_sizeLabel = nullptr;  /*!< shows the text size in the toolbar. */
    wxTimer           m_timer;                /*!< delays the highlighting after typing. */
    std::vector<PaneItem> m_items;            /*!< buttons and labels of the pane. */
    wxArrayString m_categoryNames;           /*!< names of the categories. */

    wxString m_path;            /*!< file being edited, empty if unnamed. */
    bool     m_dirty = false;   /*!< the text was changed since the last save. */
    bool     m_utf8 = false;    /*!< the file is UTF-8; false: OEM code page, as cmd.exe reads it. */
    bool     m_utf8Bom = false; /*!< the UTF-8 file starts with a byte order mark (kept when saving). */
    bool     m_busy = false;    /*!< the code itself is changing the text or its style. */
    int      m_fontSize = 11;   /*!< text size in points. */
};
