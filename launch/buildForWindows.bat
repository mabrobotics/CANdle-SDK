@echo off
if not exist C:\w64devkit\bin\gcc.exe (
    echo Downloading w64devkit...
    curl -OL https://github.com/skeeto/w64devkit/releases/download/v1.22.0/w64devkit-1.22.0.zip
    echo Extracting w64devkit to C:\w64devkit...
    tar -xvf w64devkit-1.22.0.zip -C C:\
    del w64devkit-1.22.0.zip /Q
    echo Done. w64devkit ready for use.
    echo Rerun the script to build.
    exit /b 0
)
echo w64devkit found!
path|find /i "w64devkit"    >nul || set path=%path%;C:\w64devkit\bin
echo Build files will be stored in %0\..\..\build
mkdir %0\..\..\build
set currentdir="%cd%"
cd %0\..\..\build
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make package -j4
cd %currentdir%
exit /b 0

