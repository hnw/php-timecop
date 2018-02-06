@echo off

for /f %%F in ("%PHP_SDK_TOOLS_URL%") do set PHP_SDK_TOOLS_FILENAME=%%~nxF
set PHP_SDK_TOOLS_DIRNAME=php-sdk-binary-tools-%PHP_SDK_TOOLS_FILENAME:.zip=%

set SDK_RUNNER=%PHP_SDK_DIR%\%PHP_SDK_TOOLS_DIRNAME%\phpsdk-%PHP_BUILD_CRT%-%PLATFORM%.bat

if not exist "%PHP_SDK_DIR%" (
	echo Creating %PHP_SDK_DIR%
	mkdir "%PHP_SDK_DIR%"
)

wget %PHP_SDK_TOOLS_URL%

7z x -y %PHP_SDK_TOOLS_FILENAME% -o%PHP_SDK_DIR%

if not exist "%SDK_RUNNER%" (
	echo "%SDK_RUNNER%" doesn't exist
	exit /b 3
)

cmd /c %SDK_RUNNER% -t %APPVEYOR_BUILD_FOLDER%\appveyor\build_task.bat
if %errorlevel% neq 0 exit /b 3

exit /b 0
