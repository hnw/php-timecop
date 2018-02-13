@echo off

set NO_INTERACTION=1
set REPORT_EXIT_STATUS=1
set SKIP_IO_CAPTURE_TESTS=1
set TEST_PHP_EXECUTABLE=%APPVEYOR_BUILD_FOLDER%\bin\php.exe

if not exist "%TEST_PHP_EXECUTABLE%" (
    echo "%TEST_PHP_EXECUTABLE%" doesn't exist
    exit /b 3
)

"%TEST_PHP_EXECUTABLE%" %APPVEYOR_BUILD_FOLDER%\run-tests.php --show-diff
if %errorlevel% neq 0 exit /b 3

exit /b %EXIT_CODE%

