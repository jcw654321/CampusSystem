@echo off
setlocal
cd /d "%~dp0"
if not defined VSCMD_VER (
    if exist "D:\Visual Studio\VC\Auxiliary\Build\vcvars64.bat" (
        call "D:\Visual Studio\VC\Auxiliary\Build\vcvars64.bat" >nul
    ) else (
        for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do call "%%i\VC\Auxiliary\Build\vcvars64.bat" >nul
    )
)
where cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSVC C++ tools not found. Install Desktop development with C++.
    exit /b 1
)
REM EASYX_ROOT may contain include\graphics.h and lib\x64\EasyXw.lib.
REM When EasyX is installed directly into MSVC, default include/lib paths work.
set "EXTRA="
set "EASYXLIB=EasyXw.lib"
if not defined EASYX_ROOT if exist "..\third_party\easyx\include\graphics.h" set "EASYX_ROOT=%~dp0..\third_party\easyx"
if defined EASYX_ROOT (
    set EXTRA=/I"%EASYX_ROOT%\include"
    set "EASYXLIB=%EASYX_ROOT%\lib\x64\EasyXw.lib"
) else if exist "D:\Visual Studio\VC\Auxiliary\VS\include\graphics.h" (
    set EXTRA=/I"D:\Visual Studio\VC\Auxiliary\VS\include"
    set "EASYXLIB=D:\Visual Studio\VC\Auxiliary\VS\lib\x64\EasyXw.lib"
)
if not exist build mkdir build
cl /nologo /EHsc /std:c++17 /utf-8 /W4 /MD /D_UNICODE /DUNICODE %EXTRA% ^
    /I"..\bank_manage_system\include" gui_main.cpp ^
    ..\bank_manage_system\src\BankAccount.cpp ^
    ..\bank_manage_system\src\SavingAccount.cpp ^
    ..\bank_manage_system\src\CreditAccount.cpp ^
    ..\bank_manage_system\src\Banksystem.cpp ^
    ..\bank_manage_system\src\BanksystemFile.cpp ^
    /Fo"build\\" /Fe"build\gui.exe" ^
    /link "%EASYXLIB%" user32.lib gdi32.lib winmm.lib shell32.lib
if errorlevel 1 (
    echo [FAILED] Check the errors above. EasyX Unicode x64 library is required.
    exit /b 1
)
echo [OK] %CD%\build\gui.exe
exit /b 0
