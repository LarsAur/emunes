SETLOCAL
cd ./src
gcc -O3 -c window.c logger.c ./nes/cpu.c ./nes/loader.c ./nes/ppu.c ./nes/controller.c
windres -i menu.rc -o menu.o
gcc -o emunes.exe window.o logger.o cpu.o loader.o ppu.o controller.o menu.o -s -lcomctl32 -Wl,--subsystem,windows -lgdi32 -lWinmm -lComdlg32
DEL *.o
echo Starting...
START emunes.exe