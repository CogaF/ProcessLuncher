# Process Launcher (PCR - Parallel Command Runner)

Launches different commands in parallel (or one after the other), analyses the result and records it
as **PASS** or **FAIL**.

Each row of the table is one command:

| Field | Meaning |
|---|---|
| ON / OFF | whether "Run command(s)" starts it |
| Run | starts only this command |
| command | anything `cmd /c` understands (a `.bat` file, `ping ...`, ...) |
| expected result | text that must appear in the output of the command, `:File:<path>::<text>` to look for `<text>` in a file (for commands that write their result to a file instead of the console), or `:Exit:<codes>` for the exit code (`:Exit:0`, `:Exit:0-7,16`); `{id}` is replaced by the row number |
| time limit | seconds after which a running command is terminated and counted FAIL (0 = none) |
| counters | how many times the command passed / failed |
| Parallel / Single | *Parallel* commands start at once; a *Single* command starts alone and "Run command(s)" waits for it (Settings > Stop waiting, Ctrl-B, releases it) |
| Busy / Ready | the command is running / idle |

The rows are saved in a **project file** (`Process Launcher data\commands.pcr` unless File > Save project as
chose another) when the window closes and at every run, and come back at the next start; File > Open
project switches between sets of commands.

While "Run command(s)" runs, the rows are locked; they are unlocked when the last command ends (Settings >
Disable Edit keeps them locked, Enable Edit unlocks them). Closing the window terminates the unfinished
commands and everything they started.

Each command runs in its own thread, so the GUI never freezes. Results are added to the list at the
bottom (newest on top; green = PASS, red = FAIL). In the list Ctrl+A selects everything, Ctrl+C copies
the selected rows, the tooltip shows the whole text of a row.

The program always starts in **dark mode** (wxWidgets 3.3+).

## Command line

```bat
start /wait "" "Process Launcher.exe" --run --project tests.pcr --repeat 10 --csv results.csv
echo %errorlevel%
```

`--run` runs the rows without questions, exports the CSV and closes: exit code 0 = every result PASS,
1 = at least one FAIL, 2 = nothing could run (license, project, no row ON). A `.pcr` file given alone
just opens. `--screenshots <folder>` saves pictures of the windows for the manual.

## Result file

Under the Run button the **Result file** entry shows the full path of the file where batch files append
their time stamped PASS / FAIL lines - by default `result.txt` in the folder of the exe - and `...`
chooses another one. Commands receive it in the environment variable `PCR_RESULT_FILE`. When a command
needs it and it does not exist, the program asks whether to create it there or to choose a place.
An expected result of the form `:File:::PASS` (empty path) looks in this file, **only at what the
command appended after it started**, so a PASS left by an earlier run is never counted.
`:File:<path>::<text>` looks in the whole of another file.

## Batch files

`bat_examples\` has ready-made batch files that report PASS / FAIL and log to the result file (see its
README). **File > Open batch file** (or **New batch file**) opens an editor with highlighting (commands
bold, flow words, comments, labels, variables, strings, PASS / FAIL), A- / A+ for the text size and a
pane with a button for every batch command: click to insert it, hover for the explanation (tooltip and
status bar), right click for the details and an example.

## License

Running commands, *Single* mode and the batch editor need a license (14-day trial, then per-PC
licenses): **Info > License**, see [LICENSING.md](LICENSING.md). **Info > About** shows the structured
information window (about, license, changes, system).

## Manual

`manual\Process_Launcher_User_Manual.pdf` is the user manual. `python manual\make_manual.py` rebuilds it
(reportlab, pillow); owner, version, license features and the list of examples are read from the source.
Change its text and raise `REVISION` when a release changes what it states.

## Building

Visual Studio 2022 / 2026, wxWidgets 3.3.x (3.3.3 recommended) - see [DEPENDENCIES.md](DEPENDENCIES.md).
Open `ProcessLuncher.sln`; configurations Debug / Release (static wxWidgets) and Debug_DLL /
Release_DLL (wxWidgets DLLs, copied next to the exe), x64 and Win32.

`doxygen` in this folder generates the documentation (`docs/html/index.html`).

Depends on the wxWidgets GUI library: <https://github.com/wxWidgets/wxWidgets>

## Licence of the source

MIT, see [LICENSE](LICENSE) - except the license-system files (`License*`, `Ed25519`, `HashUtils`,
`MachineId`, `StatusLed`, `TextUtils`, `TimeUtils`), copied from the author's other products with their
own header: Copyright (C) 2026 Fation Coga, proprietary.
