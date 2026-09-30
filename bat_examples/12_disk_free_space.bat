@echo off
setlocal
rem ====================================================================================
rem  12_disk_free_space.bat - a drive has at least N GB free
rem  Usage : 12_disk_free_space.bat [drive letter] [GB]   (default: C 1) - uses PowerShell
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [12_disk_free_space] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[12_disk_free_space] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "DRIVE=%~1"
set "MINGB=%~2"
if not defined DRIVE set "DRIVE=C"
if not defined MINGB set "MINGB=1"
set "DRIVE=%DRIVE:~0,1%"

powershell -NoProfile -Command "if ((Get-PSDrive %DRIVE%).Free / 1GB -ge %MINGB%) { exit 0 } else { exit 1 }" >nul 2>&1
if errorlevel 1 (
    set "MSG=drive %DRIVE%: has less than %MINGB% GB free"
    goto :fail
)
set "MSG=drive %DRIVE%: has at least %MINGB% GB free"
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
