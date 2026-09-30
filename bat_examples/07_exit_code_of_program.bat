@echo off
setlocal
rem ====================================================================================
rem  07_exit_code_of_program.bat - a program ends with exit code 0
rem  Usage : 07_exit_code_of_program.bat program [arguments]   (default: cmd /c exit 0)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [07_exit_code_of_program] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[07_exit_code_of_program] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
rem Runs whatever is given on the command line and judges its exit code: 0 = PASS.
set "PROGRAM=%*"
if not defined PROGRAM set "PROGRAM=cmd /c exit 0"

%PROGRAM% >nul 2>&1
set "CODE=%errorlevel%"
if not "%CODE%"=="0" (
    set "MSG=%PROGRAM% ended with code %CODE%"
    goto :fail
)
set "MSG=%PROGRAM% ended with code 0"
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
