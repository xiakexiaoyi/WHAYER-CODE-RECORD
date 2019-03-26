#include "limorph.h"
#include "lidebug.h"
#include "limem.h"

//lSizeX, lSizeY should be odd, so ,it is symmetric, such as 3x3,5x5 and so on
///lFlag是前景对象的像素值，lBgFlag是背景对象的像素值
MRESULT Dilate(MHandle hMemMgr, MVoid* pMaskData, MLong lMaskLine, 
			   MLong lWidth, MLong lHeight, 
			   MLong lSizeX, MLong lSizeY, 
			   MByte lFlag, MByte lBgFlag)
{
	MLong res = LI_ERR_NONE;
	
	MLong x, y, i, j;
	MByte *pRlt=MNull, *pTmpRlt=MNull, *pTmpMask=MNull;
	MLong lExt;
	MLong lXoffset, lYOffset;

	if (lFlag==lBgFlag) {res=LI_ERR_INVALID_PARAM;goto EXT;}

//	MLong nw, n, ne, w, c, e, sw, s, se; 
	
	AllocVectMem(hMemMgr, pRlt, lHeight*lMaskLine, MByte);
	SetVectMem(pRlt, lHeight*lMaskLine, 0, MByte);
	
	lXoffset = lSizeX/2, lYOffset=lSizeY/2;
	lExt = lMaskLine-lWidth+2*lXoffset;
	pTmpRlt = pRlt+lYOffset*lMaskLine+lXoffset;
	pTmpMask = (MByte*)pMaskData+lYOffset*lMaskLine+lXoffset;

	if (lSizeX==3&&lSizeY==3)
	{
		for (y=lYOffset; y<lHeight-lYOffset; y++, pTmpMask+=lExt, pTmpRlt+=lExt)
		{
			for (x=lXoffset; x<lWidth-lXoffset; x++, pTmpMask++, pTmpRlt++)
			{
				if (*pTmpMask!=lFlag) continue;
				*pTmpRlt = lFlag;
				*(pTmpRlt-lMaskLine-1) = lFlag;
				*(pTmpRlt-lMaskLine) = lFlag;
				*(pTmpRlt-lMaskLine+1) = lFlag;
				*(pTmpRlt-1) = lFlag;
				*(pTmpRlt+1) = lFlag;
				*(pTmpRlt+lMaskLine-1) = lFlag;
				*(pTmpRlt+lMaskLine) = lFlag;
				*(pTmpRlt+lMaskLine+1) = lFlag;
			}
		}
	}
	else if (lSizeX==5&&lSizeY==5)
	{
		for (y=lYOffset; y<lHeight-lYOffset; y++, pTmpMask+=lExt, pTmpRlt+=lExt)
		{
			for (x=lXoffset; x<lWidth-lXoffset; x++, pTmpMask++, pTmpRlt++)
			{
				if (*pTmpMask!=lFlag) continue;
				{
					MByte *pTmpRlt2 = pTmpRlt - lYOffset*lMaskLine - lXoffset;
					MLong lRltExt2 = lMaskLine - (lXoffset*2+1);
					for (j=-lYOffset; j<=lYOffset; j++, pTmpRlt2+=lRltExt2)
					{
						for (i=-lXoffset; i<=lXoffset; i++, pTmpRlt2++)
						{
							//*(pTmpRlt+j*lMaskLine+i) = lFlag;
							*pTmpRlt2 = lFlag;
						}
					}
				}
			}
		}
	}


	//row expand
// 	pTmpRlt = pRlt+lYOffset*lMaskLine+lXoffset;
// 	pTmpMask = (MByte*)pMaskData+lYOffset*lMaskLine+lXoffset;
// 	for (y=lYOffset; y<lHeight-lYOffset; y++, pTmpMask+=lExt, pTmpRlt+=lExt)
// 	{
// 		MBool bIsMask = MFalse;
// 		MBool bIsUnMask = MFalse;
// 		if (*pTmpMask == lFlag)  bIsMask = MTrue, bIsUnMask = MFalse; 
// 		for (x=lXoffset; x<lWidth-lXoffset; x++, pTmpMask++, pTmpRlt++)
// 		{
// 			MByte Cur = *pTmpMask;
// 			if(Cur!=lFlag)
// 			{
// 				if(!bIsUnMask) *pTmpRlt = lFlag, bIsUnMask=MTrue;
// // 				if (!bIsUnMask)
// // 				{
// // 					*(pTmpRlt)=lFlag;
// // 					*(pTmpRlt-lMaskLine)=lFlag;
// // 					*(pTmpRlt+lMaskLine)=lFlag;
// // 					*(pTmpRlt-lMaskLine-1)=lFlag;
// // 					*(pTmpRlt+lMaskLine-1)=lFlag;
// // 					*(pTmpRlt-lMaskLine-2)=lFlag;
// // 					*(pTmpRlt+lMaskLine-2)=lFlag;
// // 					bIsUnMask = MTrue;
// //				}
// 				bIsMask = MFalse;
// 			}
// 			else
// 			{
// 				if (!bIsMask) *(pTmpRlt-1) = lFlag, bIsMask=MTrue;
// // 				if (!bIsMask)
// // 				{
// // 					*(pTmpRlt-1)=lFlag;
// // 					*(pTmpRlt-lMaskLine)=lFlag;
// // 					*(pTmpRlt-lMaskLine-1)=lFlag;
// // 					*(pTmpRlt-lMaskLine+1)=lFlag;
// // 					*(pTmpRlt+lMaskLine)=lFlag;
// // 					*(pTmpRlt+lMaskLine-1)=lFlag;
// // 					*(pTmpRlt+lMaskLine+1)=lFlag;
// // 					bIsMask = MTrue;
// //				}
// 				bIsUnMask = MFalse;
// 				*pTmpRlt = lFlag;
// 				*(pTmpRlt+lMaskLine)=lFlag;
// 				*(pTmpRlt-lMaskLine)=lFlag;				
// 			}
// 		}
//	}

	CpyVectMem((MByte*)pMaskData, pRlt, lHeight*lMaskLine, MByte);
EXT:
	FreeVectMem(hMemMgr, pRlt);
	return res;
}

//lSizeX, lSizeY should be odd, so ,it is symmetric, such as 3x3,5x5 and so on
MRESULT Erose(MHandle hMemMgr, MVoid* pMaskData, MLong lMaskLine, 
			  MLong lWidth, MLong lHeight, 
			  MLong lSizeX, MLong lSizeY, 
			  MByte lFlag, MByte lBgFlag)
{
	MLong res = LI_ERR_NONE;
	
	MLong x, y, i, j;
	MByte *pRlt=MNull, *pTmpRlt=MNull, *pTmpMask=MNull;
	MLong lExt;
	MLong lXoffset, lYOffset;

	if (lFlag==lBgFlag) {res=LI_ERR_INVALID_PARAM;goto EXT;}
	
	AllocVectMem(hMemMgr, pRlt, lHeight*lMaskLine, MByte);
	SetVectMem(pRlt, lHeight*lMaskLine, lBgFlag, MByte);
	
	lXoffset = lSizeX/2, lYOffset=lSizeY/2;
	pTmpRlt = pRlt+lYOffset*lMaskLine+lXoffset;
	pTmpMask = (MByte*)pMaskData+lYOffset*lMaskLine+lXoffset;
	lExt = lMaskLine-lWidth+2*lXoffset;
	
	for (y=lYOffset; y<lHeight-lYOffset; y++, pTmpMask+=lExt, pTmpRlt+=lExt)
	{
		for (x=lXoffset; x<lWidth-lXoffset; x++, pTmpMask++, pTmpRlt++)
		{
			if (*pTmpMask!=lFlag) continue;
			for (j=-lYOffset; j<=lYOffset; j++)
			{
				for (i=-lXoffset; i<=lXoffset; i++)
				{
					if (*(pTmpMask+j*lMaskLine+i) != lFlag)
						break;
				}
				if (i<=lXoffset)
					break;
			}
			if (j>lYOffset)*pTmpRlt = lFlag; 
		}
	}
	CpyVectMem((MByte*)pMaskData, pRlt, lHeight*lMaskLine, MByte);
EXT:
	FreeVectMem(hMemMgr, pRlt);
	return res;
}