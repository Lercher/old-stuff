#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <conio.h>

/* Do we have safe operation (1) or actual code (0) */
#define SAFE 0
#define VERS "1.1"

#ifndef __SMALL__
#  error "Small model required"
#endif

#define SECTORLEN 512

enum operation { NIX, GET, PUT, TEST };

char filename[128];
FILE *f;
char buf[SECTORLEN];
char buf1[SECTORLEN];
int disk;

int loud;
#define INFO if (loud) printf


int waitasecond(int howlong)
{   time_t t0, t1, t2;

	t0 = time(NULL) + howlong + 1;
	t2 = 0;
	INFO("Warmboot requested. Press any key to abort.\n");
	while(1)
	{
		t1 = time(NULL);
		if (kbhit())
		{
			(void) getch();
			INFO("\n");
			return 0;
		}
		if (t2 != t1)
		{
			t2 = t1;
			INFO("\r%d  ", (int)(t0 - t1));
		}
		if (t1 >= t0)
			break;
	}
	INFO("\n");
	return 1;
}

void warmboot(void)
{
	INFO("Warmboot ...\n");
#if SAFE
	printf("*** here is the warmboot ***\n");
	exit(99);
#endif
	poke(0x40, 0x72, 0x1234); /* for coldboot use 0 instead of 0x1234 */
	__emit__(0xEA, 0x00, 0x00, 0xFF, 0xFF); /* jmp ffff:0 */
}

void readfile(void)
{   size_t n;

	INFO("Reading file %s\n", filename);
	f = fopen(filename, "rb");
	if (!f)
	{
		perror(filename);
		printf("\nOperation aborted.\n");
		exit(99);
	}
	n = fread(buf, sizeof(char), sizeof(buf), f);
	if (n != SECTORLEN)
	{
		printf("Length of file %s is less than %d.", filename, SECTORLEN);
		printf(" Operation aborted.\n");
		exit(99);
	}
	if (EOF == fclose(f))
	{
		perror(filename);
		printf("\nOperation aborted.\n");
		exit(99);
	}
	if (buf[0x1fe] != 0x55 || buf[0x1ff] != 0xAA)
	{
		printf("File %s does not contain a valid MBR. Operation aborted.\n", filename);
		exit(99);
	}
}

void putfile(void)
{
#if !SAFE
	struct REGPACK r;
#endif

	INFO("Writing Master boot record to disk %d\n", disk);
#if SAFE
	printf("*** write to disk %d would occur here ***\n", disk);
	return;
#else
	/* call BIOS */
	r.r_ax = 0x0301; 		/* Func 2, one sector */
	r.r_bx = FP_OFF(buf);	/* Offset Data buffer */
	r.r_cx = 0x0001;		/* We are going to write cyl 0 sector 1 */
	r.r_dx = 0x0080 + disk; /* We write to Head 0 of Harddisk disk */
	r.r_es = FP_SEG(buf);	/* Segment of data buffer */
	intr(0x13, &r);
	/* all we need to check now is the CF, if CF=CY then error */
	if (r.r_flags & 0x0001)
	{
		printf("Error 0x%04X writing MBR to disk %d. Operation aborted.\n", r.r_ax, disk);
		exit(99);
	}
#endif
	/* hurray */
}

void getrecord(void)
{   struct REGPACK r;

	INFO("Reading Master boot record from disk %d\n", disk);
	/* call BIOS */
	r.r_ax = 0x0201; 		/* Func 2, one sector */
	r.r_bx = FP_OFF(buf);	/* Offset Data buffer */
	r.r_cx = 0x0001;		/* We are going to Read cyl 0 sector 1 */
	r.r_dx = 0x0080 + disk; /* We read from Head 0 of Harddisk disk */
	r.r_es = FP_SEG(buf);	/* Segment of data buffer */
	intr(0x13, &r);
	/* all we need to check now is the CF, if CF=CY then error */
	if (r.r_flags & 0x0001)
	{
		printf("Error 0x%04X reading MBR from disk %d. Operation aborted.\n", r.r_ax, disk);
		exit(99);
	}
	/* hurray */
}

void writerecord(void)
{   size_t n;

	INFO("Writing file %s\n", filename);
	f = fopen(filename, "wb");
	if (!f)
	{
		perror(filename);
		printf("\nOperation aborted.\n");
		exit(99);
	}
	n = fwrite(buf, sizeof(char), sizeof(buf), f);
	if (n != SECTORLEN)
	{
		perror(filename);
		printf("\nOperation aborted.\n");
		exit(99);
	}
	if (EOF == fclose(f))
	{
		perror(filename);
		printf("\nOperation aborted.\n");
		exit(99);
	}
}

int compare_ne(void)
{	int result;

	memcpy(buf1, buf, sizeof(buf));
	getrecord();
	if (0 == memcmp(buf, buf1, sizeof(buf)))
		result = 0;
	else
		result = 1;
	memcpy(buf, buf1, sizeof(buf));
	INFO("Result of Compare: %sequal\n", result ? "not " : "");
	return result;
}

int main(int argc, char *argv[])
{	int result;
	int opt, boot;
	enum operation oper;

	if (argc < 2)
	{
		printf("This is "__FILE__" Version "VERS", Compiled "__DATE__" "__TIME__".\n");
		printf("(c) 1996 by Singhammer Datentechnik GmbH, M. Lercher\n");
		printf("usage: %s [-bnq] [-d#] [-B#] {-p|-g|-t} file\n", argv[0]);
		printf("\t This program reads and writes partition records.\n");
		printf("\t n - no warmboot after effective put operation\n");
		printf("\t b - immediate warmboot\n");
		printf("\t B - warmboot after # seconds.\n");
		printf("\t d - # indicates Disk Number, 0 is first HD, 1 is second HD, ...\n");
		printf("\t t - test partition record against file only\n");
		printf("\t g - get master boot record and write it to file\n");
		printf("\t p - put master boot record that was read from file\n");
		printf("\t q - quiet operation\n");
		printf("Errorlevel:\n");
		printf("\t 0 - record matches file\n");
		printf("\t 1 - record didn't match file\n");
		printf("\t99 - General error, No proper Operation or Invalid flag given\n");
#if SAFE
		printf("This is a SAFE Version: Warmboot and Writing to MBR are disabled.\n");
#endif
		return 99;
	}

	loud = 1;
	boot = 1;
	disk = 0;
	oper = NIX;

	while(EOF != (opt=getopt(argc, argv, "bnqB:d:g:t:p:")))
	{
		switch(opt)
		{
			case '?':
				printf("Illegal flag. Operation aborted.\n");
				return 99;
			case 'q':
				loud = 0;
				break;
			case 'n':
				boot = 0;
				break;
			case 'B':
				if (waitasecond(atoi(optarg)))
					warmboot();
				return 0;
			case 'b':
				warmboot();
				return 99; /* This cannot happen */
			case 'd':
				disk = atoi(optarg);
				if (disk<0 || disk>15)
				{
					printf("-d requires 0 <= # <= 15\n");
					return 99;
				}
				break;
			case 'p':
				strcpy(filename, optarg);
				oper = PUT;
				break;
			case 'g':
				strcpy(filename, optarg);
				oper = GET;
				break;
			case 't':
				strcpy(filename, optarg);
				oper = TEST;
				break;
		}
	}
	INFO("This is "__FILE__" Version "VERS", Compiled "__DATE__" "__TIME__".\n");
	INFO("(c) 1996 by Singhammer Datentechnik GmbH, M. Lercher\n");
#if SAFE
	printf("This is a SAFE Version: Warmboot and Writing to MBR are disabled.\n");
#endif
	switch(oper)
	{
		case NIX:
			printf("-p, -g or -t required. Operation aborted.\n");
			return 99;
		case GET:
			getrecord();
			writerecord();
			break;
		case PUT:
			readfile();
			if (compare_ne())
			{
				putfile();
				if (boot)
					warmboot();
				else
					INFO("No warmboot requested. Remember to boot manually.\n");
				result = 1;
			}
			else
			{
				INFO("No need to write record.\n");
				if (boot)
					INFO("No need to warmboot.\n");
				result = 0;
			}
			break;
		case TEST:
			readfile();
			if (compare_ne())
				result = 1;
			else
				result = 0;
			break;
		dafault:
			printf("Internal error: Unknown Operation. Operation aborted.\n");
			return 99;
	}
	INFO("Operation completed successfully.\n");
	return result;
}