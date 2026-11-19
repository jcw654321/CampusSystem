@echo off
setlocal
cd /d "%~dp0"

REM Build the entire bank console project, independent of shared configuration.
set "BANK_VSROOT="
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "BANK_VSROOT=%%i"
)
if not defined BANK_VSROOT if exist "D:\c++\VC\Auxiliary\Build\vcvars64.bat" set "BANK_VSROOT=D:\c++"
if not defined BANK_VSROOT if exist "D:\Visual Studio\VC\Auxiliary\Build\vcvars64.bat" set "BANK_VSROOT=D:\Visual Studio"
if not defined BANK_VSROOT (
    echo [ERROR] Visual Studio C++ tools not found.
    exit /b 1
)
call "%BANK_VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not exist build mkdir build
cl /nologo /EHsc /std:c++17 /utf-8 /Zi /W3 /MDd ^
    /I"%~dp0include" "%~dp0src\*.cpp" ^
    /Fo"%~dp0build\\" /Fd"%~dp0build\vc140.pdb" /Fe"%~dp0build\main.exe"
if errorlevel 1 (
    echo [FAILED] Bank console build failed.
    exit /b 1
)
echo [OK] %~dp0build\main.exe
exit /b 0
