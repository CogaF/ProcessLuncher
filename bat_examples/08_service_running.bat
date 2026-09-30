@echo off
setlocal
rem ====================================================================================
rem  08_service_running.bat - a Windows service is running
rem  Usage : 08_service_running.bat [service]   (default: EventLog)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [08_service_running] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[08_service_running] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "SERVICE=%~1"
if not defined SERVICE set "SERVICE=EventLog"

rem "sc query" prints STATE : 4  RUNNING when the service runs.
sc query "%SERVICE%" | find "RUNNING" >nul
if errorlevel 1 (
    set "MSG=service %SERVICE% is not running"
    goto :fail
)
set "MSG=service %SERVICE% is running"
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
