# BIN_ROM

nuvoton9 takes a .bin file and flashes it via the ICP (in circuit programming) interface of Nuvoton MCUs to internal APROM. This tool is based on the work of Steve Markgraf and has been rewritten for Plan 9 and to be minimal. For a more complete solution on *nix platforms, see: https://github.com/steve-m/N76E003-playground 

The nuvoton9 tool is designed to run on a Raspberry Pi and uses what 9Front refers to as GPIO 0, 1 and 5 to emulate an ICP interface (DATA, CLOCK, RESET). The source code makes use of the 9Front built in RPi GPIO driver. Running the tool without passing a .bin file reference will return the connected MCU's Device ID instead. Refer to the ICP wiring diagram below as GPIO numbers and pins can be different in 9Front.

sblink.bin and fblink.bin are included for testing. Both files blink an LED connected to MCU port P2.1 either slow or fast.

To view the contents of .bin files on 9Front, i recommend Phil9's Vexed tool: https://github.com/telephil9/vexed

sblink.bin and fblink.bin were generated on 9Front using the forthcoming ASM_HEX and HEX_BIN tools.

Build:

mk

mk install

Usage:

nuvoton9 sblink.bin

ICP & LED Wiring:

![nuvoton9_icp_wiring](https://github.com/user-attachments/assets/3fe61e2a-2db0-470e-9cd7-cc584e5fe6be)
