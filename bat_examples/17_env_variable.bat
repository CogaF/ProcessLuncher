@echo off
setlocal
rem ====================================================================================
rem  17_env_variable.bat - an environment variable is defined (and has a value)
rem  Usage : 17_env_variable.bat [name] [value]   (default: PATH is defined)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [17_env_variable] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[17_env_variable] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "VARNAME=%~1"
set "WANTED=%~2"
if not defined VARNAME set "VARNAME=PATH"

rem "defined" takes the name without percent signs.
if not defined %VARNAME% (
    set "MSG=variable %VARNAME% is not defined"
    goto :fail
)
if not defined WANTED goto :definedok

rem "call set" expands %%NAME%% twice: first to %PATH%, then to its value. It is done outside any
rem ( ) block on purpose: inside a block every %VAR% is expanded before the block runs.
call set "ACTUAL=%%%VARNAME%%%"
if /i not "%ACTUAL%"=="%WANTED%" (
    set "MSG=variable %VARNAME% is not %WANTED%"
    goto :fail
)

:definedok
set "MSG=variable %VARNAME% is defined"
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
