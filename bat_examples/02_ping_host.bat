@echo off
setlocal
rem ====================================================================================
rem  02_ping_host.bat - a host answers to ping
rem  Usage : 02_ping_host.bat [host]   (default 127.0.0.1)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [02_ping_host] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[02_ping_host] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "HOST=%~1"
if not defined HOST set "HOST=127.0.0.1"

rem The reply line contains "TTL=": searching it is safer than the exit code of ping, which
rem is 0 also for "Destination host unreachable".
ping -n 2 -w 2000 %HOST% | find "TTL=" >nul
if errorlevel 1 (
    set "MSG=no reply from %HOST%"
    goto :fail
)
set "MSG=%HOST% answers"
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
