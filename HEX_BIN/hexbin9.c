/*
	Intel Hex to Binary file converter for 9Front
	A refactoring of work by Brad Grantham <https://github.com/bradgrantham/hex2bin>
	Written for native compilation on 9Front by Alex La Spina <alex@arknfv.io>
	This tool is part of the Orbiter 9 MCU Toolchain
*/

#include <u.h>
#include <libc.h>
#include <stdio.h>

struct memory_desc
{
	uchar *p;
	vlong offset;
	ulong size;
	ulong size_written;
};

enum
{
	RECORD_DATA = 0x00,
	RECORD_END_OF_FILE = 0x01,
	RECORD_EXTENDED_SEGMENT_ADDRESS = 0x02,
	RECORD_START_SEGMENT_ADDRESS = 0x03,
	RECORD_EXTENDED_LINEAR_ADDRESS = 0x04,
	RECORD_START_LINEAR_ADDRESS = 0x05
};

static uchar
get_nibble(uchar *s)
{
	uchar value;
	
	if (*s >= '0' && *s <= '9')
	{
		value = *s - '0';
	}
	else if (*s >= 'a' && *s <= 'f')
	{
		value = *s - 'a' + 10;
	}
	else if (*s >= 'A' && *s <= 'F')
	{
		value = *s - 'A' + 10;
	}
	else
	{
		print("Invalid hex character %c\n", *s);
		value = 0;
	}
	return value;
}

static uchar
get_byte(uchar *s)
{
	return get_nibble(s) * 16 + get_nibble(s + 1);
}

static ushort
get_word(uchar *s)
{
	return get_byte(s) * (ushort)256 + get_byte(s + 2);
}

void
memory_desc_init(struct memory_desc *mi, uchar *p, vlong offset, ulong size)
{
	mi->p = p;
	mi->offset = offset;
	mi->size = size;
	mi->size_written = 0;
}

void
memory_desc_store(struct memory_desc *mi, int address, uchar c)
{
	if (address >= mi->offset && address < mi->offset + mi->size)
	{
		int offset = address - mi->offset;
		mi->p[offset] = c;
		if (offset + 1 > mi->size_written)
		{
			mi->size_written = offset + 1;
		}
	}
}

int
read_hex(FILE *f, struct memory_desc *mi, int bad_checksum_err)
{
	uchar *s;
	ulong base_address = 0x0;
	long start_segment_cs = -1;
	long start_segment_ip = -1;
	vlong start_execution = -1;
	int num_bytes;
	int address;
	uchar checksum;
	int i;
	char line[600];
	int line_number = 0;
	
	while (fgets(line, sizeof(line), f) != nil)
	{
		s = (uchar *)line;
		line_number++;
		if (*s != ':')
		{
			print("Invalid format on line %d: %s\n", line_number, line);
			return 0;
		}
		s++;
		checksum = 0;
		num_bytes = get_byte(s);
		if (num_bytes == 0)
		{
			break;
		}
		checksum += (uchar)num_bytes;
		s += 2;
		address = get_word(s);
		checksum += get_byte(s);
		checksum += get_byte(s + 2);
		s += 4;
		uchar type = get_byte(s);
		checksum += type;
		s += 2;
		switch (type)
		{
			case RECORD_DATA:
				for (i = 0; i < num_bytes; i++)
				{
					uchar data = get_byte(s);
					checksum += data;
					s += 2;
					memory_desc_store(mi, base_address + address, data);
					address++;
				}
				break;
			case RECORD_END_OF_FILE:
				print("Line number %d: end of file\n", line_number);
				break;
			case RECORD_EXTENDED_SEGMENT_ADDRESS:
				base_address = (ulong)get_word(s) << 4;
				checksum += get_byte(s);
				checksum += get_byte(s + 2);
				s += 4;
				break;
			case RECORD_START_SEGMENT_ADDRESS:
				start_segment_cs = get_word(s);
				checksum += get_byte(s);
				checksum += get_byte(s + 2);
				s += 4;
				start_segment_ip = get_word(s);
				checksum += get_byte(s);
				checksum += get_byte(s + 2);
				s += 4;
				break;
			case RECORD_EXTENDED_LINEAR_ADDRESS:
				base_address = (ulong)get_word(s) << 16;
				checksum += get_byte(s);
				checksum += get_byte(s + 2);
				s += 4;
				break;
			case RECORD_START_LINEAR_ADDRESS:
				start_execution = ((ulong)get_word(s) << 16) + get_word(s + 4);
				checksum += get_byte(s);
				checksum += get_byte(s + 2);
				checksum += get_byte(s + 4);
				checksum += get_byte(s + 6);
				s += 8;
				break;
			default:
				print("Line number %d: record type not supported\n", line_number);
				return 0;
		}
		checksum = ~checksum + 1;	// verify checksum ~ bitwise not -(n+1)
		uchar file_checksum = get_byte(s);
		if (checksum != file_checksum)
		{
			print("Checksum mismatch on line number %d\n", line_number);
			if (bad_checksum_err)
			{
				return 0;
			}
		}
	}
	if (start_execution >= 0)
	{
		print("Start of execution: 0x%08X\n", start_execution);
	}
	if (start_segment_cs >= 0)
	{
		print("Code Segment register: 0x%04X\n", start_segment_cs);
	}
	if (start_segment_ip >= 0)
	{
		print("Instruction Pointer register: 0x%04X\n", start_segment_ip);
	}
	return 1;
}

void
main(int argc, char *argv[])
{
	print("Hex Bin 9 Started\n");
	
	int size = 16384;	// MS51 flash rom size
	int offset = 0;		// offset 0 in SRECORD
	uchar blank_rom = 0xff;
	
	char *hex_infilename = argv[1];
	char *bin_outfilename = argv[2];
	FILE *fd_infile;
	FILE *fd_outfile;
	
	if (argv[1] == nil || argv[2] == nil)
	{
		goto err;
	}
	
	fd_infile = fopen(hex_infilename, "r");
	if(fileno(fd_infile) < 0)
	{
		print("Cannot open input file %s\n", hex_infilename);
		goto err;
	}
	
	uchar *b = malloc(size);
	if(b == nil)
	{
		print("Cannot allocate %d bytes\n", size);
		fclose(fd_infile);
		goto err;
	}
	memset(b, blank_rom, size);
	struct memory_desc md;
	memory_desc_init(&md, b, offset, size);
	int success = read_hex(fd_infile, &md, 1);
	fclose(fd_infile);
	if(success)
	{
		fd_outfile = fopen(bin_outfilename, "wb");
		if(fileno(fd_outfile) < 0)
		{
			print("Cannot create output file %s\n", bin_outfilename);
			free(b);
			goto err;
		}
		fwrite(b, 1, size, fd_outfile);
		fclose(fd_outfile);
	}
	free(b);
err:
	print("Hex Bin 9 Completed\n");
	exits(0);
}
