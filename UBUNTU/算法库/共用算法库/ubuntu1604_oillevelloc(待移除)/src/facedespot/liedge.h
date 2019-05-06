#ifndef _LI_EDGE_
#define _LI_EDGE_

#include "licomdef.h"

#define  EDGE    2
#define  NONEDGE 1


//EDGE TYPE
typedef enum {
	TYPE_SOBEL = 0x00,
	TYPE_CANNY = 0x01
}EDGE_TYPE;

/************************************************************************/
#define  Edge					PX(Edge)
#define  Canny					PX(Canny)
#define  Gradient				PX(Gradient)
#define  SobelCanny				PX(SobelCanny)
#define  HistoAnalysis			PX(HistoAnalysis)
#define Gradient_WithMask		PX(Gradient_WithMask)
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif	

MRESULT Edge(MHandle hMemMgr,
	MByte* pSrcData, MLong lSrcLine, MLong lWidth, MLong lHeight,
	MByte* pEdgeData, MLong lEdgeLine, EDGE_TYPE typeEdge);

MRESULT Canny(MHandle hMemMgr, MShort* Gx, 
			  MShort* Gy, MLong lGradLine,
			  MLong lSrcWidth, MLong lSrcHeight, 
			  MVoid *pRlt, MLong lRltLine, 
			  MLong lowThres, MLong highThres);

MVoid Gradient(MVoid *pImgSrc, MLong lSrcLine, 
			   MShort *Gx, MShort *Gy, MLong lGraLine,
			   MUInt16 *pMag, MLong lMagLine,
			   MLong lWidth, MLong lHeight, MDWord* plSum,
			   MDWord* maxGrad, MDWord *plNum);

MRESULT SobelCanny(MHandle hMemMgr, MShort* Gx, 
			  MShort* Gy, MLong lGradLine,
			  MVoid *pCanny, MLong lCannyLine, 
			  MVoid *pSobel, MLong lSobelLine,
			  MVoid *pMask, MLong lMaskLine,
			  MLong lWidth, MLong lHeight, 
			  MRECT* pRtProtect,
			  MLong dwSum, MBool bIsLeaveSmall,
			  MLong lowThres, MLong highThres);

MRESULT HistoAnalysis(MHandle hMemMgr, MShort *Gx, MShort* Gy, MLong lGradLine,
					  MVoid* pSkinMask, MLong lMaskLine,
					  MLong lWidth, MLong lHeight, 
					  MDWord dwMaxGrad, MLong lPercentage,
					  MLong* plHighThres);


MVoid Gradient_WithMask(MVoid *pSrcData, MLong lSrcLine, 
    MShort *Gx, MShort *Gy, MLong lGraLine,
    MByte* pMaskData, MLong lMaskLine,MByte maskflag,
    MLong lWidth, MLong lHeight, MDWord* plSum,
    MDWord* pMaxGrad, MDWord *plNum);
	
#ifdef __cplusplus
}
#endif

#endif