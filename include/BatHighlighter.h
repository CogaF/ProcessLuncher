/*!
 * \file BatHighlighter.h
 * \brief Syntax analysis of batch (cmd.exe) text for the editor: which parts are commands, flow
 * control, comments, labels, variables, strings, operators and the PASS / FAIL words.
 *
 * No GUI here, so it is covered by tests/ProcessLauncherSelfTest.cpp.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 */
#pragma once

#include <vector>

#include <wx/string.h>

/*! \brief The kinds of text the highlighter tells apart. */
enum class BatToken
{
    Command,  /*!< a command or program in command position (shown bold). */
    Flow,     /*!< if, else, for, goto, call, exit, not, exist, errorlevel... (shown bold). */
    Comment,  /*!< rem ... or :: ... up to the end of the line. */
    Label,    /*!< ":name" that starts a line, or the target of goto / call. */
    Variable, /*!< %VAR%, %1, %~dp0, %%i, !VAR!. */
    String,   /*!< "quoted text". */
    Operator, /*!< & | > < ( ) ^ @ and the == of a comparison. */
    Switch,   /*!< /i  /b  /s ... */
    Pass,     /*!< the word PASS. */
    Fail      /*!< the word FAIL. */
};

/*! \brief A piece of the text and its kind; positions are in characters (line breaks count one, CR is ignored). */
struct BatSpan
{
    size_t   start;  /*!< position of the first character. */
    size_t   length; /*!< number of characters. */
    BatToken kind;   /*!< what it is. */
};

/*!
 * \brief Analyses a whole batch file.
 *
 * The result is ordered so that a later span may overlap an earlier one (variables inside strings):
 * apply them in order and the later one wins.
 * \param text the batch file; carriage returns are removed before counting positions.
 * \return the spans, everything not listed is plain text.
 */
std::vector<BatSpan> HighlightBatch(const wxString& text);
