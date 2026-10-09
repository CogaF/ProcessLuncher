/*!
 * \file ResultCsv.h
 * \brief The results of the commands as a CSV file (File > Export results, command line --csv).
 *
 * No GUI here, so it is covered by tests/ProcessLauncherSelfTest.cpp.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/string.h>

#include <vector>

/*! \brief Recording and exporting results. */
namespace ResultCsv
{
    /*! \brief One result: a row of the CSV file. */
    struct Record
    {
        wxString timestamp;        /*!< when the command ended, "YYYY-MM-DD hh:mm:ss". */
        int      run = 0;          /*!< number of the repetition of Run command(s); 0 = Run button of the row. */
        int      row = 0;          /*!< row number, from 1. */
        wxString command;          /*!< the command line. */
        wxString expected;         /*!< the expected result. */
        bool     pass = false;     /*!< PASS or FAIL. */
        long     exitCode = -1;    /*!< exit code, -1 if unknown. */
        long     durationMs = 0;   /*!< how long it ran. */
        wxString note;             /*!< why the check failed or the command was terminated, may be empty. */
        wxString output;           /*!< console output. */
    };

    /*! \brief Quotes a field when it contains the separator, a quote or a line break. */
    wxString quote(const wxString& field, wxChar separator);

    /*! \brief The CSV text: a header line, then one line per record (line ends CRLF). */
    wxString format(const std::vector<Record>& records, wxChar separator);

    /*!
     * \brief Writes the CSV file in UTF-8 with a byte order mark (so Excel reads the accents right).
     * \return false on failure, \p error says why.
     */
    bool write(const wxString& path, const std::vector<Record>& records, wxChar separator, wxString& error);
}
