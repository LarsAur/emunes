# Emunes
NES emulator written in C using win32api.
Information regarding the NES architecture is from the [NESdev Wiki](https://www.nesdev.org/wiki/Nesdev_Wiki).

## Features / limitations
* Only includes the legal instructions of the RP2A07 / RP2A03 cpu, which makes many roms not work
* Sound is not implemented
* Only Mapper0 is implemented, which limits the games which can be played (see [here](https://nesdir.github.io/mapper0.html) for a list of games)
* Only one controller on port 1

## Instructions
* Compile the emulator with compile.bat (requires gcc)
* Start emunes.exe and click `file > open` to load your rom
* Controls using the arrow keys as D-Pad and A = X, B = Z, START = ENTER and SELECT = RSHIFT (These can be changed in controller.h)
* In the options you can toggle some debug info, this includes the current cycle and instruction
* The emulator logs to emunes.log, and the `CURRENT_LOG_LEVEL` can be set in logger.h (default is LL_INFO)

## Tested roms
* ARKANOID
* Donkey Kong (World) (Rev A)
* Super mario bros (Not working)
* Nes Test by kevtris (https://www.nesdev.org/wiki/Emulator_tests)

### Nes test 
![Test passing](https://user-images.githubusercontent.com/3136092/184536073-f0599696-6a67-4080-9a46-21f76344a8b9.png)

### Mario bros not working
![Mario not working](https://user-images.githubusercontent.com/3136092/184536228-ba74e595-3305-40f9-ad80-b1fa99156f42.png)
