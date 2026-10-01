@echo off
setlocal
rem ====================================================================================
rem  23_check_windows_version.bat - Windows 10 or 11 (version number 10.x)
rem  Usage : 23_check_windows_version.bat
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [23_check_windows_version] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[23_check_windows_version] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
rem "ver" prints e.g. "Microsoft Windows [Version 10.0.22631.4037]" (Windows 11 is 10.0.22000+).
ver | find "Version 10." >nul
if errorlevel 1 (
    set "MSG=this is not Windows 10 or 11"
    goto :fail
)
set "MSG=Windows 10 or 11"
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
