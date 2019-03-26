#include "limorph.h"
#include "lidebug.h"
#include "limem.h"

//lSizeX, lSizeY should be odd, so ,it is symmetric, such as 3x3,5x5 and so on
MRESULT Dilate(MHandle hMemMgr, MVoid* pMaskData, MLong lMaskLine, 
			   MLong lWidth, MLong lHeight, 
			   MLong lSizeX, MLong lSizeY, 
			   MByte lFlag/*=0*/)
{
	MLong res = LI_ERR_NONE;
	
	MLong x, y, i, j;
	MByte *pRlt=MNull, *pTmpRlt=MNull, *pTmpMask=MNull;
	MLong lExt;
	MLong lXoffset, lYOffset;
	
	AllocVectMem(hMemMgr, pRlt, lHeight*lMaskLine, MByte);
	SetVectMem(pRlt, lHeight*lMaskLine, 255, MByte);
	
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
				if (*pTmpMask!=lFlag) 
				{
					//*pTmpRlt = *pTmpMask;
					continue;
				}
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
				if (*pTmpMask!=lFlag) 
				{
					//*pTmpRlt = *pTmpMask;
					continue;
				}
				else
				{
					MByte *pTmpRlt2 = pTmpRlt - lYOffset*lMaskLine - lXoffset;
					MByte *pTmpMask2 = pTmpMask - lYOffset*lMaskLine - lXoffset;
					MLong lRltExt2 = lMaskLine - (lXoffset*2+1);
					for (j=-lYOffset; j<=lYOffset; j++, pTmpRlt2+=lRltExt2, pTmpMask2+=lRltExt2)
					{
						for (i=-lXoffset; i<=lXoffset; i++, pTmpRlt2++, pTmpMask2++)
						{
							//*(pTmpRlt+j*lMaskLine+i) = lFlag;
							//if (*pTmpMask2==255||*pTmpMask2==lFlag)
							*pTmpRlt2 = lFlag;
						}
					}
				}
			}
		}
	}

	CpyVectMem((MByte*)pMaskData, pRlt, lHeight*lMaskLine, MByte);
EXT:
	FreeVectMem(hMemMgr, pRlt);
	return res;
}


MRESULT Dilate_KeepOthers(MHandle hMemMgr, MVoid* pMaskData, MLong lMaskLine, 
			   MLong lWidth, MLong lHeight, 
			   MLong lSizeX, MLong lSizeY, 
			   MByte lFlag/*=0*/)
{
	MLong res = LI_ERR_NONE;
	
	MLong x, y, i, j;
	MByte *pRlt=MNull, *pTmpRlt=MNull, *pTmpMask=MNull;
	MLong lExt;
	MLong lXoffset, lYOffset;
	
	AllocVectMem(hMemMgr, pRlt, lHeight*lMaskLine, MByte);
	SetVectMem(pRlt, lHeight*lMaskLine, 255, MByte);
	
	lXoffset = lSizeX/2, lYOffset=lSizeY/2;
	lExt = lMaskLine-lWidth+2*lXoffset;
	pTmpRlt = pRlt+lYOffset*lMaskLine+lXoffset;
	pTmpMask = (MByte*)pMaskData+lYOffset*lMaskLine+lXoffset;

	for (y=lYOffset; y<lHeight-lYOffset; y++, pTmpMask+=lExt, pTmpRlt+=lExt)
	{
		for (x=lXoffset; x<lWidth-lXoffset; x++, pTmpMask++, pTmpRlt++)
		{	
			MByte val = *pTmpMask;
			if (val==255) continue;

			if (val!=lFlag) 
			{
				*pTmpRlt = val;
			}
			else
			{
				MByte *pTmpRlt2 = pTmpRlt - lYOffset*lMaskLine - lXoffset;
				MByte *pTmpMask2 = pTmpMask - lYOffset*lMaskLine - lXoffset;
				MLong lRltExt2 = lMaskLine - (lXoffset*2+1);
				for (j=-lYOffset; j<=lYOffset; j++, pTmpRlt2+=lRltExt2, pTmpMask2+=lRltExt2)
				{
					for (i=-lXoffset; i<=lXoffset; i++, pTmpRlt2++, pTmpMask2++)
					{
						if (*pTmpMask2==255||*pTmpMask2==lFlag)
						*pTmpRlt2 = lFlag;
					}
				}
			}
		}
	}

	CpyVectMem((MByte*)pMaskData, pRlt, lHeight*lMaskLine, MByte);
EXT:
	FreeVectMem(hMemMgr, pRlt);
	return res;
}

//lSizeX, lSizeY should be odd, so ,it is symmetric, such as 3x3,5x5 and so on
MRESULT Erose(MHandle hMemMgr, MVoid* pMaskData, MLong lMaskLine, 
			  MLong lWidth, MLong lHeight, 
			  MLong lSizeX, MLong lSizeY, 
			  MByte lFlag/*=0*/)
{
	MLong res = LI_ERR_NONE;
	
	MLong x, y, i, j;
	MByte *pRlt=MNull, *pTmpRlt=MNull, *pTmpMask=MNull;
	MLong lExt;
	MLong lXoffset, lYOffset;
	
	AllocVectMem(hMemMgr, pRlt, lHeight*lMaskLine, MByte);
	SetVectMem(pRlt, lHeight*lMaskLine, 255, MByte);
	
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
			if (j>lYOffset)*pTmpRlt = 0; 
		}
	}
	CpyVectMem((MByte*)pMaskData, pRlt, lHeight*lMaskLine, MByte);
EXT:
	FreeVectMem(hMemMgr, pRlt);
	return res;
}