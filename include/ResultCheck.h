/*!
 * \file ResultCheck.h
 * \brief Decides whether a command passed: searches its output, or a result file, for a text.
 *
 * No GUI here, so the functions run in the worker threads and are covered by
 * tests/ProcessLauncherSelfTest.cpp.
 *
 * The "expected result" of a command has two forms:
 *  - plain text: PASS when the console output of the command contains it;
 *  - <tt>:File:&lt;path&gt;::&lt;text&gt;</tt>: PASS when the file contains the text. An empty path
 *    (<tt>:File:::&lt;text&gt;</tt>) means the program's <em>result file</em> (shown in the main
 *    window, by default <tt>result.txt</tt> next to the exe). For the result file only the lines
 *    written after the command started are searched, so a PASS left by an earlier run is ignored.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 */
#pragma once

#include <wx/string.h>
#include <wx/filefn.h>

/*! \brief Evaluation of the expected result of a command. */
namespace ResultCheck
{
    /*! \brief Prefix of an expected result that refers to a file. */
    inline const wxString kFileTag = ":File:";
    /*! \brief Separates the file name from the text to find. */
    inline const wxString kSeparator = "::";

    /*! \brief An expected result taken apart. */
    struct Spec
    {
        bool     isFile = false;        /*!< true if the expected result refers to a file. */
        bool     valid = true;          /*!< false if it starts with the file tag but has no separator. */
        bool     usesResultFile = false; /*!< true if the path is empty: the program's result file is meant. */
        wxString path;                  /*!< the file, with %VARIABLES% expanded; empty for the result file. */
        wxString text;                  /*!< the text to find (the whole expected result for plain text). */
    };

    /*! \brief Takes an "expected result" field apart. \param expected the text of the field. */
    Spec parse(const wxString& expected);

    /*!
     * \brief Searches a file, line by line (it is never loaded entirely), for a text.
     *
     * The text cannot span several lines.
     * \param path the file to read.
     * \param text the text to find.
     * \param startOffset only the bytes from this position on are searched (0 = the whole file).
     * \param[out] opened false if the file could not be opened.
     * \return true if a line contains \p text.
     */
    bool findInFile(const wxString& path, const wxString& text, wxFileOffset startOffset, bool& opened);

    /*! \brief Size of a file in bytes, 0 if it does not exist. */
    wxFileOffset fileSize(const wxString& path);

    /*!
     * \brief Decides PASS or FAIL. Called in the worker thread when the command has ended.
     * \param expected the "expected result" field.
     * \param output console output of the command.
     * \param resultFile full path of the program's result file.
     * \param resultFileOffset size of the result file when the command started.
     * \param[out] note explanation when the check could not be done (missing file...), else left empty.
     * \param[out] missingResultFile set to the path when the result file was needed but does not exist.
     * \return true for PASS.
     */
    bool evaluate(const wxString& expected, const wxString& output, const wxString& resultFile,
                  wxFileOffset resultFileOffset, wxString& note, wxString& missingResultFile);
}
