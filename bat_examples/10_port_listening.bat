@echo off
setlocal
rem ====================================================================================
rem  10_port_listening.bat - a TCP port is open (a program listens on it)
rem  Usage : 10_port_listening.bat [port]   (default: 135)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [10_port_listening] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[10_port_listening] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "PORT=%~1"
if not defined PORT set "PORT=135"

rem The space after the port avoids matching :1350 when looking for :135.
netstat -ano | find ":%PORT% " | find "LISTENING" >nul
if errorlevel 1 (
    set "MSG=nobody listens on port %PORT%"
    goto :fail
)
set "MSG=port %PORT% is listening"
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
