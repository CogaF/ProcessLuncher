/*!
 * \file BatCommands.h
 * \brief The reference of the batch (cmd.exe) commands shown in the batch file editor.
 *
 * Every entry has a short summary (shown as tooltip and in the status bar), the details and an
 * example (shown by a right click in a message box) and the text that a click inserts in the file.
 * The table also tells the syntax highlighter which words are commands and which are flow control.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 */
#pragma once

#include <vector>

#include <wx/arrstr.h>
#include <wx/string.h>

/*! \brief One batch command, operator or snippet of the reference. */
struct BatCommand
{
    const char* category;   /*!< group of the pane, see BatCommands::categories(). */
    const char* name;       /*!< the command as typed, also the text of its button. */
    const char* insertText; /*!< text inserted in the editor by a click (may contain new lines). */
    const char* summary;    /*!< one line explanation: tooltip and status bar. */
    const char* details;    /*!< longer explanation: syntax, switches, exit codes. */
    const char* example;    /*!< example of use (several lines allowed). */
};

/*! \brief Access to the command reference and to the word classes used for highlighting. */
namespace BatCommands
{
    /*! \brief Every entry, grouped by category in the order they are listed. */
    const std::vector<BatCommand>& all();
    /*! \brief The category names in display order. */
    const wxArrayString& categories();
    /*! \brief true if \p word (any case) is a built-in command or a common console tool. */
    bool isCommand(const wxString& word);
    /*! \brief true if \p word (any case) is flow control (if, else, for, goto, call, exit, not, exist...). */
    bool isFlow(const wxString& word);
    /*! \brief The text of the message box of a right click: summary, details and example. */
    wxString infoText(const BatCommand& command);
}
