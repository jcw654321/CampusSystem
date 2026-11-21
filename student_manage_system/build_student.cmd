@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build

call "D:\Visual Studio\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] failed to run vcvars64.bat
    exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /Zi /W3 /MDd ^
   /I. /Iinclude /I..\include ^
   src\main.cpp src\student.cpp src\student_manager.cpp ^
   src\subject.cpp src\subject_manager.cpp src\fileoperate.cpp ^
   ..\src\user.cpp ..\src\user_manager.cpp ^
   /Fo"build\\" /Fd"build\vc140.pdb" /Fe"build\main.exe"

if errorlevel 1 (
    echo.
    echo [FAILED] compile or link error
    exit /b 1
)

echo.
echo [OK] output: %CD%\build\main.exe
exit /b 0
