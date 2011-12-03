#include "macrompatcher.h"

const char *GetROMErrString(RomErr err) {
	switch(err) {
	case eSuccess:  return "Success!";
	case eNotSupp:  return "Modification not supported for this ROM";
	case eNotFound: return "Couldn't find where to apply the modification";
	case eParmErr:  return "Parameter error";
	};
	return "Unknown error";
}
