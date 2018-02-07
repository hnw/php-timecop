rem @echo off

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

for /f %%F in ("%PHP_SRC_URL%") do set PHP_SRC_FILENAME=%%~nxF
set PHP_SRC_DIRNAME=%PHP_SRC_FILENAME:.tar.bz2=%
set PHP_BUILD_ROOT=%PHP_SDK_DIR%\phpdev\%PHP_BUILD_CRT%\%PLATFORM%\%PHP_SRC_DIRNAME%

cd %PHP_SDK_DIR%
if %errorlevel% neq 0 exit /b 3

if exist %PHP_SDK_TOOLS_DIRNAME% ( cd %PHP_SDK_TOOLS_DIRNAME% )
if %errorlevel% neq 0 exit /b 3

cmd /c phpsdk_buildtree.bat phpdev
if %errorlevel% neq 0 exit /b 3

wget %PHP_SRC_URL%
if %errorlevel% neq 0 exit /b 3

mkdir %PHP_BUILD_ROOT%
if %errorlevel% neq 0 exit /b 3

7z x -so %PHP_SRC_FILENAME% | 7z x -si -ttar -o%PHP_BUILD_ROOT%\..
if %errorlevel% neq 0 exit /b 3

set DEPS_DIR=%PHP_SDK_DIR%\deps-%PHP_SRC_DIRNAME%-%PHP_SDK_VC%-%PHP_SDK_ARCH%
echo Downloading dependencies in %DEPS_DIR%

cd %PHP_BUILD_ROOT%
if %errorlevel% neq 0 exit /b 3

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

