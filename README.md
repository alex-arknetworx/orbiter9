# orbiter9
Hand Coded MCU Toolchain for Plan 9

Minimal 'Unix like' tools for microcontroller development on Plan 9 and 9Front.

This toolchain comprises 4 tools:

-C Compiler tool which outputs assembly from a C source file [C_ASM]

-Assembler tool which outputs ASCII hex from a S source file [ASM_HEX]

-Hex to Bin tool which creates a binary for flashing to the MCU [HEX_BIN]

-Bin to Rom tool which is a MCU specific flashing tool [BIN_ROM]

The first target architecture for this toolchain is the 1T 8051 8-bit.

The first reference MCUs are Nuvoton N76E003AQ20 and Nuvoton MS51XB9AE with 18kB/16kB ROM and 768B/1kB SRAM along with a uart, i2c and spi.
These MCUs require minimal external components. Two capacitors, two resistors and an LED are all that is needed along with 9Front on a Raspberry Pi to assemble and flash the blinking LED demonstration.
