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


#elif defined(__GCCE__) || defined(__GCC32__)

#ifndef _STDDEF_H_
//typedef unsigned short wchar_t;
#endif//#ifndef _STDDEF_H_

#endif//#if	defined(WINCE)


#if	defined(__GCC32__)  || defined(__GCCE__) || defined(WINCE) || defined(WIN32)
typedef unsigned short		MWChar;
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

#if defined(_MSC_VER)
typedef signed		__int64		MInt64;
typedef unsigned	__int64		MUInt64;
#else
typedef signed		long long	MInt64;
typedef unsigned	long long	MUInt64;
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

#endif
