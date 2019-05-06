#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include "liimage.h"

#include "ncc.h"
#include "liintegral.h"
#define MAX_K 15
//return result {0 -- 255}
MVoid FastNCC(MVoid *pIntegral, MVoid *pSqIntegral, MLong lIntegralLine,
				MLong lSrcWidth, MLong lSrcHeight,
				MVoid* pTemplate, MLong lTemplWidth, MLong lTemplHeight,
				MVoid* pMask, MLong lMaskLine,//the same with RltWidth, lRltHeight
				MByte* pRlt, MLong lRltLine, MLong lRltWidth, MLong lRltHeight)
{
	MLong i, u, v;
	MByte *pTmpTempl, *pTmpMask, *pTmpRlt;
	MLong lRltExt, lIntExt, lMaskExt;

	JIntegral *pTLInt, *pBRInt;
	JSqIntegral *pTLSqInt, *pBRSqInt;

	MDWord lTemplateSum;
	MDWord lTemplEnergy;
	MLong lTemplateMean;

	MLong KCoeff[MAX_K];
	MLong kLen;
	MLong winSize=lTemplHeight*lTemplWidth;
	MLong lIntegralLineInc, lIntegralLineDec, lTemplWidthDec2;

	//To optimize the unsigned division
	MLong d = lTemplWidth;
	MLong k, N=16, s;
	LOG2_FLOOR(d, k);
	k++;
	s = ((1<<(16+k))+(1<<k)-1)/d;

	JASSERT(lSrcWidth-lTemplWidth+1==lRltWidth);
	JASSERT(lSrcHeight-lTemplHeight+1==lRltHeight);
	JASSERT(lTemplWidth%2 && lTemplHeight%2);

	lTemplateSum = 0;
	pTmpTempl = (MByte*)pTemplate;
	for(u=0; u<lTemplHeight; u++)
		for (v=0; v<lTemplWidth; v++, pTmpTempl++)
			lTemplateSum += *pTmpTempl;
	lTemplateMean = (lTemplateSum<<8)/(winSize);
	//denominator
	lTemplEnergy = 0;
	pTmpTempl = (MByte*)pTemplate;
	for(u=0; u<lTemplHeight; u++)
		for (v=0; v<lTemplWidth; v++, pTmpTempl++)
			lTemplEnergy += (MDWord)SQARE(((MLong)(*pTmpTempl)<<4) - (MLong)(lTemplateMean>>4));
	lTemplEnergy = lTemplEnergy>>8;
	// k components of rectangle basis functions -- approximation to zero mean template mask
	kLen = (MAX(lTemplWidth, lTemplHeight)/2 + 1);
	pTmpTempl = (MByte*)pTemplate + (lTemplHeight/2)*lTemplWidth;
	KCoeff[0] = ((*pTmpTempl)<<8)-lTemplateMean;
	pTmpTempl++;
	for (i=1; i<kLen; i++, pTmpTempl++)
	{
		KCoeff[i] = (*pTmpTempl)-(*(pTmpTempl-1));
	}
	
	lIntegralLineDec = lIntegralLine-1;
	lIntegralLineInc = lIntegralLine+1;
	lTemplWidthDec2 = lTemplWidth-2;
	//numerator, go through the image
	pTLInt = (JIntegral*)pIntegral;
	pBRInt = (JIntegral*)pIntegral + lIntegralLine*(lTemplHeight)+(lTemplWidth);
	pTLSqInt = (JSqIntegral*)pSqIntegral;
	pBRSqInt = (JSqIntegral*)pSqIntegral + lIntegralLine*(lTemplHeight)+(lTemplWidth);
	pTmpRlt = (MByte*)pRlt;
	pTmpMask = (MByte*)pMask;
	lIntExt = lIntegralLine - (lSrcWidth-lTemplWidth);
	lRltExt = lRltLine - (lSrcWidth-lTemplWidth);
	lMaskExt = lMaskLine-(lSrcWidth-lTemplWidth);
	for (u=0; u<lSrcHeight-lTemplHeight; u++, pTmpRlt+=lRltExt,
										pTmpMask += lMaskExt,
										pTLInt+=lIntExt, pBRInt+=lIntExt,
										pTLSqInt+=lIntExt, pBRSqInt+=lIntExt)
	{
		for(v=0; v<lSrcWidth-lTemplWidth; v++, pTmpRlt++,
										  pTmpMask++,
										  pTLInt++, pBRInt++,
										  pTLSqInt++, pBRSqInt++)
		{
			//denominator
			MLong lNomi=0, tmp;
			MDWord L2Energy,L1Energy,lDenomi,LocalEnergy;
			JIntegral *pBR, *pTL, *pBL, *pTR;
			MUInt8 CurVal = 0;

			//if (*pTmpMask != 255) 
			if (*pTmpMask!=0)
			{
				//*pTmpRlt = 0;
				continue;
			}
			L2Energy= (JSqIntegral)(*pBRSqInt - *(pBRSqInt-lTemplWidth)- 
				*(pTLSqInt+lTemplWidth) + *pTLSqInt);
			L1Energy = ((JIntegral)(*pBRInt - *(pBRInt-lTemplWidth) -
				*(pTLInt+lTemplWidth) + *pTLInt));
			//LocalEnergy = L2Energy - SQARE(L1Energy)/winSize;
			//LocalEnergy = L2Energy - SQARE(L1Energy/lTemplWidth);
			LocalEnergy = L2Energy - SQARE((L1Energy*s)>>(N+k));
			
			// 			lDenomi = LocalEnergy * lTemplEnergy;
			//			lDenomi >>= 8;
			lDenomi = 0;
			if (LocalEnergy<=0)continue;
			
			else if(LocalEnergy>65536)
			{
				lDenomi = (LocalEnergy>>8)*lTemplEnergy;
			}
			else if (lTemplEnergy>65536)
			{
				lDenomi = (lTemplEnergy>>8)*LocalEnergy;
			}
			else if(lTemplEnergy<65536 && LocalEnergy<65536)
				lDenomi = (lTemplEnergy*LocalEnergy)>>8;
			if (lDenomi <= 0)continue;
// 			else
// 				lDenomi = (lTemplEnergy>>4)*(LocalEnergy>>4);

			lNomi = KCoeff[0]*L1Energy;
			lNomi >>= 8;
			pTL = pTLInt+lIntegralLineInc, pTR = pTL+lTemplWidthDec2;
			pBR = pBRInt-lIntegralLineInc, pBL = pBR-lTemplWidthDec2;
			for (i=1; i<kLen; i++, pTL+=lIntegralLineInc,
				pBR -= lIntegralLineInc,
				pTR += lIntegralLineDec,
				pBL -= lIntegralLineDec)
			{ 
				if (KCoeff[i]==0)continue;
				L1Energy = (JIntegral)(*pBR - *pBL - *pTR + *pTL);
				lNomi += KCoeff[i]*L1Energy;
			}
			tmp = SQARE(lNomi)/lDenomi;
			tmp = lNomi>0?(tmp>>1)+128:128-(tmp>>1);
			CurVal = TRIM_UINT8(tmp);
			//*pTmpRlt = TRIM_UINT8(tmp);
			//Update mask and save the result
			if (CurVal<=128) *pTmpMask = 255;
			else if(*pTmpRlt<CurVal)
			{
				*pTmpRlt = CurVal;
			}
		}
	}
}

MVoid FSpecial(MByte *LoG, MLong lLine, MLong fSize)
{
	MByte coeffArray[4][11] = {
		//{0, 45, 59, 54},
		{0, 45, 56, 56},
			//{0, 11, 26, 31, 31, 29},
		{0, 11, 26, 30, 30, 30},
		//{0, 4, 13, 21, 24, 25, 24, 23},
		{0, 4, 13, 21, 24, 24, 24, 24},
		//{0, 5, 16, 28, 38, 44, 45, 44, 43, 42, 41}
		{0, 5, 16, 28, 38, 43, 43, 43, 43, 43, 43}
	};//LoG template
	// 	MByte coeffArray[4][11] = {
	// 		{0, 111, 199, 215},
	// 		{0, 22, 60, 88, 101, 105},
	// 		//{0, 11, 26, 30, 30, 30},
	// 		{0, 8, 27, 47, 63, 73, 78, 80},
	// 		//{0, 4, 13, 21, 24, 24, 24, 24},
	// 		{0, 5, 16, 28, 38, 44, 45, 44, 43, 42, 41}
	// 		//{0, 5, 16, 28, 38, 43, 43, 43, 43, 43, 43}
	//	};//Gaussian template
	
	MLong x, y, lIndex, lExt;
	MLong xCen, yCen;
	MByte *pTmp;
	
	xCen = fSize/2;
	yCen = fSize/2;
	
	lIndex = 0;
	if (fSize == 7) lIndex = 0;
	else if (fSize == 11) lIndex = 1;
	else if (fSize == 15) lIndex = 2;
	else if (fSize == 21) lIndex = 3;
	
	pTmp = (MByte*)LoG;
	lExt = lLine-fSize;
	for (y=0; y<fSize; y++, pTmp+=lExt)
	{
		for (x=0; x<fSize; x++, pTmp++)
		{
			MLong lDis = MAX(ABS(x-xCen), ABS(y-yCen));
			*pTmp = coeffArray[lIndex][lDis];
		}
	}
}

#define  MAX_TEMPLATE  25*25
#define MARGINAL_MEM_BYTES 1024*100
MRESULT MoleMatch(MHandle hMemMgr, 
				  MVoid* pSrcImage, MLong lSrcLine, MLong lWidth, MLong lHeight,
				  MVoid* pMask, MLong lMaskLine,
				  MVoid* pMole, MLong lMoleLine,
				  MLong sStart, MLong sEnd, MLong sStep,
				  MLong lMemUsed)
{
	MRESULT res = LI_ERR_NONE;
	
	MLong sigma;
	JIntegral *pIntegral=MNull;
	JSqIntegral *pSqIntegral=MNull;
	MLong i;
	MByte *pTemplate = MNull, *pInter, *pTmpSrc, *pTmpMask, *pTmpRlt;
	MByte *pNCCRlt=MNull; 
	MLong lIntegralLine;

	MLong lIterateNum;
	MLong lTmpHeight, lHeightStep;
	MLong maxFSize = (sEnd-sStart)/sStep*sStep+sStart;
	maxFSize = (3*maxFSize/10)*2+1;
	
	//lRltLine = CEIL_4(lWidth);
	AllocVectMem(hMemMgr, pTemplate, MAX_TEMPLATE, MByte);
	//AllocVectMem(hMemMgr, pNCCRlt, lRltLine*lHeight, MByte);
	//SetVectZero(pNCCRlt, lRltLine*lHeight*sizeof(MByte));
	
	lIntegralLine = CEIL_4(lWidth+1);

	//for mutual use of memory between lumin data and spot mask
	//the order is important
	SetVectZero(pMole, lMoleLine*lHeight*sizeof(MByte));
	
	lIterateNum = 1;
// 	while (/*dwUsedSize+*/lIntegralLine/lIterateNum*lHeight
// 		*(sizeof(JIntegral)+sizeof(JSqIntegral))>=(CONFIG_MAX_MEM_BYTES)/3/*dwTotalSize*/)
// 	{
// 		lIterateNum*=2;
//	}
	while (lMemUsed+lIntegralLine/lIterateNum*lHeight*(sizeof(JIntegral)+sizeof(JSqIntegral))+MARGINAL_MEM_BYTES>(CONFIG_MAX_MEM_BYTES))
	{
		lIterateNum*=2;
	}
	lHeightStep = lHeight/lIterateNum;

	if (lIterateNum == 1) lTmpHeight = (lHeightStep+1);
	else lTmpHeight = lHeightStep+maxFSize+1;
	AllocVectMem(hMemMgr, pIntegral, lIntegralLine*lTmpHeight, JIntegral);
	AllocVectMem(hMemMgr, pSqIntegral, lIntegralLine*lTmpHeight, JSqIntegral);
	//Integral(pSrcImage, lSrcLine, DATA_U8, pIntegral, pSqIntegral, lIntegralLine, lWidth, lHeight);

	pTmpSrc = (MByte*)pSrcImage;
	pTmpMask = (MByte*)pMask;
	pTmpRlt = (MByte*)pMole;
	i = 0;
	do 
	{
		MLong lTmpHeight2;
		if (i==lIterateNum-1) lTmpHeight2 = lHeightStep;
		else lTmpHeight2 = lHeightStep + maxFSize;
		//calculate Integral once
		Integral(pTmpSrc, lSrcLine, DATA_U8, pIntegral, pSqIntegral, lIntegralLine, lWidth, lTmpHeight2);
		sigma = sStart;
		do 
		{
			MLong fSize = (MLong)(3*sigma/10)*2+1;
			MLong rltWidth = lWidth-fSize+1;
			MLong rltHeight = lTmpHeight2-fSize+1;
			//MLong rltLine = CEIL_4(rltWidth);
			MLong rltLine = lMoleLine;
			FSpecial(pTemplate, fSize, fSize);//construct template
			//fast NCC match
			//pInter = (MByte*)pMask + lMaskLine*(fSize/2) + fSize/2;
			pInter = pTmpMask + lMaskLine*(fSize/2)+fSize/2;
			//pNCCRlt = (MByte*)pMole+(fSize/2)*lMoleLine+fSize/2;
			pNCCRlt = pTmpRlt + (fSize/2)*lMoleLine+fSize/2;
			FastNCC(pIntegral, pSqIntegral, lIntegralLine,
				lWidth, lTmpHeight2,
				pTemplate, fSize, fSize, 
				pInter, lMaskLine,
				pNCCRlt, rltLine, rltWidth, rltHeight);
		
			sigma+=sStep;
		} while(sigma<sEnd);
		pTmpSrc+=lHeightStep*lSrcLine;
		pTmpMask +=lHeightStep*lMaskLine;
		pTmpRlt += lHeightStep*lMoleLine;
	} while(++i<lIterateNum);
	
EXT:
	FreeVectMem(hMemMgr, pIntegral);
	FreeVectMem(hMemMgr, pSqIntegral);
	FreeVectMem(hMemMgr, pTemplate);
	//	FreeVectMem(hMemMgr, pNCCRlt);
	return res;
}