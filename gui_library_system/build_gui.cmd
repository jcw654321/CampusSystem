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

set "BANK_EASYX_LIB=..\third_party\easyx\lib\x64\EasyXw.lib"
if not exist "%BANK_EASYX_LIB%" set "BANK_EASYX_LIB=D:\c++\VC\Auxiliary\VS\lib\x64\EasyXw.lib"

cl /nologo /EHsc /std:c++17 /utf-8 /Zi /W3 /MDd /wd4828 ^
   /D_UNICODE /DUNICODE ^
   /I"..\library_manage_system\include" ^
   /I"..\include" ^
   /I"..\gui_student_system\include" ^
   /I"..\student_manage_system\include" ^
   /I"%BANK_EASYX_INCLUDE%" ^
   /I"..\bank_manage_system\include" ^
   /I"..\gui_bank_system" ^
   gui_main.cpp ^
   ..\gui_bank_system\gui_bank.cpp ^
   ..\bank_manage_system\src\BankAccount.cpp ^
   ..\bank_manage_system\src\SavingAccount.cpp ^
   ..\bank_manage_system\src\CreditAccount.cpp ^
   ..\bank_manage_system\src\Banksystem.cpp ^
   ..\bank_manage_system\src\BanksystemFile.cpp ^
   ..\library_manage_system\src\book.cpp ^
   ..\library_manage_system\src\dateutils.cpp ^
   ..\library_manage_system\src\loanrecord.cpp ^
   ..\library_manage_system\src\library_system.cpp ^
   ..\gui_student_system\src\student_manager_gui.cpp ^
   ..\gui_student_system\src\student_gui.cpp ^
   ..\gui_student_system\src\course_manager_gui.cpp ^
   ..\student_manage_system\src\student.cpp ^
   ..\student_manage_system\src\student_manager.cpp ^
   ..\student_manage_system\src\subject.cpp ^
   ..\student_manage_system\src\subject_manager.cpp ^
   ..\student_manage_system\src\fileoperate.cpp ^
   ..\src\user.cpp ^
   ..\src\user_manager.cpp ^
   /Fo"build\\" /Fe"build\gui.exe" ^
   /link "%BANK_EASYX_LIB%" user32.lib gdi32.lib winmm.lib shell32.lib

if errorlevel 1 (
    echo.
    echo [FAILED] compile or link error
    exit /b 1
)

echo.
echo [OK] output: %CD%\build\gui.exe
exit /b 0
