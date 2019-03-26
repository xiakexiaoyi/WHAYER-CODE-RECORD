#include "limath.h"
#define PROCESS_RADIUS 1

#define QUASI_SQRT(x, y, rlt) {MInt32 x1=ABS(x); MInt32 y1=ABS(y); MInt32 mn=MIN(x1,y1); rlt=x1+y1-(mn>>1)-(mn>>2)+(mn>>4);}
MLong GetAngle(MInt32 lY,MInt32 lX)
{
    MInt32 TanA[]={9,26,44,61,79,96,114,132,150,167,185,204,222,240,259,278,297,316,335,354,374,394,414,435,456,477,499,521,543,566,589,613,637,662,688,714,741,768,796,825,854,885,917,950,983,1018,1054,1092,1131,1171,1214,1258,1304,1352,1402,1456,1512,1570,1632,1698,1768,1843,1922,2007,2098,2196,2301,2416,2540,2676,2826,2991,3175,3379,3610,3872,4171,4518,4925,5408,5993,6715,7630,8829,10472,12866,16691,23859,42963};

    MLong i;

    MInt32 tanA;
    MLong offset=0;
    MLong flag=1;
    if (lX>=0&&lY<0)
    {
        offset=360;
        flag=-1;
    }
    if (lX<0&&lY>=0);
    {
        offset=180;
        flag=-1;
    }
    if (lX<0&&lY<0)
    {
        flag=1;
        offset=180;
    }
    tanA=ABS(lY)*1000*1000/(ABS(lX)*1000+1);//
    for (i=0;i<90;i++)
    {
        if (tanA<TanA[i])
        {
            return flag*i+offset;
        }
    }
    return offset+flag*i;	
}
MVoid CreateGradAngleImg(const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag, MUInt16 *pAngle)
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
		MUInt16 *pAngleCur = pAngle + y*lImgLine;
		for (x=PROCESS_RADIUS; x<lWidth-PROCESS_RADIUS; x++)
		{
			MInt32 lMag;
			MLong lAngle;
			MInt32 lGradX = (pImgCur1[x+1] + (pImgCur[x+1]<<1) + pImgCur2[x+1]) - 
				(pImgCur1[x-1] + (pImgCur[x-1]<<1) + pImgCur2[x-1]);
			MInt32 lGradY = (pImgCur2[x-1] + (pImgCur2[x]<<1) + pImgCur2[x+1]) - 
				(pImgCur1[x-1] + (pImgCur1[x]<<1) + pImgCur1[x+1]);
			//fMag = (sqrt((MDouble)(lGradX*lGradX + lGradY*lGradY)) );
			QUASI_SQRT(lGradX,lGradY,lMag);

			//lAngle = (MUInt16)(atan2((MDouble)lGradY,(MDouble)lGradX));
			lAngle=GetAngle(lGradY,lGradX);
			//QUASI_SQRT(lGradX, lGradY, lMag);
			//lMag	= ARER_RootLong(lGradX*lGradX+lGradY*lGradY);
			pImgXCur[x] = (MInt16)lGradX;
			pImgYCur[x] = (MInt16)lGradY;
			pMagCur[x] =(MUInt16) lMag;
			//pAngleCur[x] = (MUInt16)(((MUInt16)((lAngle+2*V_PI)*180/V_PI))%360);	
			pAngleCur[x]=(MUInt16)lAngle;
		}
	}
}
//TanA={9	26	44	61	79	96	114	132	150	167	185	204	222	240	259	278	297	316	335	354	374	394	414	435	456	477	499	521	543	566	589	613	637	662	688	714	741	768	796	825	854	885	917	950	983	1018	1054	1092	1131	1171	1214	1258	1304	1352	1402	1456	1512	1570	1632	1698	1768	1843	1922	2007	2098	2196	2301	2416	2540	2676	2826	2991	3175	3379	3610	3872	4171	4518	4925	5408	5993	6715	7630	8829	10472	12866	16691	23859	42963}


MLong GetSin(MLong angle)
{
	MLong sinA[]={0,17,35,52,70,87,105,122,139,156,174,191,208,225,242,259,276,292,309,326,342,358,375,391,407,423,438,454,469,485,500,515,530,545,559,574,588,602,616,629,643,656,669,682,695,707,719,731,743,755,766,777,788,799,809,819,829,839,848,857,866,875,883,891,899,906,914,921,927,934,940,946,951,956,961,966,970,974,978,982,985,988,990,993,995,996,998,999,999,1000,1000};
	MLong flag=1;
	if (angle<0)
	{
		flag=-1;
	}
	angle=ABS(angle);
	angle=angle%360;
	if (angle<=90)
	{
		return flag*sinA[angle];
	}
	if (angle<=180)
	{
		return flag*sinA[180-angle];
	}
	if (angle<=270)
	{
		return flag*(-1)*sinA[angle-180];
	}
	return flag*(-1)*sinA[360-angle];
}

MLong GetCosA(MLong angle)
{
	MLong cosA[]={1000,1000,999,999,998,996,995,993,990,988,985,982,978,974,970,966,961,956,951,946,940,934,927,921,914,906,899,891,883,875,866,857,848,839,829,819,809,799,788,777,766,755,743,731,719,707,695,682,669,656,643,629,616,602,588,574,559,545,530,515,500,485,469,454,438,423,407,391,375,358,342,326,309,292,276,259,242,225,208,191,174,156,139,122,105,87,70,52,35,17,0};
	MLong flag=1;
	angle=ABS(angle);
	angle=angle%360;
	if (angle<=90)
	{
		return cosA[angle];
	}
	if (angle<=180)
	{
		return -cosA[180-angle];
	}
	if (angle<=270)
	{
		return -cosA[angle-180];
	}
	return cosA[360-angle];

}