@echo off
setlocal
rem ====================================================================================
rem  06_compare_files.bat - two files are identical (and how to create test files)
rem  Usage : 06_compare_files.bat [file1 file2]   (no arguments: compares a file with its copy)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [06_compare_files] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[06_compare_files] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "A=%~1"
set "B=%~2"
if defined B goto :compare

rem No arguments: make a file and a copy in the temp folder, to show the check working.
set "A=%TEMP%\pcr_compare_a.txt"
set "B=%TEMP%\pcr_compare_b.txt"
echo sample text> "%A%"
copy /y "%A%" "%B%" >nul

:compare
if not exist "%A%" (
    set "MSG=missing %A%"
    goto :fail
)
if not exist "%B%" (
    set "MSG=missing %B%"
    goto :fail
)
rem fc exit code: 0 identical, 1 different, 2 error. /b compares byte by byte.
fc /b "%A%" "%B%" >nul
if errorlevel 1 (
    set "MSG=files are different"
    goto :fail
)
set "MSG=files are identical"
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
