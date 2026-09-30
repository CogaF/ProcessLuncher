@echo off
setlocal
rem ====================================================================================
rem  05_text_in_file.bat - a text is present in a file
rem  Usage : 05_text_in_file.bat [file] [text]   (default: win.ini, [fonts])
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [05_text_in_file] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[05_text_in_file] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FILE=%~1"
set "TEXT=%~2"
if not defined FILE set "FILE=%SystemRoot%\win.ini"
if not defined TEXT set "TEXT=[fonts]"

if not exist "%FILE%" (
    set "MSG=file not found: %FILE%"
    goto :fail
)
rem /i ignore case, /c:"..." the text as it is (spaces included), >nul hides the found line
findstr /i /c:"%TEXT%" "%FILE%" >nul
if errorlevel 1 (
    set "MSG=text not found in %FILE%"
    goto :fail
)
set "MSG=text found in %FILE%"
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
