@echo off
setlocal
rem ====================================================================================
rem  11_http_status.bat - a web address answers with HTTP status 200
rem  Usage : 11_http_status.bat [url] [status]   (default: https://example.com 200) - needs curl (Windows 10+)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [11_http_status] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[11_http_status] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "URL=%~1"
set "WANTED=%~2"
if not defined URL set "URL=https://example.com"
if not defined WANTED set "WANTED=200"

where curl >nul 2>&1
if errorlevel 1 (
    set "MSG=curl is not installed"
    goto :fail
)
set "CODE=000"
for /f %%c in ('curl -s -o nul -m 15 -w "%%{http_code}" "%URL%"') do set "CODE=%%c"
if not "%CODE%"=="%WANTED%" (
    set "MSG=%URL% answered %CODE%, expected %WANTED%"
    goto :fail
)
set "MSG=%URL% answered %CODE%"
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
