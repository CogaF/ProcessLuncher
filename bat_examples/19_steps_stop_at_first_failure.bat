@echo off
setlocal
rem ====================================================================================
rem  19_steps_stop_at_first_failure.bat - several checks in one batch file: the first failure stops it
rem  Usage : 19_steps_stop_at_first_failure.bat
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [19_steps_stop_at_first_failure] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[19_steps_stop_at_first_failure] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
rem Every step is "call :step <name> <command...>". The step result is logged as INFO (no PASS / FAIL
rem word, so it cannot be mistaken for the final result); the batch file ends at the first failing step.
set "STEPS_DONE=0"

call :step "computer name"   hostname                       || goto :stepfailed
call :step "cmd.exe found"   where cmd                      || goto :stepfailed
call :step "loopback ping"   ping -n 1 127.0.0.1            || goto :stepfailed

set "MSG=all %STEPS_DONE% steps done"
goto :pass

:stepfailed
set "MSG=step '%STEP%' failed after %STEPS_DONE% good steps"
goto :fail

:step
set "STEP=%~1"
shift
%1 %2 %3 %4 %5 %6 %7 %8 %9 >nul 2>&1
if errorlevel 1 (
    call :log INFO step "%STEP%" failed
    exit /b 1
)
call :log INFO step "%STEP%" ok
set /a STEPS_DONE+=1
exit /b 0

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
