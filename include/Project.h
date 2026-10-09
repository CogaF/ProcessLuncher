/*!
 * \file Project.h
 * \brief The command rows saved in a project file (.pcr, INI text) - kept between sessions.
 *
 * No GUI here, so it is covered by tests/ProcessLauncherSelfTest.cpp.
 *
 * \code
 * [Project]
 * Version=1
 * ResultFile=C:\Tests\result.txt
 * Repeat=1
 * [Row01]
 * Active=1
 * Single=0
 * Command=bat_examples\01_minimal_pass_fail.bat
 * Expected=:File:::[01_minimal_pass_fail] PASS
 * Timeout=0
 * \endcode
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/string.h>

#include <vector>

/*! \brief Reading and writing project files. */
namespace Project
{
    /*! \brief Extension of a project file, without the dot. */
    inline const wxString kExtension = "pcr";

    /*! \brief One command row. */
    struct Row
    {
        bool     active = true;    /*!< ON / OFF. */
        bool     single = false;   /*!< Single (true) or Parallel. */
        wxString command;          /*!< the command line. */
        wxString expected;         /*!< the expected result. */
        int      timeout = 0;      /*!< time limit in seconds, 0 = none. */
    };

    /*! \brief Everything a project file holds. */
    struct Data
    {
        std::vector<Row> rows;     /*!< the rows, from the top. */
        wxString resultFile;       /*!< the result file; empty = keep the current one. */
        int      repeat = 1;       /*!< how many times "Run command(s)" runs the rows, 0 = until stopped. */
    };

    /*!
     * \brief Reads a project file.
     * \param path the file.
     * \param[out] data the contents.
     * \param[out] error why the file could not be read.
     * \return false if the file does not exist or is not a project file.
     */
    bool load(const wxString& path, Data& data, wxString& error);

    /*! \brief Writes \p data to \p path (replacing the file). \return false on failure, \p error says why. */
    bool save(const wxString& path, const Data& data, wxString& error);
}
