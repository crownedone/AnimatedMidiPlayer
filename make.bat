@echo off

cd /d "%~dp0"
mkdir build
cd build
cmake.exe .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
myproject.sln
cd /d "%~dp0"
pause