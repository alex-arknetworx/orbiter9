# orbiter9
Hand Coded MCU Toolchain for Plan 9

A set of minimal 'Unix like' tools for microcontroller programming on Plan 9 and 9Front.

This toolchain comprises 4 tools:

-C Compiler tool which outputs assembly from a C source file [C_ASM]

-Assembler tool which outputs ASCII hex from a S source file [ASM_HEX]

-Hex to Bin tool which generates a binary from ASCII hex for flashing to the MCU rom [HEX_BIN]

-Bin to Rom tool which is a MCU specific rom flashing tool [BIN_ROM]

The first target architecture for this toolchain is the 8 bit 1T 8051. The first reference MCUs are Nuvoton N76E003AQ20 and Nuvoton MS51XB9AE with 18kB or 16kB ROM and 768B or 1kB SRAM along with uart, i2c and spi buses. These MCUs require minimal external components. Two capacitors, two resistors and an LED are all that is needed along with 9Front on a Raspberry Pi to assemble and flash the blinking LED demonstration.
