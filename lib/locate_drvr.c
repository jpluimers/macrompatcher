#include <arpa/inet.h>
#include <stdio.h>
#include "macrompatcher.h"

RomErr GetDRVROffset(RomCtx *rom, uint16_t drvrid, uint32_t *offset) {
	uint8_t *tmpptr;
	RomErr ret = eNotFound;

	if(!rom || !rom->data || !offset) return eParmErr;

	// 0x1A stores the 4 byte location of the resources
	tmpptr = rom->data + 0x1A;

	uint32_t resourcesoffset = *(uint32_t*)tmpptr;
	resourcesoffset = ntohl(resourcesoffset);

	if(resourcesoffset > rom->datasize) return eNotFound;
	
	tmpptr = rom->data + resourcesoffset;

	if(rom->type == e32bit) {
		uint32_t indirect = *(uint32_t*)tmpptr;
		indirect = ntohl(indirect);
		tmpptr = rom->data + indirect;
		do {
			uint32_t nextoff;
			uint32_t data;
			uint32_t type;
			uint16_t resid;
			data = *(uint32_t*)(tmpptr+12);
			data = ntohl(data);
			type = *(uint32_t*)(tmpptr+16);
			type = ntohl(type);
			resid = *(uint16_t*)(tmpptr+20);
			resid = ntohs(resid);
			if(type == 0x44525652 && resid == drvrid) {
				*offset = data;
				ret = eSuccess;
				break;
			}
			
			nextoff = *(uint32_t*)(tmpptr+8);
			nextoff = ntohl(nextoff);
			if(nextoff > rom->datasize) break;
			tmpptr = rom->data + nextoff;
		}while(1);
	} else if(rom->type == e24bit) {
		uint16_t typelistoff = *(uint16_t*)(tmpptr+28);
		typelistoff = ntohs(typelistoff);
		tmpptr += 4;
		tmpptr += typelistoff;

		uint16_t numresourcetypes = *(uint16_t*)(tmpptr);;
		numresourcetypes = ntohs(numresourcetypes);
		tmpptr += 2;

		uint16_t drvroff = 0;
		uint16_t numdrvrs = 0;
		int i;
		for(i = 0; i < 11; i++) {
			uint32_t resourceid;
			uint16_t numresources;
			uint16_t listoffset;
			resourceid = *(uint32_t*)(tmpptr + (i*8));
			resourceid = ntohl(resourceid);
			if(resourceid != 0x44525652) continue; /* DRVR 4char const */

			numresources = *(uint16_t*)(tmpptr + (i*8) + 4);
			listoffset = *(uint16_t*)(tmpptr + (i*8) + 6);
			numresources = ntohs(numresources);
			listoffset = ntohs(listoffset);

			numdrvrs = numresources;
			drvroff = listoffset;
			break;
		}

		if(!drvroff) return eNotFound;

		tmpptr = rom->data + resourcesoffset + typelistoff + drvroff + 4;
		for(i = 0; i <= numdrvrs; i++) {
			uint16_t resid;
			resid = *(uint16_t*)(tmpptr + (i*12));
			resid = ntohs(resid);
			if(resid == drvrid) {
				uint32_t attroff;
				attroff = *(uint32_t*)(tmpptr + (i*12) + 4);
				attroff = ntohl(attroff);
				attroff &= 0x00FFFFFF;
				*offset = attroff;
				ret = eSuccess;
				break;
			}
		}
	}

	return ret;
}
