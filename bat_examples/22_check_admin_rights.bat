@echo off
setlocal
rem ====================================================================================
rem  22_check_admin_rights.bat - the batch file runs with administrator rights
rem  Usage : 22_check_admin_rights.bat
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [22_check_admin_rights] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[22_check_admin_rights] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
rem "net session" succeeds only in an elevated prompt.
net session >nul 2>&1
if errorlevel 1 (
    set "MSG=not running as administrator"
    goto :fail
)
set "MSG=running as administrator"
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
