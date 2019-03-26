
#include "liyuv_hue.h"
#include "lidebug.h"

#define	ZOOM_SHIFT			15
#define COLOR_OFFSET		(1<<7)

#define	YUV_RGB_R 			45941	//	(1-0.299)/0.5	= 1.402
#define	YUV_RGB_U			-11277	// -0.114/((0.5/(1-0.114))*0.587) = -0.344
#define	YUV_RGB_V			-23401	// -0.299/((0.5/(1-0.299))*0.587) = -0.714
//#define	YUV_RGB_B 			58065	//	(1-0.114)/0.5	= 1.772

#define _YUV2RG_L(Y, U, V, lHalf, lR, lG)   								\
{																			\
	MLong _lY = Y, _lCb = (U)-(lHalf), _lCr = (V)-(lHalf);					\
	lG = (_lY<<ZOOM_SHIFT) + YUV_RGB_U*_lCb + YUV_RGB_V*_lCr;				\
	lR = (_lY<<ZOOM_SHIFT) + YUV_RGB_R*_lCr;								\
}

#define _YUV2RG(Y, U, V, R, G)     											\
{																			\
	MLong _g, _r;											   			    \
	_YUV2RG_L(Y, U, V, COLOR_OFFSET,_r, _g);    							\
	_r = DOWN_ROUND(_r, ZOOM_SHIFT);										\
	R = TRIM_UINT8(_r);														\
	_g = DOWN_ROUND(_g, ZOOM_SHIFT);										\
	G = TRIM_UINT8(_g);														\
}

MVoid YUV2Hue(MLong Y, MLong U, MLong V, MUInt8 *pHue)
{
    MLong R, G;
    _YUV2RG(Y, U, V, R, G);
    if (R<=G || R==0)
        *pHue = 255;
    else
        *pHue = (MUInt8)((G*256)/R);
}

//#define	_OPTIMIZE_TEST_
#if	defined(_OPTIMIZE_TEST_)

static	const MUInt32	g_pi32DivTable[256]	= 
{
	0, 33554432, 16777216, 11184810, 8388608, 6710886, 5592405, 4793490,
	4194304, 3728270, 3355443, 3050402, 2796202, 2581110, 2396745, 2236962,
	2097152, 1973790, 1864135, 1766022, 1677721, 1597830, 1525201, 1458888,
	1398101, 1342177, 1290555, 1242756, 1198372, 1157049, 1118481, 1082401,
	1048576, 1016800, 986895, 958698, 932067, 906876, 883011, 860370,
	838860, 818400, 798915, 780335, 762600, 745654, 729444, 713924,
	699050, 684784, 671088, 657930, 645277, 633102, 621378, 610080,
	599186, 588674, 578524, 568719, 559240, 550072, 541200, 532610,
	524288, 516222, 508400, 500812, 493447, 486296, 479349, 472597,
	466033, 459649, 453438, 447392, 441505, 435771, 430185, 424739,
	419430, 414252, 409200, 404270, 399457, 394758, 390167, 385683,
	381300, 377016, 372827, 368730, 364722, 360800, 356962, 353204,
	349525, 345921, 342392, 338933, 335544, 332222, 328965, 325771,
	322638, 319566, 316551, 313592, 310689, 307838, 305040, 302292,
	299593, 296941, 294337, 291777, 289262, 286790, 284359, 281970,
	279620, 277309, 275036, 272800, 270600, 268435, 266305, 264208,
	262144, 260111, 258111, 256140, 254200, 252288, 250406, 248551,
	246723, 244922, 243148, 241398, 239674, 237974, 236298, 234646,
	233016, 231409, 229824, 228261, 226719, 225197, 223696, 222214,
	220752, 219310, 217885, 216480, 215092, 213722, 212369, 211034,
	209715, 208412, 207126, 205855, 204600, 203360, 202135, 200924,
	199728, 198546, 197379, 196224, 195083, 193956, 192841, 191739,
	190650, 189573, 188508, 187454, 186413, 185383, 184365, 183357,
	182361, 181375, 180400, 179435, 178481, 177536, 176602, 175677,
	174762, 173857, 172960, 172074, 171196, 170327, 169466, 168615,
	167772, 166937, 166111, 165292, 164482, 163680, 162885, 162098,
	161319, 160547, 159783, 159025, 158275, 157532, 156796, 156067,
	155344, 154628, 153919, 153216, 152520, 151830, 151146, 150468,
	149796, 149130, 148470, 147816, 147168, 146525, 145888, 145257,
	144631, 144010, 143395, 142784, 142179, 141579, 140985, 140395,
	139810, 139230, 138654, 138084, 137518, 136956, 136400, 135847,
	135300, 134756, 134217, 133682, 133152, 132626, 132104, 131586
};

MVoid YUYVIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				  MInt32 lWidth, MInt32 lHeight)
{
	MInt32 x, y;
	MUInt8 *pYUVCur = YUV,
        *pHueCur   = Hue;
	MInt32 lWidth2 = FLOOR_2(lWidth),
        lYUVExt = lYUVLine - lWidth2*2,
        lHueExt = lHueLine - lWidth;
	const MUInt32*	pi32DivTable	= g_pi32DivTable;

	JASSERT((MInt32)YUV % 4 ==0);
	JASSERT((MInt32)Hue % 4 ==0);
	//for(x=0; x<256; x++)
	//{
	//	for(y=1; y<256; y++)
	//	{
	//		if(x*256/y != (x*256*g_i32DivTable[y]+g_i32DivTable[y]/2)>>25)
	//			x = x;
	//	}
	//}
	//for(y=1; y<256; y++)
	//{
	//	prMLongf(" %ld,", (65536*512/y));
	//	if((y&7) == 7)
	//		prMLongf("\r\n", (65536*512/y));
	//}

    for(y=lHeight; y>0; y--, pYUVCur+=lYUVExt, pHueCur+=lHueExt)
	{
		for(x=lWidth2; x>0; x-=2, pYUVCur+=4, pHueCur+=2)
		{
			MLong R, G, _g, _r;
			MInt32 lY1 = pYUVCur[0], lY2 = pYUVCur[2];
			MInt32 lCb = pYUVCur[1], lCr = pYUVCur[3];

			lCb	= lCb-(COLOR_OFFSET);
			lCr	= lCr-(COLOR_OFFSET);
			_g	= YUV_RGB_U*lCb + YUV_RGB_V*lCr;
			_r	= YUV_RGB_R*lCr;
			G	= (lY1<<ZOOM_SHIFT) + _g;
			R	= (lY1<<ZOOM_SHIFT) + _r;
			G	= DOWN_ROUND(G, ZOOM_SHIFT);
			G	= TRIM_UINT8(G);
			R	= DOWN_ROUND(R, ZOOM_SHIFT);
			R	= TRIM_UINT8(R);
			if (R<=G )
				pHueCur[0] = 255;
			else
				pHueCur[0] = (MUInt8)((G*pi32DivTable[R]+(pi32DivTable[R]>>9))>>17);

			G	= (lY2<<ZOOM_SHIFT) + _g;
			R	= (lY2<<ZOOM_SHIFT) + _r;
			G	= DOWN_ROUND(G, ZOOM_SHIFT);
			G	= TRIM_UINT8(G);
			R	= DOWN_ROUND(R, ZOOM_SHIFT);
			R	= TRIM_UINT8(R);
			if (R<=G)
				pHueCur[1] = 255;
			else
				pHueCur[1] = (MUInt8)((G*pi32DivTable[R]+(pi32DivTable[R]>>9))>>17);
		}
	}
}
MVoid YUVIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				 MInt32 lWidth, MInt32 lHeight)
{
	MInt32 x, y;
	MUInt8 *pYUVCur = YUV,
        *pHueCur   = Hue;
	MInt32 lYUVExt = lYUVLine - lWidth*3,
        lHueExt = lHueLine - lWidth;
	const MUInt32*	pi32DivTable	= g_pi32DivTable;

	JASSERT((MInt32)YUV % 4 ==0);
	JASSERT((MInt32)Hue % 4 ==0);

    for(y=lHeight; y>0; y--, pYUVCur+=lYUVExt, pHueCur+=lHueExt)
	{
		for(x=lWidth; x>0; x--, pYUVCur+=3, pHueCur++)
		{
			MLong R, G, _g, _r;
			MInt32 lY1 = pYUVCur[0], lCb = pYUVCur[1], lCr = pYUVCur[2];

			lCb	= lCb-(COLOR_OFFSET);
			lCr	= lCr-(COLOR_OFFSET);
			_g	= YUV_RGB_U*lCb + YUV_RGB_V*lCr;
			_r	= YUV_RGB_R*lCr;
			G	= (lY1<<ZOOM_SHIFT) + _g;
			R	= (lY1<<ZOOM_SHIFT) + _r;
			G	= DOWN_ROUND(G, ZOOM_SHIFT);
			G	= TRIM_UINT8(G);
			R	= DOWN_ROUND(R, ZOOM_SHIFT);
			R	= TRIM_UINT8(R);
			if (R<=G)
				pHueCur[0] = 255;
			else
				pHueCur[0] = (MUInt8)((G*pi32DivTable[R]+(pi32DivTable[R]>>9))>>17);
		}
	}
}
#else
MVoid YUYVIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				  MInt32 lWidth, MInt32 lHeight)
{
	MInt32 x, y;
	MUInt8 *pYUVCur = YUV,
        *pHueCur   = Hue;
	MInt32 lWidth2 = FLOOR_2(lWidth),
        lYUVExt = lYUVLine - lWidth2*2,
        lHueExt = lHueLine - lWidth;
	JASSERT((MInt32)YUV % 4 ==0);
	JASSERT((MInt32)Hue % 4 ==0);

    for(y=0; y<lHeight; y++, 
        pYUVCur+=lYUVExt, pHueCur+=lHueExt)
	{
		for(x=0; x<lWidth2; x+=2, pYUVCur+=4, pHueCur+=2)
		{
			MInt32 lY1 = pYUVCur[0], lY2 = pYUVCur[2];
			MInt32 lCb = pYUVCur[1], lCr = pYUVCur[3];
            YUV2Hue(lY1, lCb, lCr, pHueCur);
            YUV2Hue(lY2, lCb, lCr, pHueCur+1);
		}
	}
}
MVoid YUVIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				 MInt32 lWidth, MInt32 lHeight)
{
	MInt32 x,y;
	MInt32 lYUVExt = lYUVLine - lWidth*3,
		   lHueExt = lHueLine - lWidth;
	MUInt8 *YUVCur = YUV,
		   *HueCur = Hue;

	for(y=0; y<lHeight; y++, 
		YUVCur+=lYUVExt, HueCur+=lHueExt)
	{
		for(x=0; x<lWidth; x++, YUVCur+=3, HueCur++)
		{
			YUV2Hue(YUVCur[0], YUVCur[1], YUVCur[2], HueCur);
		}
	}
}
#endif
MVoid Y1VY0UIMG2Hue(MUInt8 *YUV, MInt32 lYUVLine, MUInt8 *Hue, MInt32 lHueLine,
				    MInt32 lWidth, MInt32 lHeight)
{
  	MInt32 x, y;
	MUInt8 *pYUVCur = YUV,
        *pHueCur   = Hue;
	MInt32 lWidth2 = FLOOR_2(lWidth),
        lYUVExt = lYUVLine - lWidth2*2,
        lHueExt = lHueLine - lWidth;
	JASSERT((MInt32)YUV % 4 ==0);
	JASSERT((MInt32)Hue % 4 ==0);

    for(y=0; y<lHeight; y++, 
        pYUVCur+=lYUVExt, pHueCur+=lHueExt)
	{
		for(x=0; x<lWidth2; x+=2, pYUVCur+=4, pHueCur+=2)
		{
			MInt32 lY1 = pYUVCur[2], lY2 = pYUVCur[0];
			MInt32 lCb = pYUVCur[3], lCr = pYUVCur[1];
            YUV2Hue(lY1, lCb, lCr, pHueCur);
            YUV2Hue(lY2, lCb, lCr, pHueCur+1);
		}
	}
}

MVoid BGRImg2Hue(MUInt8* BGR, MInt32 lBGRLine, MUInt8* Hue, MInt32 lHueLine,
				MInt32 lWidth, MInt32 lHeight)
{
	MLong i, j;
	MLong lSrcExt, lDstExt;
	if (!BGR || !Hue)return ;

	lSrcExt = lBGRLine - 3*lWidth;
	lDstExt = lHueLine - lWidth;
	for (j=0; j<lHeight; j++, BGR+=lSrcExt, Hue+=lDstExt)
	{
		for (i=0; i<lWidth; i++,BGR+=3, Hue++)
		{
			MByte B = BGR[0];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3);
			MByte G = BGR[1];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+1);
			MByte R = BGR[2];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+2);

			MByte maxVal = MAX(B,MAX(G,R));
			MByte minVal = MIN(B,MIN(G,R));
			MDouble H;
			if (R==maxVal)H=(G-B)*1.0/(maxVal-minVal);
			if (G==maxVal)H=2+(B-R)*1.0/(maxVal-minVal);
			if (B==maxVal)H=4+(R-G)*1.0/(maxVal-minVal);

			H = 60*H;
			if (H<0) H = H+360;
			
			*Hue = (MByte)(H*255/360);
		}
	}
}

////由于H图是圆周形的，所以分离的时候不能用一分为二的方法
////使用相对距离比较合适,以红色为最大值(Hue=0)，Hue值在圆周上离红色越远值越小 
MVoid BGRImg2RedLedColor(MByte* BGR, MLong lBGRLine, MByte* RedLedC, MLong lRedLedCLine,
				MLong lWidth, MLong lHeight)
{
	MLong i, j;
	MLong lSrcExt, lDstExt;
	if (!BGR || !RedLedC)return ;

	lSrcExt = lBGRLine - 3*lWidth;
	lDstExt = lRedLedCLine - lWidth;
	for (j=0; j<lHeight; j++, BGR+=lSrcExt, RedLedC+=lDstExt)
	{
		for (i=0; i<lWidth; i++,BGR+=3, RedLedC++)
		{
			MByte B = BGR[0];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3);
			MByte G = BGR[1];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+1);
			MByte R = BGR[2];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+2);

			MByte maxVal = MAX(B,MAX(G,R));
			MByte minVal = MIN(B,MIN(G,R));
			MDouble H=0;

			if (maxVal == minVal)
			{
				///look for local H value
				MLong lSum = 0;
				MLong lNum = 0;
				if (i!=0)
				{
					lSum += (*(RedLedC-1)*2 - *(BGR-3+2));
					lNum++;
				}
				if (j!=0)
				{
					lSum += (*(RedLedC-lRedLedCLine)*2 - *(BGR-lRedLedCLine+2));
					lNum++;
				}
				if (i!=0 && j!=0)
				{
					lSum += (*(RedLedC-lRedLedCLine-1)*2 - *(BGR-lRedLedCLine-3+2));
					lNum++;
				}

				if (lNum==0)
					H=128;
				else
					H = lSum*1.0/lNum;
				*RedLedC = (BGR[2]+H)/2;
			}
			else
			{
				if (R==maxVal)H=(G-B)*1.0/(maxVal-minVal);
				else if (G==maxVal)H=2+(B-R)*1.0/(maxVal-minVal);
				else if (B==maxVal)H=4+(R-G)*1.0/(maxVal-minVal);
				H = 60*H;
				*RedLedC = (BGR[2]+(MByte)((180-FABS(H))*255/180))/2;
			}
		}
	}
}


MVoid BGRImg2LedColor(MByte* BGR, MLong lBGRLine, MByte* RedLedC, MLong lRedLedCLine,
				MLong lWidth, MLong lHeight)
{
	MLong i, j;
	MLong lSrcExt, lDstExt;
	if (!BGR || !RedLedC)return ;
	
	MByte BGR_base[3]={140,0,255};

	lSrcExt = lBGRLine - 3*lWidth;
	lDstExt = lRedLedCLine - lWidth;
	for (j=0; j<lHeight; j++, BGR+=lSrcExt, RedLedC+=lDstExt)
	{
		for (i=0; i<lWidth; i++,BGR+=3, RedLedC++)
		{
			MDouble B = BGR[0]-BGR_base[0];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3);
			MDouble G = BGR[1]-BGR_base[1];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+1);
			MDouble R = BGR[2]-BGR_base[2];//*((MByte*)imgsrc.pixelArray.chunky.pPixel+imgsrc.pixelArray.chunky.dwImgLine*j+i*3+2);
			
			MDouble dist = sqrt(B*B/3.0+G*G/3.0+R*R/3.0);
			if(BGR[2]>240 && BGR[0]<210)
			{
				dist=15;
			}
		    else if(BGR[2]>185 && BGR[0]>185 && BGR[1]>185)
			{
				B=BGR[0]-200;
				G=BGR[1]-200;
				R=BGR[2]-200;
				dist=sqrt(B*B/3.0+G*G/3.0+R*R/3.0);
			}
			*RedLedC=255-dist;
		}
	}

}

