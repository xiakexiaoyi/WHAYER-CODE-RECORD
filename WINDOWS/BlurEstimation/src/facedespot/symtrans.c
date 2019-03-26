#include "symtrans.h"

#include "limem.h"
#include "limath.h"
#include "lidebug.h"

#define ENABLE_COARSE_EYE_SEARCH
#ifdef ENABLE_COARSE_EYE_SEARCH

#define PROCESS_RADIUS 1

MInt32 gausskernal[3*3] = 
{
	156,  380, 156,
	380,  925, 380,
	156,  380, 156
};//1024

/*gx = [-1 0 1
        -2 0 2
        -1 0 1];
  gy = gx';*/

JSTATIC MVoid _CreateGradImg_Sobel(const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,
								   MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag, MInt32 *pMaxGrad);
JSTATIC MRESULT _Smooth_U8(MHandle hMemMgr, 
						   MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, 
						   MInt32 *pGaussOperator, MUInt8 *pImgRlt);

#define QUASI_SQRT(x, y, rlt) {MInt32 x1=ABS(x); MInt32 y1=ABS(y); MInt32 mn=MIN(x1,y1); rlt=x1+y1-(mn>>1)-(mn>>2)+(mn>>4);}			

#ifdef OPTIMIZATION_ARM

static	const MUInt32	g_pi32DivTable[256]	= 
{
	0, 1048576, 524288, 349525, 262144, 209715, 174762, 149796,
	131072, 116508, 104857, 95325, 87381, 80659, 74898, 69905,
	65536, 61680, 58254, 55188, 52428, 49932, 47662, 45590,
	43690, 41943, 40329, 38836, 37449, 36157, 34952, 33825,
	32768, 31775, 30840, 29959, 29127, 28339, 27594, 26886,
	26214, 25575, 24966, 24385, 23831, 23301, 22795, 22310,
	21845, 21399, 20971, 20560, 20164, 19784, 19418, 19065,
	18724, 18396, 18078, 17772, 17476, 17189, 16912, 16644,
	16384, 16131, 15887, 15650, 15420, 15196, 14979, 14768,
	14563, 14364, 14169, 13981, 13797, 13617, 13443, 13273,
	13107, 12945, 12787, 12633, 12483, 12336, 12192, 12052,
	11915, 11781, 11650, 11522, 11397, 11275, 11155, 11037,
	10922, 10810, 10699, 10591, 10485, 10381, 10280, 10180,
	10082, 9986, 9892, 9799, 9709, 9619, 9532, 9446,
	9362, 9279, 9198, 9118, 9039, 8962, 8886, 8811,
	8738, 8665, 8594, 8525, 8456, 8388, 8322, 8256,
	8192, 8128, 8065, 8004, 7943, 7884, 7825, 7767,
	7710, 7653, 7598, 7543, 7489, 7436, 7384, 7332,
	7281, 7231, 7182, 7133, 7084, 7037, 6990, 6944,
	6898, 6853, 6808, 6765, 6721, 6678, 6636, 6594,
	6553, 6512, 6472, 6432, 6393, 6355, 6316, 6278,
	6241, 6204, 6168, 6132, 6096, 6061, 6026, 5991,
	5957, 5924, 5890, 5857, 5825, 5793, 5761, 5729,
	5698, 5667, 5637, 5607, 5577, 5548, 5518, 5489,
	5461, 5433, 5405, 5377, 5349, 5322, 5295, 5269,
	5242, 5216, 5190, 5165, 5140, 5115, 5090, 5065,
	5041, 5017, 4993, 4969, 4946, 4922, 4899, 4877,
	4854, 4832, 4809, 4788, 4766, 4744, 4723, 4702,
	4681, 4660, 4639, 4619, 4599, 4578, 4559, 4539,
	4519, 4500, 4481, 4462, 4443, 4424, 4405, 4387,
	4369, 4350, 4332, 4315, 4297, 4279, 4262, 4245,
	4228, 4211, 4194, 4177, 4161, 4144, 4128, 4112,
};

#define	SQRT_STEP(n)								\
	t2			= (0x40000000l >> (n)) + Estimate;	\
	Estimate	= Estimate >> 1;					\
	if(t2 <= t1)									\
	{												\
		t1			-= t2;							\
		Estimate	|= (0x40000000l >> (n));		\
	}

// unsigned long ARER_RootLong(unsigned long t1)
// {
// 	MUInt32 t2, Estimate = 0;
// 	SQRT_STEP(0)
// 	SQRT_STEP(2)
// 	SQRT_STEP(4)
// 	SQRT_STEP(6)
// 	SQRT_STEP(8)
// 	SQRT_STEP(10)
// 	SQRT_STEP(12)
// 	SQRT_STEP(14)
// 	SQRT_STEP(16)
// 	SQRT_STEP(18)
// 	SQRT_STEP(20)
// 	SQRT_STEP(22)
// 	SQRT_STEP(24)
// 	SQRT_STEP(26)
// 	SQRT_STEP(28)
// 	SQRT_STEP(30)
// 	//if(Estimate < t1)
// 	//	Estimate++;
// 	return Estimate;
// }

JSTATIC MVoid _Smooth_I32_Div4(MInt32 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, 
							   MUInt8 *pImgRlt)
{
	MInt32 x;
	pImgSrc	+= lImgLine*PROCESS_RADIUS + PROCESS_RADIUS;
	pImgRlt	+= lImgLine*PROCESS_RADIUS + PROCESS_RADIUS;
	lWidth	-= PROCESS_RADIUS*2;
	lHeight	-= PROCESS_RADIUS*2;
	for (; lHeight>0; lHeight--)
	{
		for (x=lWidth; x>0; x--)
		{
			MInt32 lSum = 0;

			lSum	= (pImgSrc[-1-lImgLine] + pImgSrc[1-lImgLine] + pImgSrc[-1+lImgLine] + pImgSrc[1+lImgLine]) * 156;
			lSum	+= (pImgSrc[0-lImgLine] + pImgSrc[-1] + pImgSrc[1] + pImgSrc[0+lImgLine]) * 380;
			lSum	+= pImgSrc[0]*925;
			pImgSrc	++;
			lSum	= DIV_ROUND(lSum, 3*4*1024);
			//lSum	= DIV_ROUND(lSum, 4);
			//JASSERT(lVal<256);
			*pImgRlt++ = (MUInt8)TRIM_UINT8(lSum);

		}
		pImgSrc	+= lImgLine - lWidth;
		pImgRlt	+= lImgLine - lWidth;
	}
}

JSTATIC MVoid RadialSymTransEx_1(MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, MUInt8 *pO, MInt32 *pM,
								 MInt32 Beta, MInt32 r0, MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag)
{
	MInt32	x, y;
	const MUInt32*	pi32DivTable	= g_pi32DivTable;

#define	SCALE	(1<<20)
	//for(y=1; y<256; y++)
	//{
	//	printf(" %ld,", (SCALE/y));
	//	if((y&7) == 7)
	//		printf("\r\n", (SCALE/y));
	//}

	lHeight	-= 1;
	lWidth	-= 1;
	for(y=1; y<lHeight; y++)
	{
		pMag	+=	lImgLine;
		pImgX	+=	lImgLine;
		pImgY	+=	lImgLine;
		for(x=1; x<lWidth; x++)
		{
			MInt32 PosX, PosY;
			MInt32 lMagData = pMag[x];
			if(lMagData < Beta)
				continue;

			if(lMagData<256)
			{
				MInt32 t1, t2, t4;
				t1	= pImgX[x];
				t2	= pImgY[x];
				t4	= pi32DivTable[lMagData];
				t1	= t1*r0+(lMagData>>1);
				t2	= t2*r0+(lMagData>>1);
				t1	*= t4;
				t2	*= t4;
				if(t1>=0)
					t1	= (t1 + (t4>>1));
				else
					t1	= (t1 - (t4>>1));
				if(t2>=0)
					t2	= (t2 + (t4>>1));
				else
					t2	= (t2 - (t4>>1));
				PosX= x + t1/SCALE;
				PosY= y + t2/SCALE;

				//if(PosX != x + DIV_ROUND(pImgX[x]*r0, lMagData))
				//	PosX = PosX;
				//if(PosY != y + DIV_ROUND(pImgY[x]*r0, lMagData))
				//	PosX = PosX;
			}
			else
			{
				// Positive direction
				PosX = x + DIV_ROUND(pImgX[x]*r0, lMagData);
				PosY = y + DIV_ROUND(pImgY[x]*r0, lMagData);
			}

			// Clamp coordinate values to range [1 rows 1 cols]
			if(PosX<0 || PosX>lWidth || PosY<0 || PosY>lHeight)
				continue;
			
			pO[PosY*lImgLine+PosX] ++;
			pM[PosY*lImgLine+PosX] += lMagData;
		}
	}
}


JSTATIC MVoid RadialSymTransEx_2(MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, 
								 MUInt8 *pO, MInt32 *pM, MInt32 *pS)
{
	MInt32	x, y;
	MUInt8 *pOCur;
	MInt32 *pMCur;
	MInt32 *pSCur;

	pOCur = pO + lImgLine + 1;
	pMCur = pM + lImgLine + 1;
	pSCur = pS + lImgLine + 1;
	lWidth		-= 2;
	lImgLine	-= lWidth;
	// Unsmoothed symmetry measure at this radius value
	for(y=lHeight-2; y>0; y--)
	{		
		for(x=lWidth; x>0; x--)
		{
			MInt32 lOVal = *pOCur++;
			MInt32 lMVal = *pMCur++;
			MInt32 lSVal = pSCur[0];
			if(lOVal >= 10)
				lSVal += DIV_ROUND(lMVal, 10);
			else
			{
				lSVal += DIV_ROUND(lOVal*lOVal*lMVal, 1000);
			}
			*pSCur++	= lSVal;
		}
		pOCur	+= lImgLine;
		pMCur	+= lImgLine;
		pSCur	+= lImgLine;
	}
}

JSTATIC MRESULT RadialSymTransEx(MHandle hMemMgr, MUInt8 *pImgSrc, 
								 MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,
								 MUInt8 *pImgRlt, MBool bPosDirection)
{
	MInt32 Beta		= 5;

	MRESULT res = LI_ERR_NONE;
	MInt32 r;

	MInt16 *pImgX = MNull, *pImgY = MNull;
	MUInt16 *pMag = MNull;
	MInt32 lMaxGrad = 0;
	MUInt8 *pO = pImgRlt;
	MInt32 *pM = MNull, *pS = MNull;
	
	JASSERT(pImgSrc!=MNull && pO!=MNull);
	JASSERT(2>=1 && 3>=1 && 9>3);
	JASSERT(Beta>=0);//0~100

	AllocVectMem(hMemMgr, pImgX, lHeight*lImgLine, MInt16);
	AllocVectMem(hMemMgr, pImgY, lHeight*lImgLine, MInt16);
	AllocVectMem(hMemMgr, pMag, lHeight*lImgLine, MUInt16);
	AllocVectMem(hMemMgr, pM, lHeight*lImgLine, MInt32);
	AllocVectMem(hMemMgr, pS, lHeight*lImgLine, MInt32);
	SetVectZero(pS, lHeight*lImgLine*sizeof(MInt32));

	// Smooth source img
	GO(_Smooth_U8(hMemMgr, pImgSrc, lImgLine, lWidth, lHeight, gausskernal, pImgSrc));
//	PrintBmp(pImgSrc, lImgLine, DATA_U8, lWidth, lHeight, 1);

	// Use the Sobel masks to get gradients in x and y
	_CreateGradImg_Sobel(pImgSrc, lImgLine, lWidth, lHeight, pImgX, pImgY, pMag, &lMaxGrad);
// 	PrintBmp(pMag, lImgLine, DATA_U16, lWidth, lHeight, 1);
// 	PrintWordChannel(pMag, lImgLine, lWidth, lHeight, 1, 0);
	Beta = DIV_ROUND(lMaxGrad*Beta, 100);

	for(r=3; r<=9; r+=2)
	{
		SetVectZero(pO, lHeight*lImgLine*sizeof(MUInt8));
		SetVectZero(pM, lHeight*lImgLine*sizeof(MInt32));

		RadialSymTransEx_1(lImgLine, lWidth, lHeight, pO, pM, Beta, bPosDirection?r:-r, pImgX, pImgY, pMag);

		RadialSymTransEx_2(lImgLine, lWidth, lHeight, pO, pM, pS);
	}

	_Smooth_I32_Div4(pS, lImgLine, lWidth, lHeight, pO);

EXT:
	FreeVectMem(hMemMgr, pImgX);
	FreeVectMem(hMemMgr, pImgY);
	FreeVectMem(hMemMgr, pMag);
	FreeVectMem(hMemMgr, pM);
	FreeVectMem(hMemMgr, pS);
	return res;
}

MRESULT RadialSymTrans(MHandle hMemMgr, MUInt8 *pImgSrc, 
						MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,
						MInt32 Alpha, MInt32 Beta, MInt32 lMinR, MInt32 lMaxR, 
						MUInt8 *pImgRlt, MBool bPosDirection)
{
	return RadialSymTransEx(hMemMgr, pImgSrc, lImgLine, lWidth, lHeight, pImgRlt, bPosDirection);
}

MRESULT _Smooth_U8(MHandle hMemMgr, 
				   MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, 
				   MInt32 *pGaussOperator, MUInt8 *pImgRlt)
{
	MRESULT res = LI_ERR_NONE;
	MInt32 x;
	MUInt8 *pImgCpy = MNull;
	MUInt8 *pImgBack = MNull;
	AllocVectMem(hMemMgr, pImgCpy, lHeight*lImgLine, MUInt8);
	CpyVectMem(pImgCpy, pImgSrc, lHeight*lImgLine, MUInt8);
	pImgBack	= pImgCpy;

	pImgCpy	+= lImgLine*PROCESS_RADIUS + PROCESS_RADIUS;
	pImgRlt	+= lImgLine*PROCESS_RADIUS + PROCESS_RADIUS;
	lWidth	-= PROCESS_RADIUS*2;
	lHeight	-= PROCESS_RADIUS*2;
	for (; lHeight>0; lHeight--)
	{
		for (x=lWidth; x>0; x--)
		{
			MInt32 lSum = 0;

			lSum	= (pImgCpy[-1-lImgLine] + pImgCpy[1-lImgLine] + pImgCpy[-1+lImgLine] + pImgCpy[1+lImgLine]) * 156;
			lSum	+= (pImgCpy[0-lImgLine] + pImgCpy[-1] + pImgCpy[1] + pImgCpy[0+lImgLine]) * 380;
			lSum	+= pImgCpy[0]*925;
			pImgCpy	++;
			lSum	= DIV_ROUND(lSum, 3*1024);
			*pImgRlt++ = (MUInt8)TRIM_UINT8(lSum);
		}
		pImgCpy	+= lImgLine - lWidth;
		pImgRlt	+= lImgLine - lWidth;
	}
EXT:
	FreeVectMem(hMemMgr, pImgBack);
	return res;
}

#else// OPTIMIZATION_ARM

JSTATIC MVoid _Smooth_I32(MInt32 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, 
						  MInt32 *pGaussOperator, MInt32 *pImgRlt);

MRESULT RadialSymTrans(MHandle hMemMgr, MUInt8 *pImgSrc, 
						MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,
						MInt32 Alpha, MInt32 Beta, MInt32 lMinR, MInt32 lMaxR, 
						MUInt8 *pImgRlt, MBool bPosDirection)
{
	MRESULT res = LI_ERR_NONE;
	MInt32 x, y, r;

	MInt16 *pImgX = MNull, *pImgY = MNull;
	MUInt16 *pMag = MNull;
	MInt32 lMaxGrad = 0;
	MUInt8 *pO = pImgRlt;
	MInt32 *pM = MNull, *pS = MNull;

	MInt32 lRLen = (lMaxR - lMinR)/2 + 1;
	
	JASSERT(pImgSrc!=MNull && pImgRlt!=MNull);
	JASSERT(Alpha>=1 && lMinR>=1 && lMaxR>lMinR);
	JASSERT(Beta>=0);//0~100

	AllocVectMem(hMemMgr, pImgX, lHeight*lImgLine, MInt16);
	AllocVectMem(hMemMgr, pImgY, lHeight*lImgLine, MInt16);
	AllocVectMem(hMemMgr, pMag, lHeight*lImgLine, MUInt16);
	AllocVectMem(hMemMgr, pM, lHeight*lImgLine, MInt32);
	AllocVectMem(hMemMgr, pS, lHeight*lImgLine, MInt32);
	SetVectZero(pS, lHeight*lImgLine*sizeof(MInt32));

	// Smooth source img
	GO(_Smooth_U8(hMemMgr, pImgSrc, lImgLine, lWidth, lHeight, gausskernal, pImgSrc));

	// Use the Sobel masks to get gradients in x and y
	_CreateGradImg_Sobel(pImgSrc, lImgLine, lWidth, lHeight, pImgX, pImgY, pMag, &lMaxGrad);
	Beta = DIV_ROUND(lMaxGrad*Beta, 100);

	for(r=lMinR; r<=lMaxR; r+=2)
	{
		MInt32 kappa = (r==1) ? 8 : 10;
		MInt32 r0 = (bPosDirection) ? r : -r;

		SetVectZero(pO, lHeight*lImgLine*sizeof(MUInt8));
		SetVectZero(pM, lHeight*lImgLine*sizeof(MInt32));
		
		for(y=PROCESS_RADIUS; y<lHeight-PROCESS_RADIUS; y++)
		{
			MUInt16 *pMagCur = pMag + y*lImgLine;
			MInt16 *pImgXCur = pImgX + y*lImgLine;
			MInt16 *pImgYCur = pImgY + y*lImgLine;
			for(x=PROCESS_RADIUS; x<lWidth-PROCESS_RADIUS; x++)
			{
				MInt32 PosX, PosY;
				MInt32 lMagData = pMagCur[x];
				if(lMagData < Beta)
					continue;

				// Positive direction
				PosX = x + DIV_ROUND(pImgXCur[x]*r0, pMagCur[x]);
				PosY = y + DIV_ROUND(pImgYCur[x]*r0, pMagCur[x]);

				// Clamp coordinate values to range [1 rows 1 cols]
				if(PosX<0 || PosX>lWidth-1 || PosY<0 || PosY>lHeight-1)
					continue;
				
				pO[PosY*lImgLine+PosX] ++;
				pM[PosY*lImgLine+PosX] += pMagCur[x];
			}
		}

		// Unsmoothed symmetry measure at this radius value
		for(y=PROCESS_RADIUS; y<lHeight-PROCESS_RADIUS; y++)
		{		
			MUInt8 *pOCur = pO + y*lImgLine;
			MInt32 *pMCur = pM + y*lImgLine;
			MInt32 *pSCur = pS + y*lImgLine;
			for(x=PROCESS_RADIUS; x<lWidth-PROCESS_RADIUS; x++)
			{
				MInt32 lOVal = pOCur[x];
				if(lOVal > kappa)
					pSCur[x] += DIV_ROUND(pMCur[x], kappa);
				else
				{
					MInt32 tmp1 = 1, tmp2 = kappa; 
					MInt32 Alpha1 = Alpha;
					do 
					{
						tmp1 *= lOVal;
						tmp2 *= kappa;
					} while(--Alpha1!=0);
					pSCur[x] += DIV_ROUND(tmp1*pMCur[x], tmp2);
				}
			}
		}
	}
	_Smooth_I32(pS, lImgLine, lWidth, lHeight, gausskernal, pM);

	for(y=PROCESS_RADIUS; y<lHeight-PROCESS_RADIUS; y++)
	{
		MInt32 *pMCur = pM + y*lImgLine;
		MUInt8 *pImgRltCur = pImgRlt + y*lImgLine;
		for(x=PROCESS_RADIUS; x<lWidth-PROCESS_RADIUS; x++)
		{
			MInt32 lVal = DIV_ROUND(pMCur[x], lRLen);
			//JASSERT(lVal<256);
			//pImgRltCur[x] = (MUInt8)TRIM_UINT8(lVal);
			//临时性地替换成128+
			pImgRltCur[x] = (MUInt8)TRIM_UINT8(lVal+128);
		}
	}

EXT:
	FreeVectMem(hMemMgr, pImgX);
	FreeVectMem(hMemMgr, pImgY);
	FreeVectMem(hMemMgr, pMag);
	FreeVectMem(hMemMgr, pM);
	FreeVectMem(hMemMgr, pS);
	return res;
}

MRESULT _Smooth_U8(MHandle hMemMgr, 
				   MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, 
				   MInt32 *pGaussOperator, MUInt8 *pImgRlt)
{
	MRESULT res = LI_ERR_NONE;
	MInt32 x, y;
	MUInt8 *pImgCpy = MNull;
	AllocVectMem(hMemMgr, pImgCpy, lHeight*lImgLine, MUInt8);
	CpyVectMem(pImgCpy, pImgSrc, lHeight*lImgLine, MUInt8);

	for (y=PROCESS_RADIUS; y<lHeight-PROCESS_RADIUS; y++)
	{
		for (x=PROCESS_RADIUS; x<lWidth-PROCESS_RADIUS; x++)
		{
			MInt32 i, j;
			MInt32 lSum = 0;
			MInt32 *pGauss = pGaussOperator;
			MInt32 lTmp;
			for (j=-1;j<=1; j++)
			{
				for (i=-1; i<=1; i++)
					lSum += pImgCpy[(y+j)*lImgLine + (x+i)]*(*pGauss++);
			}
			lTmp = DIV_ROUND(lSum, 3*1024);
			pImgRlt[y*lImgLine + x] = (MUInt8)TRIM_UINT8(lTmp);
		}
	}
EXT:
	FreeVectMem(hMemMgr, pImgCpy);
	return res;
}

MVoid _Smooth_I32(MInt32 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight, 
				  MInt32 *pGaussOperator, MInt32 *pImgRlt)
{
	MInt32 x, y;
	for (y=PROCESS_RADIUS; y<lHeight-PROCESS_RADIUS; y++)
	{
		for (x=PROCESS_RADIUS; x<lWidth-PROCESS_RADIUS; x++)
		{
			MInt32 i, j;
			MInt32 lSum = 0;
			MInt32 *pGauss = pGaussOperator;
			for (j=-1;j<=1; j++)
			{
				for (i=-1; i<=1; i++)
					lSum += pImgSrc[(y+j)*lImgLine + (x+i)]*(*pGauss++);
			}
			pImgRlt[y*lImgLine + x] = DIV_ROUND(lSum, 3*1024);
		}
	}
}

#endif

MVoid _CreateGradImg_Sobel(const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,
						   MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag, MInt32 *pMaxGrad)
{
	MInt32 x, y;
	MInt32 lMaxGrad = 0;
	for (y=PROCESS_RADIUS; y<lHeight-PROCESS_RADIUS; y++)
	{
		const MUInt8 *pImgCur = pImgSrc + y*lImgLine;
		const MUInt8 *pImgCur1 = pImgSrc + (y-1)*lImgLine;
		const MUInt8 *pImgCur2 = pImgSrc + (y+1)*lImgLine;
		MInt16 *pImgXCur = pImgX + y*lImgLine;
		MInt16 *pImgYCur = pImgY + y*lImgLine;
		MUInt16 *pMagCur = pMag + y*lImgLine;
		for (x=PROCESS_RADIUS; x<lWidth-PROCESS_RADIUS; x++)
		{
			MInt32 lMag;
			MInt32 lGradX = (pImgCur1[x+1] + (pImgCur[x+1]<<1) + pImgCur2[x+1]) - 
							(pImgCur1[x-1] + (pImgCur[x-1]<<1) + pImgCur2[x-1]);
			MInt32 lGradY = (pImgCur2[x-1] + (pImgCur2[x]<<1) + pImgCur2[x+1]) - 
							(pImgCur1[x-1] + (pImgCur1[x]<<1) + pImgCur1[x+1]);
			//lMag = (MInt32)(sqrt((MDouble)(lGradX*lGradX + lGradY*lGradY)) + 0.5);
			QUASI_SQRT(lGradX, lGradY, lMag);
			//lMag	= ARER_RootLong(lGradX*lGradX+lGradY*lGradY);
			pImgXCur[x] = (MInt16)lGradX;
			pImgYCur[x] = (MInt16)lGradY;
			pMagCur[x] = (MUInt16)lMag;
			if(lMag>lMaxGrad)
				lMaxGrad = lMag;
		}
	}

	*pMaxGrad = lMaxGrad;
}

#endif// ENABLE_COARSE_EYE_SEARCH