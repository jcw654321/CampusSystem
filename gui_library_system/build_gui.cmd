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
   /I"..\library_manage_system\include" ^
   /I"..\include" ^
   /I"..\gui_student_system\include" ^
   /I"..\student_manage_system\include" ^
   /I"D:\Visual Studio\VC\Auxiliary\VS\include" ^
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
   /Fo"build\\" /Fe"build\gui.exe" ^
   /link "D:\Visual Studio\VC\Auxiliary\VS\lib\x64\EasyXw.lib" user32.lib gdi32.lib winmm.lib shell32.lib

if errorlevel 1 (
    echo.
    echo [FAILED] compile or link error
    exit /b 1
)

echo.
echo [OK] output: %CD%\build\gui.exe
exit /b 0
