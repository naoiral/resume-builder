@echo off
REM Build ResumeBuilder for Windows
REM Requires: Visual Studio 2022 or CMake + MSVC

echo ========================================
echo Building ResumeBuilder for Windows
echo ========================================

cd /d "%~dp0\.."

REM Create build directory
if not exist build_windows mkdir build_windows
cd build_windows

REM Configure with CMake
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config Release

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build successful!
echo Output: build_windows\Release\ResumeBuilder.exe
echo ========================================
pause
