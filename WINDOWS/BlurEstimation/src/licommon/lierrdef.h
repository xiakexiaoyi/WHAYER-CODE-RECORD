
//#include "merror.h"

#define LI_ERR_NONE				0

#define LI_ERR_UNKNOWN			-1
#define LI_ERR_INVALID_PARAM	-2
#define LI_ERR_USER_ABORT		-3

#define LI_ERR_IMAGE_UNKOWN					-100						// un-know fail of image module
#define LI_ERR_IMAGE_FORMAT					(-1+LI_ERR_IMAGE_UNKOWN)	// un_know image format for engine
#define LI_ERR_IMAGE_SIZE_UNMATCH			(-2+LI_ERR_IMAGE_UNKOWN)	// unmatched image size
#define LI_ERR_IMAGE_SIZE_INVALID			(-3+LI_ERR_IMAGE_UNKOWN)	// invalid image size
#define LI_ERR_IMAGE_CHANNEL_INVALID		(-4+LI_ERR_IMAGE_UNKOWN)	// invalid image channel

#define LI_ERR_MEM_UNKNOWN					-200						// un-know fail of memory manager module
#define LI_ERR_ALLOC_MEM_FAIL				(-1+LI_ERR_MEM_UNKNOWN)		// fail to allocate a block memory

#define LI_ERR_CLASS_UNKOWN                 -300

#define LI_ERR_CLASS_SIZE_TOO_BIG           (-1+LI_ERR_CLASS_UNKOWN)

#define LI_ERR_DESCRITPTOR_UNKOWN           -400
#define LI_ERR_DESCRITPTOR_SIZE_TOO_BIG     (-1+LI_ERR_DESCRITPTOR_UNKOWN)

#define LI_ERR_DATA_UNKNOWN					-700
#define LI_ERR_DATA_UNSUPPORT				(-1+LI_ERR_DATA_UNKNOWN)
#define LI_ERR_FILTER_UNKNOWN				-800
#define LI_ERR_FILTER_UNSUPPORT				(-1+LI_ERR_FILTER_UNKNOWN)

#define LI_ERR_SEEDS						-1001

#define LI_ERR_CLASS_NOT_EXIT               -1101

#define LI_ERR_NO_FIND                      -1201


