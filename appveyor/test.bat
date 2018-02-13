@echo off

cmd /c %SDK_RUNNER% -t %APPVEYOR_BUILD_FOLDER%\appveyor\test_task.bat
if %errorlevel% neq 0 exit /b 3

exit /b 0

