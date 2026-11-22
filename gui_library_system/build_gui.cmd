@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build

set "VSROOT="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
)
if not defined VSROOT if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set "VSROOT=C:\Program Files\Microsoft Visual Studio\2022\Community"
if not defined VSROOT if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" set "VSROOT=C:\Program Files\Microsoft Visual Studio\2022\Professional"
if not defined VSROOT if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" set "VSROOT=C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
if not defined VSROOT if exist "D:\Visual Studio\VC\Auxiliary\Build\vcvars64.bat" set "VSROOT=D:\Visual Studio"
if not defined VSROOT (
    echo [ERROR] Visual Studio C++ tools were not found.
    exit /b 1
)

call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] failed to run vcvars64.bat
    exit /b 1
)

set "EASYX_INCLUDE=%VSROOT%\VC\Auxiliary\VS\include"
set "EASYX_LIB=%VSROOT%\VC\Auxiliary\VS\lib\x64\EasyXw.lib"
if not exist "%EASYX_INCLUDE%\graphics.h" (
    echo [ERROR] EasyX headers were not found.
    exit /b 1
)
if not exist "%EASYX_LIB%" (
    echo [ERROR] EasyX x64 library was not found.
    exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /Zi /W3 /MDd /wd4828 ^
   /D_UNICODE /DUNICODE ^
   /I"..\library_manage_system\include" ^
   /I"..\include" ^
   /I"..\gui_student_system\include" ^
   /I"..\student_manage_system\include" ^
   /I"..\gui_bank_system" ^
   /I"..\bank_manage_system\include" ^
   /I"%EASYX_INCLUDE%" ^
   gui_main.cpp ^
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
   ..\gui_bank_system\gui_bank.cpp ^
   ..\gui_bank_system\gui_bank_admin.cpp ^
   ..\gui_bank_system\gui_bank_student.cpp ^
   ..\bank_manage_system\src\BankAccount.cpp ^
   ..\bank_manage_system\src\SavingAccount.cpp ^
   ..\bank_manage_system\src\CreditAccount.cpp ^
   ..\bank_manage_system\src\Banksystem.cpp ^
   ..\bank_manage_system\src\BanksystemFile.cpp ^
   /Fo"build\\" /Fe"build\gui.exe" ^
   /link "%EASYX_LIB%" user32.lib gdi32.lib winmm.lib shell32.lib
if errorlevel 1 (
    echo.
    echo [FAILED] compile or link error
    exit /b 1
)

echo.
echo [OK] output: %CD%\build\gui.exe
exit /b 0
