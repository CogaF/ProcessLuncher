@echo off
setlocal
rem ====================================================================================
rem  30_run_another_batch.bat - run another batch file and judge its exit code
rem  Usage : 30_run_another_batch.bat batchfile [arguments]
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [30_run_another_batch] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[30_run_another_batch] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "OTHER=%~1"
if not defined OTHER (
    set "MSG=usage: %~nx0 batchfile [arguments]"
    goto :fail
)
if not exist "%OTHER%" (
    set "MSG=batch file not found: %OTHER%"
    goto :fail
)

rem "call" is needed: without it this batch file would end when the other one ends.
call %*
set "CODE=%errorlevel%"
if not "%CODE%"=="0" (
    set "MSG=%OTHER% ended with code %CODE%"
    goto :fail
)
set "MSG=%OTHER% ended with code 0"
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
