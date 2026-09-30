@echo off
setlocal
rem ====================================================================================
rem  32_setting_in_config_file.bat - a key=value line of a config file has the wanted value
rem  Usage : 32_setting_in_config_file.bat [file] [key] [value]   (no arguments: creates a test file)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [32_setting_in_config_file] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[32_setting_in_config_file] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FILE=%~1"
set "KEY=%~2"
set "VALUE=%~3"
if defined VALUE goto :check

rem No arguments: make a small config file and check a value in it.
set "FILE=%TEMP%\pcr_config_test.ini"
set "KEY=mode"
set "VALUE=auto"
>"%FILE%" echo mode=auto
>>"%FILE%" echo retries=3

:check
if not exist "%FILE%" (
    set "MSG=config file not found: %FILE%"
    goto :fail
)
rem /b = the line must start with the key, /c: = literal text.
findstr /b /i /c:"%KEY%=%VALUE%" "%FILE%" >nul
if errorlevel 1 (
    set "MSG=%KEY% is not %VALUE% in %FILE%"
    goto :fail
)
set "MSG=%KEY%=%VALUE% in %FILE%"
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
