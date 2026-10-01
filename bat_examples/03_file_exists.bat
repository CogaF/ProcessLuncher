@echo off
setlocal
rem ====================================================================================
rem  03_file_exists.bat - a file exists
rem  Usage : 03_file_exists.bat [file]   (default: cmd.exe of Windows)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [03_file_exists] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[03_file_exists] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FILE=%~1"
if not defined FILE set "FILE=%SystemRoot%\system32\cmd.exe"

if not exist "%FILE%" (
    set "MSG=file not found: %FILE%"
    goto :fail
)
set "MSG=file found: %FILE%"
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
