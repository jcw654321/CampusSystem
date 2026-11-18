@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build

call "D:\c++\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] failed to run vcvars64.bat
    exit /b 1
)

cl /nologo /EHsc /std:c++17 /utf-8 /Zi /W3 /MDd /wd4828 ^
   /D_UNICODE /DUNICODE ^
   /I"..\library_manage_system\include" ^
   /I"D:\c++\VC\Auxiliary\VS\include" ^
   gui_main.cpp ^
   ..\library_manage_system\src\book.cpp ^
   ..\library_manage_system\src\dateutils.cpp ^
   ..\library_manage_system\src\loanrecord.cpp ^
   ..\library_manage_system\src\library_system.cpp ^
   /Fo"build\\" /Fe"build\gui.exe" ^
   /link "D:\c++\VC\Auxiliary\VS\lib\x64\EasyXw.lib" user32.lib gdi32.lib winmm.lib shell32.lib

if errorlevel 1 (
    echo.
    echo [FAILED] compile or link error
    exit /b 1
)

echo.
echo [OK] output: %CD%\build\gui.exe
exit /b 0
