# Process Launcher (PCR - Parallel Command Runner)

Launches different commands in parallel (or one after the other), analyses the result and records it
as **PASS** or **FAIL**.

Each row of the table is one command:

| Field | Meaning |
|---|---|
| ON / OFF | whether "Run command(s)" starts it |
| Run | starts only this command |
| command | anything `cmd /c` understands (a `.bat` file, `ping ...`, ...) |
| expected result | text that must appear in the output of the command, or `:File:<path>::<text>` to look for `<text>` in a file (for commands that write their result to a file instead of the console) |
| counters | how many times the command passed / failed |
| Parallel / Single | *Parallel* commands start at once; a *Single* command starts alone and "Run command(s)" waits for it (Settings > Stop waiting, Ctrl-B, releases it) |
| Busy / Ready | the command is running / idle |

Each command runs in its own thread, so the GUI never freezes. Results are added to the list at the
bottom (newest on top; green = PASS, red = FAIL). In the list Ctrl+A selects everything, Ctrl+C copies
the selected rows, the tooltip shows the whole text of a row.

The program always starts in **dark mode** (wxWidgets 3.3+). Note that with
`:File:` the file is read when the command has finished: a result file left over from an earlier
run can make a command pass, delete it in the command itself if that matters.

## Building

Visual Studio 2022 / 2026, wxWidgets 3.3.x (3.3.3 recommended) - see [DEPENDENCIES.md](DEPENDENCIES.md).
Open `ProcessLuncher.sln`; configurations Debug / Release (static wxWidgets) and Debug_DLL /
Release_DLL (wxWidgets DLLs, copied next to the exe), x64 and Win32.

`doxygen` in this folder generates the documentation (`docs/html/index.html`).

Depends on the wxWidgets GUI library: <https://github.com/wxWidgets/wxWidgets>

## Licence

MIT, see [LICENSE](LICENSE).
