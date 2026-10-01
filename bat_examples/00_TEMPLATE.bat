@echo off
setlocal
rem ====================================================================================
rem  00_TEMPLATE.bat - skeleton to copy: put your own test in the marked place
rem  Usage : 00_TEMPLATE.bat [arguments]
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [00_TEMPLATE] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[00_TEMPLATE] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
rem Replace this block with your test. When it worked:   set "MSG=what was checked" & goto :pass
rem When it did not:                                      set "MSG=what went wrong"  & goto :fail
rem Tips: keep the word PASS out of FAIL lines and the word FAIL out of PASS lines;
rem       quote paths ("%~1"); test the exit code of the command with "if errorlevel 1".

ping -n 1 127.0.0.1 >nul
if errorlevel 1 (
    set "MSG=ping returned an error"
    goto :fail
)
set "MSG=ping answered"
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
