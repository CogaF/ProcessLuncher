@echo off
setlocal
rem ====================================================================================
rem  09_process_running.bat - a program is running
rem  Usage : 09_process_running.bat [program.exe]   (default: svchost.exe)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [09_process_running] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[09_process_running] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "PROCESS=%~1"
if not defined PROCESS set "PROCESS=svchost.exe"

rem tasklist prints INFO: No tasks ... (and exit code 0) when nothing matches: search for the name.
tasklist /fi "imagename eq %PROCESS%" | find /i "%PROCESS%" >nul
if errorlevel 1 (
    set "MSG=%PROCESS% is not running"
    goto :fail
)
set "MSG=%PROCESS% is running"
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
