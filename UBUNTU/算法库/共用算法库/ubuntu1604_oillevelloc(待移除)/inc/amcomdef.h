/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and 		
* confidential information. 
* 
* The information and code contained in this file is only for authorized ArcSoft employees 
* to design, create, modify, or review.
* 
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* 
* If you are not an intended recipient of this file, you must not copy, distribute, modify, 
* or take any action in reliance on it. 
* 
* If you have received this file in error, please immediately notify ArcSoft and 
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/

#ifndef __AMCOMDEF_H__
#define __AMCOMDEF_H__


#if	defined(WINCE) || defined(WIN32)

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif//#ifndef _WCHAR_T_DEFINED

#elif (defined(EKA2) && defined(__GCCE__))
#ifndef _STDDEF_H_
#ifndef __cplusplus
typedef unsigned short wchar_t;
#define __wchar_t_defined
#endif
#endif


#elif defined(__GCCE__) || defined(__GCC32__)
#ifndef _STDDEF_H_
typedef unsigned short wchar_t;
#endif

#endif//#if	defined(WINCE)

#define HYLAPI_EXPORTS

#if defined WIN32 || defined _WIN32
#  define HYL_CDECL __cdecl
#  define HYL_STDCALL __stdcall
#else
#  define HYL_CDECL
#  define HYL_STDCALL
#endif

#ifndef HYL_EXTERN_C
#  ifdef __cplusplus
#    define HYL_EXTERN_C extern "C"
//#    define HYL_DEFAULT(val) = val
#  else
#    define HYL_EXTERN_C
//#    define HYL_DEFAULT(val)
#  endif
#endif

#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined HYLAPI_EXPORTS
#  define HYL_EXPORTS __declspec(dllexport)
#else
#  define HYL_EXPORTS
#endif

#ifndef HYLAPI
#  define HYLAPI(rettype) HYL_EXTERN_C HYL_EXPORTS rettype HYL_CDECL
#endif


#if	defined(__GCC32__)  || defined(__GCCE__) || defined(WINCE) || defined(WIN32)
typedef wchar_t			MWChar;
#else
typedef unsigned short	MWChar;
#endif

typedef long					MLong;
typedef float					MFloat;
typedef double					MDouble;
typedef unsigned char			MByte;
typedef unsigned short	MWord;
typedef unsigned long			MDWord;
typedef void*					MHandle;
typedef char					MChar;
typedef long					MBool;
typedef void					MVoid;
typedef void*					MPVoid;
typedef char*					MPChar;
typedef short					MShort;
typedef const char*				MPCChar;
typedef	MLong					MRESULT;
typedef MDWord					MCOLORREF; 


typedef	signed		char		MInt8;
typedef	unsigned	char		MUInt8;
typedef	signed		short		MInt16;
typedef	unsigned	short		MUInt16;
typedef signed		long		MInt32;
typedef unsigned	long		MUInt32;

#if !defined(M_UNSUPPORT64)
#if defined(_MSC_VER)
typedef signed		__int64		MInt64;
typedef unsigned	__int64		MUInt64;
#else
typedef signed		long long	MInt64;
typedef unsigned	long long	MUInt64;
#endif
#endif

typedef struct __tag_rect
{
	MLong left;
	MLong top;
	MLong right;
	MLong bottom;
} MRECT, *PMRECT;

typedef struct __tag_point
{ 
	MLong x; 
	MLong y; 
} MPOINT, *PMPOINT;

typedef struct //tag_result
{
	
	MFloat result;
}HYED_RESULT;///text read result

typedef struct //tag_rect
{
	MLong left;
	MLong top;
	MLong right;
	MLong bottom;
} HYED_DTAREA;

typedef struct //_tag_result
{
	HYED_RESULT *pResult;
	HYED_DTAREA *DtArea;
	MLong lAreaNum;
	MPOINT origin;
	MPOINT offset;
}HYED_RESULT_LIST;

	/* Defines for image color format*/
#define HYL_IMAGE_GRAY				0x00		//chalk original 0x0A 	
#define HYL_IMAGE_YUYV               0x02        //y,u,y,v,y,u,y,v......
#define HYL_IMAGE_RGB                0x04        //r,g,b,r,g,b.....
#define HYL_IMAGE_BGR                0x06        //b,g,r,b,g,r.....
#define HYL_IMAGE_YUV420          0x08       //yuv420.....
#define HYL_IMAGE_HUE			0x0A
#define HYL_IMAGE_YUV			0x0C

/* Defines for image data*/
typedef struct {
	MLong		lWidth;				// Off-screen width
	MLong		lHeight;			// Off-screen height
	MLong		lPixelArrayFormat;	// Format of pixel array
	union
	{
		struct
		{
			MLong lLineBytes; 
			MVoid *pPixel;
		} chunky;
		struct
		{
			MLong lLinebytesArray[4];
			MVoid *pPixelArray[4];
		} planar;
	} pixelArray;
} HYL_IMAGES, *HYL_PIMAGES;


#define MNull		0
#define MFalse		0
#define MTrue		1

#ifndef MAX_PATH
#define MAX_PATH	256
#endif

#ifdef M_WIDE_CHAR
#define MTChar MWChar
#else 
#define MTChar MChar
#endif

//xsd
#define WYRD_EPSILON  0.00001

#endif
