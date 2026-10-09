# Process Launcher 0.2.0-rc.1 - binaries

Release candidate, 2026-09-30. Parallel Command Runner: runs commands in parallel or one after the
other, analyses the output (or a result file) and records **PASS** / **FAIL**.

## What's new since 0.1.0

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

## Changes after 0.2.0-rc.1 (not released yet)

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
