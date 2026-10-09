/*!
 * \file CommandRunner.cpp
 * \brief Implementation of CommandRunner.h.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */

#include "CommandRunner.h"

#include <wx/msw/wrapwin.h>
#include <wx/strconv.h>

#include <algorithm>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace {

/*! \brief Size of the buffer used to read the pipe of a command (heap, not the thread's small stack). */
constexpr std::size_t kPipeBufferSize = 64 * 1024;

/*! \brief A command that has not ended yet. */
struct Running
{
    HANDLE job = nullptr;      /*!< its job (null if the process could not be put in one). */
    HANDLE process = nullptr;  /*!< cmd.exe, terminated directly when there is no job. */
    bool   stopped = false;    /*!< stop() / stopAll() terminated it. */
    bool   timedOut = false;   /*!< the time limit terminated it. */

    /*! \brief Terminates the process tree. Called with the registry locked. */
    void terminate()
    {
        if (job != nullptr) TerminateJobObject(job, 1);
        else if (process != nullptr) TerminateProcess(process, 1);
    }
};

/*! \brief The commands running, by key; shared by the worker threads and the main window. */
struct Registry
{
    std::mutex mutex;                                  /*!< protects \ref commands. */
    std::map<int, std::shared_ptr<Running>> commands;  /*!< key -> running command. */
};

Registry& registry()
{
    static Registry instance;
    return instance;
}

/*!
 * \brief Converts the bytes written by a console program to text.
 *
 * Console programs started through "cmd /c" write in the OEM code page of the system, not in the
 * ANSI one used by the GUI. Undecodable bytes fall back to a 1:1 mapping.
 */
wxString DecodeConsoleOutput(const std::string& raw)
{
    if (raw.empty()) return wxString();
    wxCSConv conv(wxString::Format("CP%u", static_cast<unsigned>(GetOEMCP())));
    wxString text;
    if (conv.IsOk()) text = wxString(raw.data(), conv, raw.size());
    if (text.empty()) text = wxString::From8BitData(raw.data(), raw.size());
    return text;
}

/*!
 * \brief The environment block of the program: this process's environment with \p extra added or replaced.
 *
 * Built for each command, so every command gets its own values (PCR_CMD_ID) and no thread changes the
 * environment of the others.
 */
std::wstring BuildEnvironment(const std::vector<std::pair<wxString, wxString>>& extra)
{
    std::vector<std::wstring> entries;
    if (wchar_t* block = GetEnvironmentStringsW()) {
        for (const wchar_t* p = block; *p != L'\0'; p += wcslen(p) + 1) entries.emplace_back(p);
        FreeEnvironmentStringsW(block);
    }
    auto nameOf = [](const std::wstring& entry) {
        const size_t eq = entry.find(L'=', 1); // "=C:=C:\..." entries start with '='
        return wxString(entry.substr(0, eq)).Upper();
    };
    for (const auto& [name, value] : extra) {
        const wxString upper = name.Upper();
        entries.erase(std::remove_if(entries.begin(), entries.end(), [&](const std::wstring& e) { return nameOf(e) == upper; }),
                      entries.end());
        entries.push_back((name + "=" + value).ToStdWstring());
    }
    // Windows keeps the block sorted by name, case-insensitively.
    std::sort(entries.begin(), entries.end(), [&](const std::wstring& a, const std::wstring& b) { return nameOf(a) < nameOf(b); });
    std::wstring result;
    for (const std::wstring& e : entries) {
        result += e;
        result += L'\0';
    }
    result += L'\0';
    return result;
}

} // namespace

namespace CommandRunner {

Result run(const wxString& command, const wxString& workingDirectory,
           const std::vector<std::pair<wxString, wxString>>& environment, long timeoutMs, int key)
{
    Result result;
    const auto startTime = std::chrono::steady_clock::now();
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

    HANDLE hRead = nullptr;
    HANDLE hWrite = nullptr;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        result.output = "Error: Failed to create pipe!";
        return result;
    }
    // The child must inherit only the write end.
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    // Standard input is the NUL device, so a command that asks for input ("pause") ends instead of waiting forever.
    HANDLE hNul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = (hNul != INVALID_HANDLE_VALUE) ? hNul : nullptr;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite; // stderr goes to the same pipe as stdout

    // CreateProcessW may modify the command line, so it needs its own writable buffer.
    std::wstring commandLine = L"cmd /c \"" + command.ToStdWstring() + L"\"";
    std::wstring environmentBlock = BuildEnvironment(environment);

    PROCESS_INFORMATION pi = {};
    const std::wstring directory = workingDirectory.ToStdWstring();
    // Started suspended so it is in its job before it can start any child process.
    const BOOL started = CreateProcessW(nullptr, &commandLine[0], nullptr, nullptr, TRUE,
                                        CREATE_NO_WINDOW | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
                                        environmentBlock.data(), directory.empty() ? nullptr : directory.c_str(), &si, &pi);

    // The parent must close its copy of the write end, or ReadFile() would never see the end of the output.
    CloseHandle(hWrite);
    if (hNul != INVALID_HANDLE_VALUE) CloseHandle(hNul);

    if (!started) {
        CloseHandle(hRead);
        result.output = "Error: Failed to execute command!";
        return result;
    }
    result.started = true;

    auto running = std::make_shared<Running>();
    running->process = pi.hProcess;
    running->job = CreateJobObjectW(nullptr, nullptr);
    if (running->job != nullptr && !AssignProcessToJobObject(running->job, pi.hProcess)) {
        CloseHandle(running->job); // the command still runs; stop() terminates only cmd.exe
        running->job = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(registry().mutex);
        registry().commands[key] = running;
    }
    ResumeThread(pi.hThread);

    // The time limit covers the whole output: a program that keeps the pipe open (started by the
    // command) is terminated with it.
    HANDLE done = timeoutMs > 0 ? CreateEventW(nullptr, TRUE, FALSE, nullptr) : nullptr;
    std::thread watchdog;
    if (done != nullptr) {
        watchdog = std::thread([done, timeoutMs, running]() {
            if (WaitForSingleObject(done, static_cast<DWORD>(timeoutMs)) == WAIT_TIMEOUT) {
                std::lock_guard<std::mutex> lock(registry().mutex);
                running->timedOut = true;
                running->terminate();
            }
        });
    }

    std::string raw;
    std::vector<char> buffer(kPipeBufferSize);
    DWORD bytesRead = 0;
    while (ReadFile(hRead, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr) && bytesRead > 0) {
        raw.append(buffer.data(), bytesRead); // append by length: the output may contain NUL bytes
    }
    CloseHandle(hRead);
    WaitForSingleObject(pi.hProcess, INFINITE);

    if (done != nullptr) {
        SetEvent(done);
        watchdog.join();
        CloseHandle(done);
    }

    DWORD exitCode = 0;
    if (GetExitCodeProcess(pi.hProcess, &exitCode)) result.exitCode = static_cast<long>(exitCode);
    {
        std::lock_guard<std::mutex> lock(registry().mutex);
        auto it = registry().commands.find(key);
        if (it != registry().commands.end() && it->second == running) registry().commands.erase(it);
        result.stopped = running->stopped;
        result.timedOut = running->timedOut;
        if (running->job != nullptr) CloseHandle(running->job);
        running->job = nullptr;
        running->process = nullptr;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    result.output = DecodeConsoleOutput(raw);
    result.durationMs = static_cast<long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count());
    return result;
}

bool stop(int key)
{
    std::lock_guard<std::mutex> lock(registry().mutex);
    auto it = registry().commands.find(key);
    if (it == registry().commands.end()) return false;
    it->second->stopped = true;
    it->second->terminate();
    return true;
}

void stopAll()
{
    std::lock_guard<std::mutex> lock(registry().mutex);
    for (auto& [key, running] : registry().commands) {
        running->stopped = true;
        running->terminate();
    }
}

} // namespace CommandRunner
