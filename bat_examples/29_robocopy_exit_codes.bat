@echo off
setlocal
rem ====================================================================================
rem  29_robocopy_exit_codes.bat - a folder is copied with robocopy (exit codes 0-7 are success)
rem  Usage : 29_robocopy_exit_codes.bat [source] [target]   (no arguments: copies a small test folder)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [29_robocopy_exit_codes] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[29_robocopy_exit_codes] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "SOURCE=%~1"
set "TARGET=%~2"
if defined TARGET goto :copy

rem No arguments: build a tiny source folder in the temp folder.
set "SOURCE=%TEMP%\pcr_robo_src"
set "TARGET=%TEMP%\pcr_robo_dst"
if not exist "%SOURCE%\" mkdir "%SOURCE%"
echo sample> "%SOURCE%\sample.txt"

:copy
robocopy "%SOURCE%" "%TARGET%" /e /r:1 /w:1 /nfl /ndl /njh /njs >nul
set "CODE=%errorlevel%"
rem robocopy uses bit flags: 1 = files copied, 2 = extra files, 4 = mismatches. 8 or more = failure.
if %CODE% geq 8 (
    set "MSG=robocopy failed with code %CODE%"
    goto :fail
)
set "MSG=robocopy ended with code %CODE% (0-7 is success)"
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
