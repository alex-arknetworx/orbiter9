# BIN_ROM

nuvoton9 takes a .bin file and flashes it via the 'in circuit programming' ICP interface of Nuvoton MCUs to APROM.

The tool is designed to run on a Raspberry Pi and uses GPIO 0, 1 and 5 to emulate an ICP interface (DATA, CLOCK, RESET). The source code makes use of the built in RPi GPIO driver in 9Front.
Running the tool without a .bin file reference will return the connected MCU Device ID information.

sblink.bin and fblink.bin have been included for testing. Both .bin files blink an LED connected to P2.1 either slow or fast.

Build:

mk

mk install

Usage:

nuvoton9 sblink.bin
