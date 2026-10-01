@echo off
setlocal
rem ====================================================================================
rem  27_file_is_recent.bat - a file was modified in the last N minutes
rem  Usage : 27_file_is_recent.bat [file] [minutes]   (default: the result file, 1440) - uses PowerShell
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [27_file_is_recent] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[27_file_is_recent] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FILE=%~1"
set "MINUTES=%~2"
if not defined FILE set "FILE=%RESULT_FILE%"
if not defined MINUTES set "MINUTES=1440"

if not exist "%FILE%" (
    set "MSG=file not found: %FILE%"
    goto :fail
)
set "CHECKFILE=%FILE%"
powershell -NoProfile -Command "if ((Get-Item -LiteralPath $env:CHECKFILE).LastWriteTime -gt (Get-Date).AddMinutes(-[int]%MINUTES%)) { exit 0 } else { exit 1 }" >nul 2>&1
if errorlevel 1 (
    set "MSG=%FILE% is older than %MINUTES% minutes"
    goto :fail
)
set "MSG=%FILE% was modified in the last %MINUTES% minutes"
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
