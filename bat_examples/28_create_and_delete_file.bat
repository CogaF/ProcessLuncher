@echo off
setlocal
rem ====================================================================================
rem  28_create_and_delete_file.bat - create a file, check it, delete it, check it is gone
rem  Usage : 28_create_and_delete_file.bat
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [28_create_and_delete_file] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[28_create_and_delete_file] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FILE=%TEMP%\pcr_create_delete.tmp"

echo test> "%FILE%"
if not exist "%FILE%" (
    set "MSG=could not create %FILE%"
    goto :fail
)
del /q "%FILE%"
rem The exit code of "del" says little: check that the file is really gone.
if exist "%FILE%" (
    set "MSG=could not delete %FILE%"
    goto :fail
)
set "MSG=file created and deleted"
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
