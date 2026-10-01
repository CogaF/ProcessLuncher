@echo off
setlocal
rem ====================================================================================
rem  15_timeout_guard.bat - a command ends within a time limit (it is killed and reported FAIL otherwise)
rem  Usage : 15_timeout_guard.bat [seconds] [command...]   (default: 10 s, cmd /c exit 0) - uses PowerShell
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [15_timeout_guard] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[15_timeout_guard] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "LIMIT=%~1"
if not defined LIMIT set "LIMIT=10"
set "TESTCMD=cmd /c exit 0"
set "REST="
for /f "tokens=1,* delims= " %%a in ("%*") do set "REST=%%b"
if defined REST set "TESTCMD=%REST%"

rem PowerShell starts the command, waits LIMIT seconds and kills it if it is still running (exit 99).
rem The command and the limit travel in environment variables: no quoting problems.
powershell -NoProfile -Command "$p = Start-Process cmd -ArgumentList '/c', $env:TESTCMD -PassThru -WindowStyle Hidden; if ($p.WaitForExit([int]$env:LIMIT * 1000)) { exit $p.ExitCode } else { $p.Kill(); exit 99 }" >nul 2>&1
set "CODE=%errorlevel%"
if "%CODE%"=="99" (
    set "MSG=%TESTCMD% did not end within %LIMIT% s"
    goto :fail
)
if not "%CODE%"=="0" (
    set "MSG=%TESTCMD% ended with code %CODE%"
    goto :fail
)
set "MSG=%TESTCMD% ended in time"
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
