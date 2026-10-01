@echo off
setlocal
rem ====================================================================================
rem  25_no_errors_in_logs.bat - no log file of a folder contains a word such as ERROR
rem  Usage : 25_no_errors_in_logs.bat [folder] [word]   (default: the temp folder, ERROR)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [25_no_errors_in_logs] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[25_no_errors_in_logs] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FOLDER=%~1"
set "WORD=%~2"
if not defined FOLDER set "FOLDER=%TEMP%"
if not defined WORD set "WORD=ERROR"

if not exist "%FOLDER%\" (
    set "MSG=folder not found: %FOLDER%"
    goto :fail
)
set /a BAD=0
set /a CHECKED=0
rem for %%f in (set) do ...  takes every *.log of the folder (in a batch file the variable has two %%).
for %%f in ("%FOLDER%\*.log") do (
    set /a CHECKED+=1
    findstr /i /c:"%WORD%" "%%f" >nul 2>&1 && set /a BAD+=1
)
if %BAD% gtr 0 (
    set "MSG=%BAD% of %CHECKED% log files contain %WORD%"
    goto :fail
)
set "MSG=%CHECKED% log files checked, none contains %WORD%"
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
