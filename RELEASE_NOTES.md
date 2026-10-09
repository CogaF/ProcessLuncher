# Process Launcher 0.2.0-rc.2 - binaries

Release candidate, 2026-10-09. Parallel Command Runner: runs commands in parallel or one after the
other, analyses the output (or a result file) and records **PASS** / **FAIL**.

## What's new in 0.2.0-rc.1 (since 0.1.0)

**Added**
- wxWidgets 3.3 and dark mode (the program always starts in dark mode).
- Result file next to the exe: path shown and editable, created on request, and searched only from where
  it ended when the command started (a PASS left by an earlier run is never counted).
- `bat_examples\` folder: batch files that report PASS / FAIL and append a time stamped line to the
  result file.
- File > Open batch file: batch editor with highlighting, font size buttons and a reference of the
  batch commands.
- Per-PC licenses (features `run`, `sequential`, `editor`), 14-day trial with every feature.
- Structured About window (Info > About).

**Fixed**
- Duplicate event symbol, uninitialised counters, worker threads calling the GUI, result text used as
  a format string, broken `:File:` parsing, crash when opening the batch editor.

## What's new in 0.2.0-rc.2

**Added**
- Time limit per row (seconds, 0 = none): a command still running then is terminated with everything it
  started and counted as FAIL.
- The Run button of a row becomes **Stop** while its command runs.
- Each command gets `PCR_CMD_ID` (its row number) besides `PCR_RESULT_FILE` and `PCR_APP_DIR`, in its own
  environment.
- **The command rows are kept**: they are saved in a project file (`commands.pcr` in the data folder by
  default) when the window closes and at every "Run command(s)", and loaded at the next start. File >
  Open project / Save project (Ctrl+S) / Save project as keep several sets of commands; a project also
  holds the result file and the repeat count.
- The result list shows the exit code and the duration of every run.
- **Repeat** field next to "Run command(s)": the rows are run that many times (0 = until Settings > Stop
  repeating, Ctrl+R); each run waits for all its commands and ends with a summary line (PASS / FAIL counts and
  the failed rows), followed by a total. The questions of *Single* commands are asked in the first run only.
- Expected result `:Exit:<codes>`: PASS when the exit code is in the list, e.g. `:Exit:0`, `:Exit:0-7,16`.
- `{id}` in an expected result is replaced by the row number: `:File:::[test#{id}] PASS` tells apart rows
  running the same batch file in parallel (the batch file writes `[%~n0#%PCR_CMD_ID%]`).
- File > Export results (CSV): every result of the session (time, run, row, command, expected result,
  PASS / FAIL, exit code, duration, note, output), with the list separator of the Windows regional
  settings so Excel opens it in columns. File > Clear results empties the list.

- Command line: `"Process Launcher.exe" [project.pcr] [--project <file>] [--run] [--repeat <n>] [--csv <file>]`.
  `--run` runs the rows without questions, exports the CSV, prints a summary to the console it was started
  from and closes with exit code 0 (all PASS), 1 (a FAIL) or 2 (nothing could run). It changes no file
  but the result file and the CSV. In a batch file: `start /wait "" "Process Launcher.exe" --run ...`.

- `--screenshots <folder>`: demonstration rows (run when licensed; no project or setting written), then
  pictures of the main window, the batch editor and the About window for the manual.

**Changed**
- "Run command(s)" now waits until all its commands have ended (Stop waiting, Ctrl+B, releases the wait).
- Contact e-mail: coga.fation@gmail.com (About window, manual).

**Fixed**
- Closing the window while "Run command(s)" waited for a *Single* command could crash (the window was
  destroyed while still in use).
- Closing the window now really terminates the unfinished commands, with every program they started
  (each command runs in its own Windows job). Programs left running by commands that already ended
  (`start ...`) are not touched.
- Closing the window closed the batch editors without asking to save their changes.
- After "Run command(s)" the rows stayed locked until Settings > Enable Edit; they are unlocked when the
  last command ends (unless Settings > Disable Edit locked them).
- Answering *No* to "launch all the not running commands?" left the rows locked.
- The "previous failure" question of a *Single* command looked only at the row above; it now names every
  earlier row that failed.
- Batch editor: a file is saved in the encoding it was read in. Files were read in the ANSI code page and
  always saved in UTF-8, so accented letters (in paths, `echo` texts) changed and cmd.exe, which reads
  batch files in the OEM code page, could no longer find the paths. New files use the OEM code page; text
  it cannot hold is saved in UTF-8 with a warning (add `chcp 65001 >nul`). A UTF-8 byte order mark is kept.
- The default first row looks for `:File:::[01_minimal_pass_fail] PASS`, so a PASS written by another
  command running at the same time is not counted for it.

## Which download to use

| Package | Contents | Size (approx.) | Use it when |
|---|---|---|---|
| **x64 static** (`Builds\x64\Release`) | `Process Launcher.exe`, `bat_examples\` | 5 MB | normal 64-bit Windows: one file, nothing else to copy |
| **Win32 static** (`Builds\Win32\Release`) | same, 32-bit | 5 MB | 32-bit Windows |
| **x64 DLL** (`Builds\x64\Release_DLL`) | exe + `wxbase333u_vc_x64_custom.dll`, `wxmsw333u_core_vc_x64_custom.dll` | 0.8 MB + 11.7 MB | several wxWidgets programs share the DLLs |
| **Win32 DLL** (`Builds\Win32\Release_DLL`) | exe + the two 32-bit wx DLLs | 0.7 MB + 10.7 MB | same, 32-bit |

Keep the DLLs in the same folder as the exe. The `.pdb` files are only for debugging; do not ship them.
Debug builds are for development only (they need the Visual C++ debug runtime).

## Requirements

- Windows 10 or 11.
- **Microsoft Visual C++ Redistributable 2015-2022 (or later)**, matching the bitness (x64 / x86). The
  runtime is linked dynamically, so a PC without it reports a missing `VCRUNTIME140.dll` /
  `MSVCP140.dll`.
- No installation: unzip anywhere. The program writes `result.txt`, its settings and log in a data
  folder next to the exe / user data folder, so use a folder you can write to.

## First start and license

- The first start begins a **14-day trial** with every feature. Afterwards the window, About and License
  windows still open, but running commands, *Single* mode and the batch editor need a license.
- **Info > License** (Ctrl-K) shows the PC's UID and saves `license-request.txt`; send it to the license
  issuer and install the returned `.lic` with **Load License...**, **Paste License** or by dropping it on
  the License window. See [LICENSING.md](LICENSING.md).
- Setting the PC clock back by more than a few days suspends the trial and time-limited licenses
  until the clock is right again.

## Known limitations

- Release candidate: please report problems with the batch editor and the result-file handling in
  particular.
- Binaries are not code-signed, so Windows SmartScreen may warn on first start ("More info > Run anyway").

## Verifying a download

```
certutil -hashfile "Process Launcher.exe" SHA256
```

Compare with the SHA-256 published next to the download.

## Source and licence

Source: <https://github.com/CogaF/ProcessLuncher>. MIT, except the license-system files - see
[README.md](README.md). Built with Visual Studio 2026 (toolset v145) and wxWidgets 3.3.3.
