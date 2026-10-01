@echo off
setlocal
rem ====================================================================================
rem  26_file_hash.bat - a file has the expected SHA-256 hash
rem  Usage : 26_file_hash.bat file [expected hash]   (without a hash it only prints the hash)
rem  Result: prints PASS or FAIL, exits with 0 (PASS) or 1 (FAIL) and appends one line
rem          "date time [26_file_hash] PASS|FAIL message" to the result file.
rem  Process Launcher, expected result:
rem      :File:::[26_file_hash] PASS     (looks in the result file, only at what this run wrote)
rem      PASS                      (plain text: looks in the console output)
rem ====================================================================================

rem The result file chosen in Process Launcher (variable PCR_RESULT_FILE); when the batch file is
rem started by hand, result.txt next to it.
if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"
set "RESULT_FILE=%PCR_RESULT_FILE%"
set "TEST_NAME=%~n0"

rem ---------------------------------- the test -----------------------------------------
set "FILE=%~1"
set "EXPECTED=%~2"
if not defined FILE set "FILE=%~f0"

if not exist "%FILE%" (
    set "MSG=file not found: %FILE%"
    goto :fail
)
rem certutil prints the hash on its second line.
set "HASH="
for /f "skip=1 delims=" %%h in ('certutil -hashfile "%FILE%" SHA256') do if not defined HASH set "HASH=%%h"
set "HASH=%HASH: =%"

if not defined EXPECTED (
    set "MSG=SHA-256 of %FILE% is %HASH%"
    goto :pass
)
if /i not "%HASH%"=="%EXPECTED%" (
    set "MSG=hash is %HASH%, expected %EXPECTED%"
    goto :fail
)
set "MSG=hash of %FILE% is correct"
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
