@echo off
setlocal
rem ====================================================================================
rem  13_wait_for_file.bat - a file appears within a time limit
rem  Usage : 13_wait_for_file.bat [file] [seconds]   (default: this batch file itself, 30 s)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [13_wait_for_file] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[13_wait_for_file] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FILE=%~1"
set "LIMIT=%~2"
if not defined FILE set "FILE=%~f0"
if not defined LIMIT set "LIMIT=30"

set /a WAITED=0
:waiting
if exist "%FILE%" (
    set "MSG=%FILE% appeared after %WAITED% s"
    goto :pass
)
if %WAITED% geq %LIMIT% (
    set "MSG=%FILE% did not appear within %LIMIT% s"
    goto :fail
)
rem One second of waiting: "timeout" needs a keyboard, ping does not.
ping -n 2 127.0.0.1 >nul
set /a WAITED+=1
goto :waiting

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
