#ifndef _LI_CHANNEL_H_
#define _LI_CHANNEL_H_

#include "licomdef.h"

#ifdef __cplusplus
extern "C" {
#endif	

/******************************************************************/
#define AccessChannel				PX(AccessChannel)
#define AccessChannel_C1YC2Y		PX(AccessChannel_C1YC2Y)
#define AccessChannel_YC1YC2		PX(AccessChannel_YC1YC2)
#define AccessChannel_RGB565		PX(AccessChannel_RGB565)
#define AccessChannel_C1C2			PX(AccessChannel_C1C2)
#define AccessChannel_Y8C44			PX(AccessChannel_Y8C44)
#define AccessChannel_YC1C2			PX(AccessChannel_YC1C2)

#define AccessLuminChannel_Y8C44	PX(AccessLuminChannel_Y8C44)
#define AccessLuminChannel_Y2C11	PX(AccessLuminChannel_Y2C11)
#define AccessLuminChannel_Y1VY0U	PX(AccessLuminChannel_Y1VY0U)
#define AccessLuminChannel_RGB888	PX(AccessLuminChannel_RGB888)
#define AccessLuminChannel_BGR888	PX(AccessLuminChannel_BGR888)
#define AccessLuminChannel_RGB565   PX(AccessLuminChannel_RGB565)
/******************************************************************/
	
MRESULT	  AccessChannel(MVoid *pImg, MDWord dwImgLine, 
						MVoid *pChannel, MDWord dwChannelLine, 
						MDWord dwWidth, MDWord dwHeight, 
						MLong lChannelNum, MLong lChannelCur, 
						JTYPE_DATA typeData, MBool bGet);	

MVoid	AccessChannel_YC1C2(MVoid *pImg, MDWord dwImgLine, JTYPE_DATA_A typeImg, 
							MVoid *pY, MDWord dwYLine, MVoid *pC1, MDWord dwC1Line, 
							MVoid *pC2, MDWord dwC2Line, JTYPE_DATA_A typeChl, 
							MDWord dwWidth, MDWord dwHeight, MBool bGet);


MVoid	AccessChannel_C1YC2Y(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MByte *pC1, MDWord dwC1Line, 
							 MByte *pC2, MDWord dwC2Line, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet);
MVoid	AccessChannel_YC1YC2(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MByte *pC1, MDWord dwC1Line, 
							 MByte *pC2, MDWord dwC2Line, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet);

MVoid	AccessChannel_RGB565(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MByte *pC1, MDWord dwC1Line, 
							 MByte *pC2, MDWord dwC2Line, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet);

MVoid	AccessChannel_C1C2(MByte *pC1C2, MDWord dwImgLine, 
						   MByte *pC1, MDWord dwC1Line, 
						   MByte *pC2, MDWord dwC2Line, 
						   MDWord dwWidth, MDWord dwHeight, 
						   MBool bGet);

MVoid	AccessChannel_Y8C44(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MByte *pC1, MDWord dwC1Line, 
							 MByte *pC2, MDWord dwC2Line, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet);

//////////////////////////////////////////////////////////////////////////
MVoid	AccessLuminChannel_YC1YC2(MByte *pImg, MDWord dwImgLine, 
							 MByte *pY, MDWord dwYLine, 
							 MDWord dwWidth, MDWord dwHeight, 
							 MBool bGet);
MVoid	AccessLuminChannel_Y8C44(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet);
MVoid	AccessLuminChannel_Y1VY0U(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet);
MVoid	AccessLuminChannel_Y2C11(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet);

MVoid	AccessLuminChannel_RGB888(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet);
MVoid	AccessLuminChannel_BGR888(MByte *pImg, MDWord dwImgLine, 
								  MByte *pY, MDWord dwYLine, 
								  MDWord dwWidth, MDWord dwHeight, 
								  MBool bGet);

MVoid	AccessLuminChannel_RGB565(MByte *pImg, MDWord dwImgLine, 
								 MByte *pY, MDWord dwYLine, 
								 MDWord dwWidth, MDWord dwHeight, 
								 MBool bGet);

#ifdef __cplusplus
}
#endif

#endif//_LI_CHANNEL_H_

