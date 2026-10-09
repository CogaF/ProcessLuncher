/*!
 * \file CommandRunner.h
 * \brief Runs one command line through "cmd /c" without a window: output, exit code, time limit, stop.
 *
 * No GUI here: run() is called in the worker threads. Every command runs in its own Windows job, so
 * its whole process tree (cmd.exe, the batch file and the programs it starts) can be terminated -
 * by stop(), by the time limit or when the program closes (stopAll()). A command that ended on its
 * own leaves the programs it started on purpose ("start ...") running.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/string.h>

#include <vector>
#include <utility>

/*! \brief Running the commands of the rows. */
namespace CommandRunner
{
    /*! \brief How a command ended. */
    struct Result
    {
        wxString output;          /*!< standard output and standard error, decoded with the OEM code page. */
        long     exitCode = -1;   /*!< exit code of cmd.exe (the last command / "exit /b n"); -1 if unknown. */
        bool     started = false; /*!< false if the command could not be started (output says why). */
        bool     timedOut = false; /*!< terminated because the time limit expired. */
        bool     stopped = false; /*!< terminated by stop() or stopAll(). */
        long     durationMs = 0;  /*!< from the start to the end of the output. */
    };

    /*!
     * \brief Runs \p command and waits for it (and for every program that keeps its output open).
     * \param command the command line as typed in the row.
     * \param workingDirectory the current folder of the command.
     * \param environment variables added to (or replacing those of) the program's environment.
     * \param timeoutMs time limit in milliseconds, 0 for none.
     * \param key identifies the command for stop() (the row index); one command per key at a time.
     */
    Result run(const wxString& command, const wxString& workingDirectory,
               const std::vector<std::pair<wxString, wxString>>& environment, long timeoutMs, int key);

    /*! \brief Terminates the command started with \p key, if it runs. \return true if one was running. */
    bool stop(int key);

    /*! \brief Terminates every command still running. */
    void stopAll();
}
