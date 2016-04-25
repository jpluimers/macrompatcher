#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "../lib/macrompatcher.h"

#define MINROMSIZE 262144 /* 256k is the smallest we know how to use */
#define MAXROMSIZE 2097152 /* 2MB is the largest we know what to do with */

void usage(char *progname) {
	fprintf(stderr, "Usage: %s [OPTION]\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, " -f, --infile=FILE       Specify the ROM file to operate on\n");
	fprintf(stderr, " -o, --outfile=FILE      Specify the filename to write the modified ROM to\n");
	fprintf(stderr, " -c, --checksum          Compute and print the checksum\n");
	fprintf(stderr, " -r, --romdiskdrvr       Install the ROMdisk driver\n");
	fprintf(stderr, "     --romdiskimage=FILE Install the ROMdisk image file\n");
	return;
}

int main(int argc, char *argv[]) {
	char c;
	int loptind;
	int print_checksum = 0;
	int apply_romdisk = 0;
	char *romname = NULL;
	int romfd;
	char *outname = NULL;
	uint8_t *diskimage = NULL;
	uint32_t diskimagelen = 0;
	RomErr err = (RomErr)0;
	struct option o[] = {
		{"checksum", 0, 0, 'c'},
		{"infile", 1, 0, 'f'},
		{"outfile", 1, 0, 'o'},
		{"romdiskdrvr", 0, 0, 'r'},
		{"romdiskimage", 1, 0, 1},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};
	RomCtx *rom = NULL;
	struct stat sb;

	if(argc < 2) {
		usage(argv[0]);
		exit(1);
	}

	while( (c = getopt_long(argc, argv, "chf:o:r", o, &loptind)) != -1 ) {
		switch(c) {
			case 'c': print_checksum = 1; break;
			case 'f': romname = optarg; break;
			case 'o': outname = optarg; break;
			case 'r': apply_romdisk = 1; break;
			case  1 : /* romdisk image */
				romfd = open(optarg, O_RDONLY);
				if(!romfd < 0) {
					fprintf(stderr, "Could not open romdisk image %s: %d %s\n", optarg, errno, strerror(errno));
					exit(1);
				}

				if(fstat(romfd, &sb) < 0) {
					fprintf(stderr, "Could not stat romdisk image %s: %d %s\n", optarg, errno, strerror(errno));
					exit(1);
				}

				diskimagelen = sb.st_size;
				diskimage = (uint8_t*)calloc(1, diskimagelen);
				if(!diskimage) {
					fprintf(stderr, "Could not allocate romdisk image memory\n");
					exit(1);
				}
				if(read(romfd, diskimage, diskimagelen) < diskimagelen) {
					fprintf(stderr, "Could not read entire romdisk image into memory: %d %s\n", errno, strerror(errno));
					exit(1);
				}
				close(romfd);
				break;
			case 'h': 
			default:
				usage(argv[0]);
				exit(1);
		};
	}

	if(romname == NULL) {
		fprintf(stderr, "No ROM file specified.\n");
		usage(argv[0]);
		exit(1);
	}

	romfd = open(romname, O_RDONLY);
	if(romfd < 0) {
		fprintf(stderr, "Could not open ROM %s: %d %s\n", romname, errno, strerror(errno));
		exit(1);
	}

	rom = (RomCtx*)calloc(1, sizeof(RomCtx));
	if(!rom) {
		fprintf(stderr, "Could not allocate memory for ROM context\n");
		exit(1);
	}

	if(fstat(romfd, &sb) < 0) {
		fprintf(stderr, "Could not fstat ROM file descriptor\n");
		exit(1);
	}

	if(sb.st_size < MINROMSIZE) {
		fprintf(stderr, "ROM is too small.  %d is smaller than minimum ROM size of %d\n", (int)sb.st_size, MINROMSIZE);
		exit(1);
	}

	if(sb.st_size > MAXROMSIZE) {
		fprintf(stderr, "ROM is too big.  %d is bigger than maximum ROM size of %d\n", (int)sb.st_size, MAXROMSIZE);
		exit(1);
	}

	rom->data = (uint8_t*)calloc(1, sb.st_size);
	if(!rom->data) {
		fprintf(stderr, "Could not allocate RAM for the ROM image\n");
		exit(1);
	}

	rom->datasize = rom->filesize = sb.st_size;

	if(sb.st_size < (512*1024)) {
		rom->type = e24bit;
	}else{
		rom->type = e32bit;
	}

	errno = 0;
	if(read(romfd, rom->data, rom->datasize) < rom->datasize) {
		fprintf(stderr, "Could not read ROM image into RAM: %d %s\n", errno, strerror(errno));
		exit(1);
	}

	close(romfd);

	if(apply_romdisk) {
		err = InstallRomdiskDrvr(rom);
		if(err != eSuccess) {
			fprintf(stderr, "Error installing ROMdisk driver: %s\n", GetROMErrString(err));
			exit(1);
		}

		if(diskimage) {
			err = InstallRomdiskImage(rom, diskimage, diskimagelen);
			if(err != eSuccess) {
				fprintf(stderr, "Error installing ROMdisk image: %d %s\n", err, GetROMErrString(err));
				exit(1);
			}
		}
	}

	if(print_checksum) {
		uint32_t checksum = 0;
		err = GetChecksum(rom, &checksum);
		if(err != eSuccess) {
			fprintf(stderr, "Error computing checksum: %s\n", GetROMErrString(err));
		} else {
			printf("Checksum: %#x\n", checksum);
		}
	}

	if(outname) {
		do {
			RomErr err = UpdateChecksum(rom);
			if(err != eSuccess) {
				fprintf(stderr, "Error updating checksum: %s\n", GetROMErrString(err));
				break;
			}

			int outfd = open(outname, O_WRONLY | O_TRUNC | O_CREAT, 0644);
			if(outfd < 0) {
				fprintf(stderr, "Error opening output file %s: %d %s\n", outname, errno, strerror(errno));
				break;
			}

			if(write(outfd, rom->data, rom->datasize) < rom->datasize) {
				fprintf(stderr, "Error writing to output file %s: %d %s\n", outname, errno, strerror(errno));
			}
			close(outfd);
		}while(0);
	}

	if(rom) {
		free(rom->data);
		free(rom);
	}
	exit(0);
}
