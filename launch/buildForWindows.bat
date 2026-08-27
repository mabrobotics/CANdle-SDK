@echo off
setlocal EnableDelayedExpansion
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
set "NSIS_DIR=C:\Program Files (x86)\NSIS"
if not exist "!NSIS_DIR!\makensis.exe" (
    if not exist "C:\NSIS\makensis.exe" (
        echo NSIS not found. Downloading NSIS installer...
        curl -L -o nsis-setup.exe "https://sourceforge.net/projects/nsis/files/NSIS%203/3.10/nsis-3.10-setup.exe/download"
        echo Installing NSIS silently...
        nsis-setup.exe /S
        del nsis-setup.exe /Q
        echo NSIS installed successfully.
    ) else (
        set "NSIS_DIR=C:\NSIS"
    )
)
echo w64devkit found!
path|find /i "w64devkit"    >nul || set path=%path%;C:\w64devkit\bin
path|find /i "NSIS"         >nul || set "PATH=%PATH%;!NSIS_DIR!"

echo Build files will be stored in %0\..\..\build
mkdir %0\..\..\build
set currentdir="%cd%"
cd %0\..\..\build
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make package -j4
cd %currentdir%
exit /b 0

