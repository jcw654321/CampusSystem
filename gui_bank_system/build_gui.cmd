@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build

REM Prefer the installed Visual Studio; retain the original custom-path fallback.
set "BANK_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "BANK_VSROOT="
if exist "%BANK_VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%BANK_VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "BANK_VSROOT=%%i"
)
if defined BANK_VSROOT (
    call "%BANK_VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
) else (
    call "D:\c++\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
)
if errorlevel 1 (
    echo [ERROR] MSVC x64 build environment is unavailable.
    exit /b 1
)

set "BANK_EASYX_INCLUDE=..\third_party\easyx\include"
if not exist "%BANK_EASYX_INCLUDE%\graphics.h" set "BANK_EASYX_INCLUDE=D:\c++\VC\Auxiliary\VS\include"
if not exist "%BANK_EASYX_INCLUDE%\graphics.h" (
    echo [ERROR] EasyX headers were not found.
    exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /W4 /MDd /D_UNICODE /DUNICODE ^
    /I"..\bank_manage_system\include" ^
    /I"..\include" ^
    /I"%BANK_EASYX_INCLUDE%" ^
    gui_bank.cpp ^
    ..\src\user.cpp ^
    ..\bank_manage_system\src\BankAccount.cpp ^
    ..\bank_manage_system\src\SavingAccount.cpp ^
    ..\bank_manage_system\src\CreditAccount.cpp ^
    ..\bank_manage_system\src\Banksystem.cpp ^
    ..\bank_manage_system\src\BanksystemFile.cpp ^
    /c /Fo"build\\"
if errorlevel 1 (
    echo [FAILED] C++ compilation failed.
    exit /b 1
)

lib /nologo /OUT:"build\gui_bank.lib" ^
    "build\gui_bank.obj" ^
    "build\user.obj" ^
    "build\BankAccount.obj" ^
    "build\SavingAccount.obj" ^
    "build\CreditAccount.obj" ^
    "build\Banksystem.obj" ^
    "build\BanksystemFile.obj"
if errorlevel 1 (
    echo [FAILED] Static library creation failed.
    exit /b 1
)

echo [OK] %CD%\build\gui_bank.lib
exit /b 0
