# HEX_BIN

hexbin9 takes an intel ASCII format file and extracts all bytes for the default APROM address range of a Nuvoton MS51 MCU into a binary file ready for rom flashing. This tool is based on the work of Brad Grantham and has been rewritten for Plan 9 and to be minimal. For a complete solution on *nix platforms, see: https://github.com/bradgrantham/hex2bin

Build:

mk

mk install

Usage:

hexbin9 in_file.hex out_file.bin

Example:

hexbin9 sblink.hex sblink.bin
