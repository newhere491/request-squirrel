@echo off
set MINGW64_PATH=C:/msys64/mingw64
set PATH=%MINGW64_PATH%/bin;%PATH%

:: Remove and recreate build64
rmdir /s /q build64
mkdir build64
cd build64

echo Running CMake with forced MinGW 64-bit toolchain...
cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_C_COMPILER=%MINGW64_PATH%/bin/gcc.exe ^
  -DCMAKE_CXX_COMPILER=%MINGW64_PATH%/bin/g++.exe

if errorlevel 1 (
  echo CMake configuration failed
  exit /b 1
)

mingw32-make -j4


copy src\request04rel64.dll ..\..\..\vcmp\Rizwan\plugins
if errorlevel 1 (
    echo Failed to copy DLL
    exit /b 1
)

cd ..
