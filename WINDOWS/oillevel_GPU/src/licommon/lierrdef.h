
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

#define LI_ERR_DENOISE_UNKNOWN				-400						// un-know fail of noise remove module
#define LI_ERR_DENOISE_TYPE					(-1+LI_ERR_DENOISE_UNKNOWN)	// unsupported noise remove type

#define LI_ERR_NOISE_UNKNOWN				-500						// un-know fail of noise estimate module
#define LI_ERR_NOISE_TYPE					(-1+LI_ERR_NOISE_UNKNOWN)	// unsupported noise estimate type
#define LI_ERR_NOISE_INVALID				-502

#define LI_ERR_PYRAMID_UNKOWN				-600
#define LI_ERR_PYRAMID_FILTER_UNSUPPORT		(-1+LI_ERR_PYRAMID_UNKOWN)
#define LI_ERR_PYRAMID_LEVEL_UNSUPPORT		(-2+LI_ERR_PYRAMID_UNKOWN)
#define LI_ERR_PYRAMID_CHANNEL_UNSUPPORT	(-3+LI_ERR_PYRAMID_UNKOWN)

#define LI_ERR_DATA_UNKNOWN					-700
#define LI_ERR_DATA_UNSUPPORT				(-1+LI_ERR_DATA_UNKNOWN)
#define LI_ERR_FILTER_UNKNOWN				-800
#define LI_ERR_FILTER_UNSUPPORT				(-1+LI_ERR_FILTER_UNKNOWN)

#define LI_ERR_DETECTING					-900
#define LI_ERR_DETECTING_NOFACE				-901
#define LI_ERR_TOO_MANY_FACES				-902
#define LI_ERR_FACE_ERROR					-903
#define LI_ERR_DETECTING_NOEYE				-904

#define LI_ERR_MASK							-1000
#define LI_ERR_SEEDS						-1001
#define LI_ERR_MASK_TOO_LARGE				-1002
#define LI_ERR_NO_SKIN_MASK					-1003

#define LI_ERR_BLOCK						-1100
#define LI_ERR_BLOCK_POS					-1101

#define LI_ERR_NO_FIND                      -1201
#define LI_ERR_CLASS_NOT_EXIT               -1301
#define LI_ERR_CLASS_SIZE_TOO_BIG           -1401
#define LI_ERR_INDEX_NO_FIND                -1501
#define LI_ERR_SEEDS_NUMBER                 -1601
#define LI_ERR_INVALID_POINT                -1701
#define LI_ERR_INVALID_NUMBERDATA           -1801
#define LI_ERR_NOT_INIT                     -1901
#define LI_ERR_RANSAC_PIXEL                 -2001
#define LI_ERR_GET_POINT                    -2101

#define LI_ERR_REGION_NUM                   -2201
#define LI_ERR_INSIDE_PARAM_INVALID         -2301