@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build

call "D:\c++\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] failed to run vcvars64.bat
    exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /W4 /MDd /D_UNICODE /DUNICODE ^
    /I"..\bank_manage_system\include" ^
    /I"D:\c++\VC\Auxiliary\VS\include" ^
    gui_bank.cpp ^
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
