/*
	Nuvoton APROM 'In Circuit Programmer' for 9Front and RPi
	Supports flashing Nuvoton N76E003 and MS51 1T 8051 MCUs
	A refactoring of work by Steve Markgraf <steve@steve-m.de>
	Written for native compilation on 9Front by Alex La Spina <alex@arknfv.io>
	This tool is part of the Orbiter 9 MCU Toolchain
*/

#include <u.h>
#include <libc.h>
#include <stdio.h>

#define GPIO_DAT	"GPIO0" // RPi4 Pin 11
#define GPIO_CLK	"GPIO1" // RPi4 Pin 12
#define GPIO_RST	"GPIO5" // RPi4 Pin 18

int fd_gpio_ctl;
int fd_gpio_dat;
int fd_gpio_clk;
int fd_gpio_rst;

#define N76E003AQ20_DEVID	0x3650
#define MS51XB9AE_DEVID		0x4b21

#define FLASH_SIZE_N76E		(18 * 1024)
#define	FLASH_SIZE_MS51		(16 * 1024)
#define FLASH_SIZE_MAX		FLASH_SIZE_N76E

#define APROM_FLASH_ADDR	0x0

#define CMD_READ_DEVICE_ID	0x0c
#define CMD_READ_CID		0x0b
#define CMD_MASS_ERASE		0x26
#define CMD_WRITE_FLASH		0x21

void
opengpio(void)
{
	if(bind("#G", "/dev", MBEFORE) < 0)
	{
		sysfatal("No RPi GPIO Device Driver");
	}
	
	fd_gpio_ctl = open("/dev/gpio/ctl", ORDWR);
	
	fd_gpio_dat = open("/dev/gpio/" GPIO_DAT, ORDWR);
	fd_gpio_clk = open("/dev/gpio/" GPIO_CLK, ORDWR);
	fd_gpio_rst = open("/dev/gpio/" GPIO_RST, ORDWR);
	
	if(fd_gpio_ctl < 0 || fd_gpio_dat < 0 || fd_gpio_clk < 0 || fd_gpio_rst < 0)
	{
		sysfatal("Cannot Open RPi GPIO Device File Descriptors");
	}
	
	fprint(fd_gpio_dat, "0");
	fprint(fd_gpio_clk, "0");
	fprint(fd_gpio_rst, "0");
	
	fprint(fd_gpio_ctl, "function in " GPIO_DAT);
	fprint(fd_gpio_ctl, "function out " GPIO_CLK);
	fprint(fd_gpio_ctl, "function out " GPIO_RST);
}

void
closegpio(void)
{
	fprint(fd_gpio_rst, "1");
	
	close(fd_gpio_rst);
	close(fd_gpio_clk);
	close(fd_gpio_dat);
	
	close(fd_gpio_ctl);
	
	unmount("#G", "/dev");
}

void
dirgpio_dat(int dir)
{
	if(dir)
	{
		fprint(fd_gpio_ctl, "function out " GPIO_DAT);
	}
	else
	{
		fprint(fd_gpio_ctl, "function in " GPIO_DAT);
	}
}

void
setgpio_dat(int val)
{
	if(val)
	{
		fprint(fd_gpio_dat, "1");
	}
	else
	{
		fprint(fd_gpio_dat, "0");
	}
}

int
getgpio_dat(void)
{
	char buf[2];
	read(fd_gpio_dat, buf, 2);
	return atoi(buf);
}

void
setgpio_clk(int val)
{
	if(val)
	{
		fprint(fd_gpio_clk, "1");
	}
	else
	{
		fprint(fd_gpio_clk, "0");
	}
}

void
setgpio_rst(int val)
{
	if(val)
	{
		fprint(fd_gpio_rst, "1");
	}
	else
	{
		fprint(fd_gpio_rst, "0");
	}
}

void
sendicp_bit(ulong data, int len)
{
	dirgpio_dat(1);
	
	int i = len;
	while (i--) {
		setgpio_dat((data >> i) & 1);
		setgpio_clk(1);
		setgpio_clk(0);
	}
}

void
initicp(void)
{
	ulong icp_seq = 0x9e1cb6;
	int i = 24;
	
	while (i--) {
		setgpio_rst((icp_seq >> i) & 1);
		sleep(10);
	}
	
	sleep(1);
	
	sendicp_bit(0x5aa503, 24);
}

void
sendicp_cmd(uchar cmd, ulong dat)
{
	sendicp_bit((dat << 6) | cmd, 24);
}

uchar
readicp_byte(int end)
{
	dirgpio_dat(0);
	
	uchar data = 0;
	int i = 8;
	
	while (i--) {
		int state = getgpio_dat();
		setgpio_clk(1);
		setgpio_clk(0);
		data |= (state << i);
	}
	
	dirgpio_dat(1);
	setgpio_dat(end);
	setgpio_clk(1);
	setgpio_clk(0);
	setgpio_dat(0);
	
	return data;
}

ulong
readicp_did(void)
{
	sendicp_cmd(CMD_READ_DEVICE_ID, 0);
	
	uchar devid[2];
	devid[0] = readicp_byte(0);
	devid[1] = readicp_byte(1);
	
	return (devid[1] << 8) | devid[0];
}

uchar
readicp_cid(void)
{
	sendicp_cmd(CMD_READ_CID, 0);
	return readicp_byte(1);
}

void
writeicp_byte(uchar data, int end, int delay1, int delay2)
{
	sendicp_bit(data, 8);
	setgpio_dat(end);
	sleep(delay1);
	setgpio_clk(1);
	sleep(delay2);
	setgpio_dat(0);
	setgpio_clk(0);
}

void
writeicp_erase(void)
{
	sendicp_cmd(CMD_MASS_ERASE, 0x3a5a5);
	writeicp_byte(0xff, 1, 100, 10);
}

ulong
writeicp_flash(ulong addr, ulong len, uchar *data)
{
	sendicp_cmd(CMD_WRITE_FLASH, addr);
	
	for (int i = 0; i < len; i++) {
		writeicp_byte(data[i], i == (len-1), 2, 1);
	}
	
	return addr + len;
}

void
exiticp(void)
{
	setgpio_rst(1);
	sleep(5);
	setgpio_rst(0);
	sleep(10);
	sendicp_bit(0xf78f0, 24);
	sleep(1);
	setgpio_rst(1);
}

void
main(int argc, char *argv[])
{
	opengpio();
	print("Nuvoton 9 Started\n");
	
	initicp();
	
	ushort devid = readicp_did();
	print("Found Device ID:\t0x%04x\n", devid);
	uchar cid = readicp_cid();
	print("With CID:\t\t\t0x%02x\n", cid);
	
	int aprom_size = 0;
	int aprom_program_size = 0;
	
	if (devid == N76E003AQ20_DEVID) {
		aprom_size = FLASH_SIZE_N76E;
	} else if (devid == MS51XB9AE_DEVID) {
		aprom_size = FLASH_SIZE_MS51;
	} else {
		print("Unknown MCU or not connected! Data: %s Clock: %s Reset: %s\n", GPIO_DAT, GPIO_CLK, GPIO_RST);
		goto err;
	}
	print("APROM Flash Size:\t%d bytes\n", aprom_size);
	
	uchar write_data[FLASH_SIZE_MAX];
	memset(write_data, 0xff, sizeof(write_data));
	
	if (argv[1] == nil) {
		goto err;
	}
	
	FILE *file = nil;
	file = fopen(argv[1], "rb");
	if (fileno(file) < 0) {
		print("BIN file not found!\n");
		goto err;
	}
	
	aprom_program_size = fread(write_data, 1, aprom_size, file);
	print("-- Flashing %s To APROM --\n", argv[1]);
	writeicp_erase();
	writeicp_flash(APROM_FLASH_ADDR, aprom_program_size, write_data);
	print("Programmed APROM Size:\t%d bytes\n", aprom_program_size);
	fclose(file);
	
err:
	exiticp();
	closegpio();
	print("Nuvoton 9 Completed\n");
	exits(0);
}
