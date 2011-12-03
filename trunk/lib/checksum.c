#include <stdlib.h>
#include <arpa/inet.h>
#include "macrompatcher.h"

RomErr GetChecksum(RomCtx *rom, uint32_t *checksum) {
	uint8_t *tmpptr = NULL;
	size_t i;

	if(!rom || !rom->data || !checksum) return eParmErr;

	tmpptr = rom->data+4;

	*checksum = 0;
	for(i = 0; i < rom->filesize; i++) {
		*checksum += tmpptr[i++] << 8;
		*checksum += tmpptr[i];
	}

	return eSuccess;
}

RomErr UpdateChecksum(RomCtx *rom) {
	uint32_t checksum;
	RomErr ret;
	ret = GetChecksum(rom, &checksum);
	if(ret != eSuccess) return ret;

	*(uint32_t*)(rom->data) = htonl(checksum);
	return eSuccess;
}
