@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build

call "D:\Visual Studio\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] failed to run vcvars64.bat
    exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /Zi /W3 /MDd /wd4828 ^
   /D_UNICODE /DUNICODE ^
   /I"..\student_manage_system\include" ^
   /I"..\include" ^
   /I"include" ^
   src\gui_test.cpp ^
   src\student_manager_gui.cpp ^
   src\student_gui.cpp ^
   src\course_manager_gui.cpp ^
   ..\student_manage_system\src\student.cpp ^
   ..\student_manage_system\src\student_manager.cpp ^
   ..\student_manage_system\src\subject.cpp ^
   ..\student_manage_system\src\subject_manager.cpp ^
   ..\student_manage_system\src\fileoperate.cpp ^
   ..\src\user.cpp ^
   ..\src\user_manager.cpp ^
   /Fo"build\\" /Fd"build\vc140.pdb" /Fe"build\gui.exe" ^
   /link "D:\Visual Studio\VC\Auxiliary\VS\lib\x64\EasyXw.lib" user32.lib gdi32.lib winmm.lib shell32.lib

if errorlevel 1 (
    echo.
    echo [FAILED] compile or link error
    exit /b 1
)

echo.
echo [OK] output: %CD%\build\gui.exe
exit /b 0
