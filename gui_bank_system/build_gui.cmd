@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build

set "VSROOT="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
)
if not defined VSROOT if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set "VSROOT=%ProgramFiles%\Microsoft Visual Studio\2022\Community"
if not defined VSROOT (
    echo [ERROR] Visual Studio C++ tools were not found.
    exit /b 1
)

call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSVC x64 build environment is unavailable.
    exit /b 1
)

set "BANK_EASYX_INCLUDE=..\third_party\easyx\include"
if not exist "%BANK_EASYX_INCLUDE%\graphics.h" (
    echo [ERROR] EasyX headers were not found.
    exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /W4 /MDd /D_UNICODE /DUNICODE ^
    /I"..\bank_manage_system\include" ^
    /I"..\include" ^
    /I"%BANK_EASYX_INCLUDE%" ^
    gui_bank.cpp ^
    gui_bank_admin.cpp ^
    gui_bank_student.cpp ^
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
    "build\gui_bank_admin.obj" ^
    "build\gui_bank_student.obj" ^
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
