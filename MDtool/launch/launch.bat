:: C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/MDtool/MDtool.exe update_md80 -f C:/Users/klonyyy/PROJECTS/MAB/projects/CANdle-SDK/CANdle-SDK/build/MDtool/md80.mab
:: C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/MDtool/MDtool.exe -v update_bootloader -f C:/Users/klonyyy/PROJECTS/MAB/projects/CANdle-SDK/CANdle-SDK/build/MDtool/boot_50f348.mab
:: C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/MDtool/MDtool.exe --id 1 readSDO --idx 0x2006 --subidx 0
:: C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/MDtool/MDtool.exe --id 2 writeSDO 0x2000 0x06 AAAAAAAAAAA str
:: C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/MDtool/MDtool.exe --id 2 writeSDO 0x2000 0x01 5 u8
:: C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/MDtool/MDtool.exe --id 2 writeSDO 0x6075 0x00 10000 u32
:: C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/Examples/Examples.exe

C:\\msys64\\mingw64\\bin\\gdbserver.exe localhost:2000 ./build/MDtool/MDtool.exe -i 2 status