@rem Update CMake (we just delete the old CMake files and copy the new ones)

@echo off

cmake --version
cd "C:\Program Files (x86)"
rmdir "CMake" /s /q 
set CMAKE_URL="https://cmake.org/files/v3.17/cmake-3.17.2-win32-x86.zip"
appveyor DownloadFile %CMAKE_URL% -FileName cmake.zip
7z x cmake.zip -o. > nul
move cmake-3.17.2-win32-x86 CMake
del cmake.zip /q /f
cmake --version
