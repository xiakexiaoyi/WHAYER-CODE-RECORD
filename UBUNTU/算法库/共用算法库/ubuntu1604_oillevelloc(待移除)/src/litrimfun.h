#ifndef _LI_TRIM_FUN_H_
#define _LI_TRIM_FUN_H_

#define PX(FUN)		ALLMeterR##FUN
#define CONFIG_MAX_MEM_BYTES  (1024*1024*30/10)
#define CONFIG_MAX_MASK_BYTES (1204*64*2)

/************************************************************************/
/* Function open/close                                                                     */
/************************************************************************/
//#define ENABLE_WATERMARK

#define ENABLE_TIMER
#define ENABLE_LOG
#define ENABLE_DEBUG
//#define ENABLE_ASSERT
//#define ENABLE_PRINT
#define ENABLE_ERR_PRINT

//#define ENABLE_FACE_DESPOT
//#define ENABLE_EYE_LOCATION
//#define ENABLE_MASK_ONLY_FACE

/************************************************************************/
/* Memory management                                                    */
/************************************************************************/
//#define ENABLE_PLATFORM_DEPENDENT
//#define ENABLE_SEQUENCE_MEMORY_MANAGER
#define ENABLE_RANDOM_MEMORY_MANAGER

/************************************************************************/
/* Code open/close                                                      */
/************************************************************************/
#define  TRIM_RGB
//#define  TRIM_LAB
//#define TRIM_YUV_LAB

//#define TRIM_DATA_14BITS
//#define TRIM_DATA_16BITS
//#define TRIM_DATA_INT8

//#define TRIM_FLT_NL
#define TRIM_INTEGRAL

//#define TRIM_BLOCK_LOOP
//#define TRIM_ONLY_LUMIN_CHANNEL
#define TRIM_MASK_OPT_EXT

/************************************************************************/
/* Platform configure                                                   */
/************************************************************************/
// #define PLATFORM_COACH
// #define PLATFORM_SOFTUNE	
//#define PLATFORM_GREENHILLS

/************************************************************************/
/* Optimization                                                         */
/************************************************************************/
//#define OPTIMIZATION_ARM
//#define OPTIMIZATION_COACH
//#define OPTIMIZATION_THREAD

// #define OPTIMIZATION_ASSEMBLE	
// #define OPTIMIZATION_HAREWARE	
// #define OPTIMIZATION_ENABLE_CACHE
// #define OPTIMIZATION_FASTMEMORY	

/************************************************************************/
/* color space convert                                                  */
/************************************************************************/
//#define OIL_AMMETER_COLOR_CONVERT

#endif	//_LI_TRIM_FUN_H_
