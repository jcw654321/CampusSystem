@echo off
setlocal

REM ============================================================
REM  Build all .cpp files in a directory with MSVC (VS 2022).
REM  Usage: build.cmd [source-dir]
REM         no argument = the folder containing this script
REM ============================================================

set "SRC=%~1"
if "%SRC%"=="" set "SRC=%~dp0."

if not exist "%SRC%" (
    echo [ERROR] directory not found: %SRC%
    exit /b 1
)

pushd "%SRC%"

REM MSVC needs INCLUDE/LIB from vcvars64.bat, which a plain shell lacks
call "D:\c++\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] failed to run vcvars64.bat
    popd
    exit /b 1
)

set "OUT=build"
if not exist "%OUT%" mkdir "%OUT%"
set "RSP=%OUT%\sources.rsp"
if exist "%RSP%" del "%RSP%" >nul 2>&1

for /r . %%f in (*.cpp) do echo "%%f">>"%RSP%"

if not exist "%RSP%" (
    echo [ERROR] no .cpp file found in: %CD%
    popd
    exit /b 1
)

set /a N=0
for /f "usebackq delims=" %%l in ("%RSP%") do set /a N+=1
echo Building %N% source file(s) in %CD% ...

cl /nologo /EHsc /std:c++17 /utf-8 /Zi /W3 /MDd /I. /Iinclude /Fo"%OUT%\\" /Fe"%OUT%\main.exe" @"%RSP%"
if errorlevel 1 (
    echo.
    echo [FAILED] compile or link error, see messages above
    popd
    exit /b 1
)

echo.
echo [OK] output: %CD%\%OUT%\main.exe
popd
exit /b 0
