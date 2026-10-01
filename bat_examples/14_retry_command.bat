@echo off
setlocal
rem ====================================================================================
rem  14_retry_command.bat - a command succeeds within N attempts
rem  Usage : 14_retry_command.bat [attempts] [command...]   (default: 3 attempts of ping -n 1 127.0.0.1)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [14_retry_command] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[14_retry_command] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "ATTEMPTS=%~1"
if not defined ATTEMPTS set "ATTEMPTS=3"
set "COMMAND=ping -n 1 127.0.0.1"
rem Everything after the first argument is the command.
set "REST="
for /f "tokens=1,* delims= " %%a in ("%*") do set "REST=%%b"
if defined REST set "COMMAND=%REST%"

set /a TRY=0
:again
set /a TRY+=1
%COMMAND% >nul 2>&1
if not errorlevel 1 (
    set "MSG=succeeded at attempt %TRY% of %ATTEMPTS%"
    goto :pass
)
if %TRY% lss %ATTEMPTS% (
    ping -n 3 127.0.0.1 >nul
    goto :again
)
set "MSG=still failing after %ATTEMPTS% attempts"
goto :fail

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
