@echo off
setlocal
rem ====================================================================================
rem  16_registry_value.bat - a registry value contains a text
rem  Usage : 16_registry_value.bat [key] [value name] [text]   (default: Windows ProductName contains Windows)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [16_registry_value] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[16_registry_value] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "KEY=%~1"
set "NAME=%~2"
set "TEXT=%~3"
if not defined KEY set "KEY=HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion"
if not defined NAME set "NAME=ProductName"
if not defined TEXT set "TEXT=Windows"

reg query "%KEY%" /v "%NAME%" 2>nul | find /i "%TEXT%" >nul
if errorlevel 1 (
    set "MSG=%NAME% of %KEY% does not contain %TEXT%"
    goto :fail
)
set "MSG=%NAME% of %KEY% contains %TEXT%"
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
