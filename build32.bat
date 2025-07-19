@echo off
:: Correct MinGW 32-bit path
set MINGW32_PATH=C:/msys64/mingw32
set PATH=%MINGW32_PATH%/bin;%PATH%

:: Clean previous build
rmdir /s /q build32
mkdir build32
cd build32

echo Running CMake with forced MinGW 32-bit toolchain...
cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_C_COMPILER=%MINGW32_PATH%/bin/gcc.exe ^
  -DCMAKE_CXX_COMPILER=%MINGW32_PATH%/bin/g++.exe ^
  -DFORCE_32BIT_BIN=ON

if errorlevel 1 (
  echo CMake configuration failed
  exit /b 1
)

echo.
echo Building...
mingw32-make -j4


copy src\request04rel32.dll ..\..\..\vcmp\Rizwan\plugins
if errorlevel 1 (
    echo Failed to copy DLL
    exit /b 1
)

cd ..
