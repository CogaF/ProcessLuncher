@echo off
setlocal
rem ====================================================================================
rem  18_copy_and_verify.bat - a copy is identical to its source
rem  Usage : 18_copy_and_verify.bat [source] [target folder]   (default: this batch file into the temp folder)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [18_copy_and_verify] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[18_copy_and_verify] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "SOURCE=%~1"
set "TARGET=%~2"
if not defined SOURCE set "SOURCE=%~f0"
if not defined TARGET set "TARGET=%TEMP%\pcr_copy_test"

if not exist "%SOURCE%" (
    set "MSG=source not found: %SOURCE%"
    goto :fail
)
if not exist "%TARGET%\" mkdir "%TARGET%"
copy /y "%SOURCE%" "%TARGET%\" >nul
if errorlevel 1 (
    set "MSG=copy to %TARGET% failed"
    goto :fail
)
rem The copy keeps the file name of the source.
for %%f in ("%SOURCE%") do set "NAME=%%~nxf"
fc /b "%SOURCE%" "%TARGET%\%NAME%" >nul 2>&1
if errorlevel 1 (
    set "MSG=the copy differs from %SOURCE%"
    goto :fail
)
set "MSG=copy verified in %TARGET%"
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
