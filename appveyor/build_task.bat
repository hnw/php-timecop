@echo off

if "%APPVEYOR%" equ "True" rmdir /s /q C:\cygwin >NUL 2>NUL
if %errorlevel% neq 0 exit /b 3
if "%APPVEYOR%" equ "True" rmdir /s /q C:\cygwin64 >NUL 2>NUL
if %errorlevel% neq 0 exit /b 3
if "%APPVEYOR%" equ "True" rmdir /s /q C:\mingw >NUL 2>NUL
if %errorlevel% neq 0 exit /b 3
if "%APPVEYOR%" equ "True" rmdir /s /q C:\mingw-w64 >NUL 2>NUL
if %errorlevel% neq 0 exit /b 3
if "%APPVEYOR%" equ "True" rmdir /s /q C:\msys64 >NUL 2>NUL
if %errorlevel% neq 0 exit /b 3

cd /D %APPVEYOR_BUILD_FOLDER%
if %errorlevel% neq 0 exit /b 3

set DEPS_DIR=%PHP_SDK_DIR%\deps-%BRANCH%-%PHP_SDK_VC%-%PHP_SDK_ARCH%
echo Downloading dependencies in %DEPS_DIR%
cmd /c phpsdk_deps -u --deps %DEPS_DIR%
if %errorlevel% neq 0 exit /b 3

cmd /c buildconf.bat
if %errorlevel% neq 0 exit /b 3

if "%THREAD_SAFE%" equ "0" set ADD_CONF=--disable-zts

cmd /c configure.bat ^
        --disable-all ^
        --enable-phar ^
        --enable-json ^
        --enable-hash ^
        --enable-ctype ^
        --enable-filter ^
        --enable-tokenizer ^
        --with-iconv ^
        --with-openssl ^
        --with-dom ^
        --with-libxml ^
        --enable-zip ^
        --enable-timecop=shared ^
        --enable-cli ^
        --enable-zts ^
        --with-config-file-scan-dir=%PROJECT_DIR%\bin\modules.d ^
        --with-prefix=%PROJECT_DIR%\bin ^
        --with-php-build=%DEPS_DIR% ^
	%ADD_CONF%
if %errorlevel% neq 0 exit /b 3

nmake /NOLOGO
if %errorlevel% neq 0 exit /b 3

exit /b 0

