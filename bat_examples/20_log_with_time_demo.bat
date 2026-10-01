@echo off
setlocal
rem ====================================================================================
rem  20_log_with_time_demo.bat - the ways to write a time stamped line to the result file
rem  Usage : 20_log_with_time_demo.bat
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [20_log_with_time_demo] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[20_log_with_time_demo] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
rem 1) The :log routine at the bottom of every example: "date time [name] text", appended.
call :log INFO written by the :log routine

rem 2) By hand, in one line (the >> goes at the END and appends; a single > would overwrite the file).
set "T=%time: =0%"
>>"%RESULT_FILE%" echo %date% %T:~0,8% [%TEST_NAME%] INFO written by hand

rem 3) An ISO time stamp (2026-09-30T21:45:10) from PowerShell - slower, but the same on every PC.
for /f "delims=" %%t in ('powershell -NoProfile -Command "Get-Date -Format s"') do set "ISO=%%t"
>>"%RESULT_FILE%" echo %ISO% [%TEST_NAME%] INFO ISO time stamp

rem The real result goes last: Process Launcher looks for PASS / FAIL among the lines written by this run.
set "MSG=three lines appended to %RESULT_FILE%"
goto :pass

:pass
call :log PASS %MSG%
exit /b 0

:fail
call :log FAIL %MSG%
exit /b 1

:log
rem Appends "date time [name] text" to the result file (never overwrites it) and prints the text too.
set "T=%time: =0%"
>>"%RESULT_FILE%" echo %date% %T:~0,8% [%TEST_NAME%] %*
echo %*
goto :eof
