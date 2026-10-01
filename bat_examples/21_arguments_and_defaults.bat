@echo off
setlocal
rem ====================================================================================
rem  21_arguments_and_defaults.bat - how to read arguments, give defaults and refuse a missing one
rem  Usage : 21_arguments_and_defaults.bat name [count]
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [21_arguments_and_defaults] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[21_arguments_and_defaults] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
rem %1 is the first argument, %~1 is it without quotes, %* is all of them.
set "NAME=%~1"
set "COUNT=%~2"

if not defined NAME (
    rem Wrong use: report it as FAIL with a clear text.
    set "MSG=usage: %~nx0 name [count]"
    goto :fail
)
if not defined COUNT set "COUNT=1"

rem /a does arithmetic; an argument that is not a number would end up as 0.
set /a DOUBLE=COUNT*2
set "MSG=name=%NAME% count=%COUNT% double=%DOUBLE%"
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
