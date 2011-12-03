#ifndef __MACROMPATCHER_H__
#define __MACROMPATCHER_H__ 1
#include <stdint.h>

/* Known types of ROM images. */
enum RomType {
	eUnknown = 0,

	/* 24bit ROMs are Mac II, IIx, SE/30 ROMs that are normally 256KB */
	e24bit   = 1,

	/* 32bit ROMs are IIsi, IIci, and higher that are normally 
	 * 512KB or larger.
	 */
	e32bit   = 2
};
typedef enum RomType RomType;

struct romctx {
	/* pointer to the in-memory instance of the ROM, ready to be modified.
	 * This pointer can change, if something needs to increase the size
	 * of the ROM, it can be realloc'd to be larger.
	 */
	uint8_t *data;

	/* Size of the memory pointed to by data above.  If the ROM image grows
	 * via realloc, this needs to be updated.
	 */ 
	uint32_t datasize;

	/* The original filesize that was read in from disk.  If the ROM image
	 * size hasn't changed in the modification proces, this will == datasize
	 */
	uint32_t filesize;

	/* Represents what type the ROM is.  Some ROMs are structured 
	 * differently, so modification routines will need to key off this
	 * to determine how and if they can perform their modification.
	 */
	RomType type;
};
typedef struct romctx RomCtx;

/* Standard error types the ROM modification routines can return */
enum RomErr {
	eSuccess = 0,

	/* The modification is not supported for the given ROM */
	eNotSupp = 1, 

	/* Couldn't find where to apply the patch in the given ROM */
	eNotFound = 2,

	/* Error with one of the parameters */
	eParmErr = 3
};
typedef enum RomErr RomErr;

RomErr GetChecksum(RomCtx *rom, uint32_t *checksum);
RomErr UpdateChecksum(RomCtx *rom);
const char *GetROMErrString(RomErr err);
RomErr GetDRVROffset(RomCtx *rom, uint16_t drvrid, uint32_t *offset);
RomErr InstallRomdiskDrvr(RomCtx *rom);

#endif /* __MACROMPATCHER_H__ */
