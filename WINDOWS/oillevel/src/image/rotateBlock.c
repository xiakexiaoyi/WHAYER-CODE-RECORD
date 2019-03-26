#include <math.h>
#include <string.h>
#include"rotateBlock.h"
#define ROTATE_PI 3.14159265358979323846
#define DEG2RAD (0.01745329)  // PI/180
#define FLOAT_SCALOR (256)
#define FLOAT_SHIFT (8)

MRESULT RotateImage(MHandle hMemMgr,BLOCK *pSrc,BLOCK *pDst,MLong lAngle)
{
	MLong x;
	MLong y;
	MLong ind;
	MLong x_dst;
	MLong y_dst;
	MLong ind_dst;
	MDouble angle = lAngle  * ROTATE_PI / 180.; // angle in radian  
	MDouble a = sin(angle), b = cos(angle); // sine and cosine of angle 
	MDouble xu,yv;
	MDouble u,v;
	MLong pixel[4];
	MRESULT res = LI_ERR_NONE;

	MLong w_src = pSrc->lWidth;  
	MLong h_src = pSrc->lHeight; 

	MLong w_dst = (MLong)(h_src * fabs(a) + w_src * fabs(b)); 
	MLong h_dst = (MLong)(w_src * fabs(a) + h_src * fabs(b));

	MDouble m[6];
	MUInt8 *pSrcData;
	MUInt8 *pDstData;

	if (pSrc->typeDataA!=DATA_U8)
	{
		res=LI_ERR_IMAGE_FORMAT;
		goto EXT;
	}
	m[0]=b;
	m[1]=-a;
	m[2]=w_src/2-b*w_dst/2+a*h_dst/2;
	m[3]=a;
	m[4]=b;
	m[5]=h_src/2-a*w_dst/2-b*h_dst/2;
	
	

	GO(B_Create(hMemMgr, pDst, DATA_U8, w_dst, h_dst));
	

	pSrcData=(MUInt8*)(pSrc->pBlockData);
	pDstData=(MUInt8*)(pDst->pBlockData);

	memset(pDstData,0,pDst->lBlockLine*pDst->lHeight);

	for (y=0;y<pDst->lHeight;y++)
	{
		for (x=0;x<pDst->lWidth;x++)
		{
			xu=m[0]*x+m[1]*y+m[2];
			yv=m[3]*x+m[4]*y+m[5];
			x_dst=(MLong)(m[0]*x+m[1]*y+m[2]);
			y_dst=(MLong)(m[3]*x+m[4]*y+m[5]);
			u=xu-x_dst;
			v=yv-y_dst;

			ind=y*(pDst->lBlockLine)+x;
			ind_dst=y_dst*(pSrc->lBlockLine)+x_dst;



			if (y_dst<h_src-1&&x_dst<w_src-1&&y_dst>=0&&x_dst>=0)
			{
				pixel[0]=*(pSrcData+ind_dst);                          //image[x,y]
				pixel[1]=*(pSrcData+ind_dst+pSrc->lBlockLine);         //image[x,y+1]
				pixel[2]=*(pSrcData+ind_dst+1);                        //image[x+1,y]
				pixel[3]=*(pSrcData+ind_dst+pSrc->lBlockLine+1);       //image[x+1,y+1]
				*(pDstData+ind)=(MUInt8)((1-u)*(1-v)*pixel[0]+(1-u)*v*pixel[1]+u*(1-v)*pixel[2]+u*v*pixel[3]);
			}
		}
	}	

EXT:
	//FreeVectMem(hMemMgr,pDst);
	return res;
}

MVoid RotateImage_EX(BLOCK *pSrc,BLOCK *pDst,MLong lAngle)
{
	MLong x;
	MLong y;
	MLong ind;
	MLong x_dst;
	MLong y_dst;
	MLong ind_dst;
	MDouble angle = lAngle  * ROTATE_PI / 180.; // angle in radian  
	MFloat a = (MFloat)(sin(angle));
	MFloat b = (MFloat)(cos(angle)); // sine and cosine of angle 
	MFloat xu,yv;
	MFloat u,v;
	MLong pixel[4];
	MRESULT res = LI_ERR_NONE;

	MLong w_src = pSrc->lWidth;  
	MLong h_src = pSrc->lHeight; 
	MFloat temp1 = (MFloat)(fabs(a));
	MFloat temp2 = (MFloat)(fabs(b));
	MFloat temp3 = (MFloat)(h_src * fabs(a) + w_src * fabs(b));
	MFloat temp4 = (MFloat)(h_src * temp1 + w_src * temp2);
	MLong w_dst = (MLong)(h_src * fabs(a) + w_src * fabs(b)); 
	MLong h_dst = (MLong)(w_src * fabs(a) + h_src * fabs(b));

	MDouble m[6];
	MUInt8 *pSrcData;
	MUInt8 *pDstData;

	if (pSrc->typeDataA!=DATA_U8)
	{
		//res=LI_ERR_IMAGE_FORMAT;
		//goto EXT;
		return;
	}
	m[0]=b;
	m[1]=-a;
	m[2]=w_src/2-b*w_dst/2+a*h_dst/2;
	m[3]=a;
	m[4]=b;
	m[5]=h_src/2-a*w_dst/2-b*h_dst/2;


	//GO(B_Create(hMemMgr, pDst, DATA_U8, w_dst, h_dst));
	pDst->lWidth=w_dst;
	pDst->lHeight=h_dst;
	pDst->lBlockLine=w_dst;
	//pDst->lBlockLine=(pDst->lWidth+3)&(~3);
	//pDst->typeDataA=DATA_U8;
	//pDst->pBlockData=pWorkBuf;


	pSrcData=(MUInt8*)(pSrc->pBlockData);
	pDstData=(MUInt8*)(pDst->pBlockData);

	memset(pDstData,0,pDst->lBlockLine*pDst->lHeight);

	for (y=0;y<pDst->lHeight;y++)
	{
		for (x=0;x<pDst->lWidth;x++)
		{
			xu = (MFloat)(m[0]*x+m[1]*y+m[2]);
			yv = (MFloat)(m[3]*x+m[4]*y+m[5]);
			x_dst=(MLong)(m[0]*x+m[1]*y+m[2]);
			y_dst=(MLong)(m[3]*x+m[4]*y+m[5]);
			u=xu-x_dst;
			v=yv-y_dst;

			ind=y*(pDst->lBlockLine)+x;
			ind_dst=y_dst*(pSrc->lBlockLine)+x_dst;



			if (y_dst<h_src-1&&x_dst<w_src-1&&y_dst>=0&&x_dst>=0)
			{
				pixel[0]=*(pSrcData+ind_dst);                          //image[x,y]
				pixel[1]=*(pSrcData+ind_dst+pSrc->lBlockLine);         //image[x,y+1]
				pixel[2]=*(pSrcData+ind_dst+1);                        //image[x+1,y]
				pixel[3]=*(pSrcData+ind_dst+pSrc->lBlockLine+1);       //image[x+1,y+1]
				*(pDstData+ind) = (MUInt8)((1-u)*(1-v)*pixel[0]+(1-u)*v*pixel[1]+u*(1-v)*pixel[2]+u*v*pixel[3]);
			}
		}
	}	
	//FreeVectMem(hMemMgr,pDst);
	//return res;
}

MVoid RotateImage_xsd(BLOCK *pSrc, BLOCK *pDst, MLong lAngle, BLOCK *xDstBlock, BLOCK *yDstBlock)
{
    MRESULT res = LI_ERR_NONE;
	MLong Ax, Bx, Cx;
	MLong Ay, By, Cy;
	MDouble sinA, cosA, absSinA, absCosA;
	MDouble angle;

	MLong srcWidth, srcHeight, srcStride;
	MLong dstWidth, dstHeight, dstStride;
	MLong dstExt;
	MUInt8 *pSrcData, *pDstData;
	MUInt8 *pSrcDataTmp;
	MUInt16 *pXData, *pYData;
	//MLong offSet;
	MLong A, B, C, D;
	MLong srcX, srcY;
	MLong tmpU, tmpV, tmpRes, u, v;
	MLong xx, yy;
	MLong tmpVal1, tmpVal2, tmpVal3;
    MLong xOffSet, yOffSet;

	srcWidth = pSrc->lWidth;
	srcHeight = pSrc->lHeight;
	srcStride = pSrc->lBlockLine;

	angle = lAngle * DEG2RAD;
	sinA = sin(angle);
	cosA = cos(angle);
	absSinA = fabs(sinA);
	absCosA = fabs(cosA);

	dstWidth = (MLong)(srcHeight * absSinA + srcWidth * absCosA);
	dstHeight = (MLong)(srcWidth * absSinA + srcHeight * absCosA);
	dstStride = pDst->lBlockLine;
	dstExt = dstStride - dstWidth;

	pSrcData = (MUInt8*)pSrc->pBlockData;
	pDstData = (MUInt8*)pDst->pBlockData;
	pXData = (MUInt16*)xDstBlock->pBlockData;
	pYData = (MUInt16*)yDstBlock->pBlockData;

	pDst->lWidth = dstWidth;
	pDst->lHeight = dstHeight;

	JMemSet(pDstData, 0, dstStride * dstHeight);

	Ax = (MLong)(cosA * FLOAT_SCALOR + 0.5);
	Bx = (MLong)(-sinA * FLOAT_SCALOR + 0.5);
	Cx = (MLong)((-dstWidth * cosA + dstHeight * sinA + srcWidth)*(FLOAT_SCALOR>>1) + 0.5);
	Ay = (MLong)(sinA * FLOAT_SCALOR + 0.5);
	By = (MLong)(cosA * FLOAT_SCALOR + 0.5);
	Cy = (MLong)((-dstWidth * sinA - dstHeight * cosA + srcHeight)*(FLOAT_SCALOR>>1) + 0.5);

	tmpVal1 = ((1<<FLOAT_SHIFT)-1);
	tmpVal2 = 1<<(FLOAT_SHIFT - 1);
	tmpVal3 = 1<<(FLOAT_SHIFT*2-1);

	tmpU = Cx;
	tmpV = Cy;
	xOffSet = dstWidth * Ax;
	yOffSet = dstWidth * Ay;

	for(yy=0; yy<dstHeight; yy++, pDstData+=dstExt, pXData+=dstExt, pYData+=dstExt, tmpU += Bx, tmpV += By)
	{
		for(xx=0; xx<dstWidth; xx++, pDstData++, pXData++, pYData++, tmpU += Ax, tmpV += Ay)
        {
			//tmpU = Ax * xx + Bx * yy + Cx;
			//tmpV = Ay * xx + By * yy + Cy;
			srcX = tmpU>>FLOAT_SHIFT;
			srcY = tmpV>>FLOAT_SHIFT;
			u = tmpU & tmpVal1;
			v = tmpV & tmpVal1;

			//offSet = yy * dstStride + xx;
			*pXData = (MUInt16)((tmpU + tmpVal2)>>FLOAT_SHIFT);
			*pYData = (MUInt16)((tmpV + tmpVal2)>>FLOAT_SHIFT);

			if(srcX>=0 && srcX<srcWidth-1 && srcY>=0 && srcY<srcHeight-1)
			{
              	pSrcDataTmp = (pSrcData + srcY * srcStride + srcX);
				A = *pSrcDataTmp;
				B = *(pSrcDataTmp + 1);
				C = *(pSrcDataTmp + srcStride);
				D = *(pSrcDataTmp + srcStride + 1);
				tmpRes = (u*v*D + (FLOAT_SCALOR-u)*v*C + u*(FLOAT_SCALOR-v)*B
					      + (FLOAT_SCALOR-u)*(FLOAT_SCALOR-v)*A + (tmpVal3))>>(FLOAT_SHIFT<<1);
				*pDstData = (MUInt8)tmpRes;
			}
		}
		tmpU -= xOffSet;
		tmpV -= yOffSet;
	}
}

MVoid RotateImage_EX3(BLOCK *pSrc,BLOCK *pDst,MLong lAngle,BLOCK *xDstBlock,BLOCK *yDstBlock)
{
	MLong x;
	MLong y;
	MLong ind;
	MLong x_dst;
	MLong y_dst;
	MLong ind_dst;
	MDouble angle = lAngle  * ROTATE_PI / 180.; // angle in radian  
	MFloat a = (MFloat)(sin(angle));
	MFloat b = (MFloat)(cos(angle)); // sine and cosine of angle 
	MFloat xu,yv;
	MFloat u,v;
	MLong pixel[4];
	MRESULT res = LI_ERR_NONE;

	MLong w_src = pSrc->lWidth;  
	MLong h_src = pSrc->lHeight; 
	MFloat temp1 = (MFloat)(fabs(a));
	MFloat temp2 = (MFloat)(fabs(b));
	MFloat temp3 = (MFloat)(h_src * fabs(a) + w_src * fabs(b));
	MFloat temp4 = h_src * temp1 + w_src * temp2;
	MLong w_dst = (MLong)(h_src * fabs(a) + w_src * fabs(b)); 
	MLong h_dst = (MLong)(w_src * fabs(a) + h_src * fabs(b));
	MInt16 *x_Data;
	MInt16 *y_Data;

	MDouble m[6];
	MUInt8 *pSrcData;
	MUInt8 *pDstData;


	x_Data=(MInt16*)(xDstBlock->pBlockData);
	y_Data=(MInt16*)(yDstBlock->pBlockData);
	m[0]=b;
	m[1]=-a;
	m[2]=w_src/2-b*w_dst/2+a*h_dst/2;
	m[3]=a;
	m[4]=b;
	m[5]=h_src/2-a*w_dst/2-b*h_dst/2;


	//GO(B_Create(hMemMgr, pDst, DATA_U8, w_dst, h_dst));
	pDst->lWidth=w_dst;
	pDst->lHeight=h_dst;
	xDstBlock->lBlockLine=pDst->lBlockLine;
	xDstBlock->lWidth=pDst->lWidth;
	xDstBlock->lHeight=pDst->lHeight;
	yDstBlock->lBlockLine=pDst->lBlockLine;
	yDstBlock->lHeight=pDst->lHeight;
	yDstBlock->lWidth=pDst->lWidth;
	//pDst->lBlockLine=(pDst->lWidth+3)&(~3);
	//pDst->typeDataA=DATA_U8;
	//pDst->pBlockData=pWorkBuf;


	pSrcData=(MUInt8*)(pSrc->pBlockData);
	pDstData=(MUInt8*)(pDst->pBlockData);

	//memset(pDstData,0,pDst->lBlockLine*pDst->lHeight);
	JMemSet(pDstData,0,pDst->lBlockLine*pDst->lHeight);

	for (y=0;y<pDst->lHeight;y++)
	{
		for (x=0;x<pDst->lWidth;x++)
		{
			xu = (MFloat)(m[0]*x+m[1]*y+m[2]);
			yv = (MFloat)(m[3]*x+m[4]*y+m[5]);
			x_dst=(MLong)(m[0]*x+m[1]*y+m[2]);
			y_dst=(MLong)(m[3]*x+m[4]*y+m[5]);
			u=xu-x_dst;
			v=yv-y_dst;

			ind=y*(pDst->lBlockLine)+x;
			ind_dst=y_dst*(pSrc->lBlockLine)+x_dst;
			/*
			if ((x==59)&&(y==45))
			{
				printf("Angle:%d X:%d Y:%d ",lAngle,(MLong)(xu+0.5),(MLong)(yv+0.5));
				printf("X:%f,Y:%f\n",xu,yv);
			}*/
			*(x_Data+ind) = (MUInt16)(xu+0.5);
			*(y_Data+ind) = (MUInt16)(yv+0.5);



			if (y_dst<h_src-1&&x_dst<w_src-1&&y_dst>=0&&x_dst>=0)
			{
				pixel[0]=*(pSrcData+ind_dst);                          //image[x,y]
				pixel[1]=*(pSrcData+ind_dst+pSrc->lBlockLine);         //image[x,y+1]
				pixel[2]=*(pSrcData+ind_dst+1);                        //image[x+1,y]
				pixel[3]=*(pSrcData+ind_dst+pSrc->lBlockLine+1);       //image[x+1,y+1]
				*(pDstData+ind) = (MUInt8)((1-u)*(1-v)*pixel[0]+(1-u)*v*pixel[1]+u*(1-v)*pixel[2]+u*v*pixel[3]+0.5);
			}
		}
	}	
	
}
MVoid RotateImage_EX2(BLOCK *pSrc,BLOCK *pDst,MLong lAngle)
{
	MLong x;
	MLong y;
	MLong ind;
	MLong x_dst;
	MLong y_dst;
	MLong ind_dst;
	MDouble angle = lAngle  * ROTATE_PI / 180.; // angle in radian  
	MFloat a = (MFloat)(sin(angle));
	MFloat b = (MFloat)(cos(angle)); // sine and cosine of angle 
	MFloat xu,yv;
	MFloat u,v;
//	MLong pixel[4];
//	MRESULT res = LI_ERR_NONE;

	MLong w_src = pSrc->lWidth;  
	MLong h_src = pSrc->lHeight; 
	MFloat temp1 = (MFloat)(fabs(a));
	MFloat temp2 = (MFloat)(fabs(b));
	MFloat temp3 = (MFloat)(h_src * fabs(a) + w_src * fabs(b));
	MFloat temp4 = (MFloat)(h_src * temp1 + w_src * temp2);
	MLong w_dst = (MLong)(h_src * fabs(a) + w_src * fabs(b)); 
	MLong h_dst = (MLong)(w_src * fabs(a) + h_src * fabs(b));

	MDouble m[6];
	MUInt8 *pSrcData;
	MUInt8 *pDstData;

	if (pSrc->typeDataA!=DATA_U8)
	{
//		res=LI_ERR_IMAGE_FORMAT;
//		goto EXT;
		return;
	}
	m[0]=b;
	m[1]=-a;
	m[2]=w_src/2-b*w_dst/2+a*h_dst/2;
	m[3]=a;
	m[4]=b;
	m[5]=h_src/2-a*w_dst/2-b*h_dst/2;


	//GO(B_Create(hMemMgr, pDst, DATA_U8, w_dst, h_dst));
	pDst->lWidth=w_dst;
	pDst->lHeight=h_dst;
	pDst->lBlockLine=w_dst;
	//pDst->lBlockLine=(pDst->lWidth+3)&(~3);
	//pDst->typeDataA=DATA_U8;
	//pDst->pBlockData=pWorkBuf;


	pSrcData=(MUInt8*)(pSrc->pBlockData);
	pDstData=(MUInt8*)(pDst->pBlockData);

	memset(pDstData,0,pDst->lBlockLine*pDst->lHeight);

	for (y=0;y<pDst->lHeight;y++)
	{
		for (x=0;x<pDst->lWidth;x++)
		{
			xu = (MFloat)(m[0]*x+m[1]*y+m[2]);
			yv = (MFloat)(m[3]*x+m[4]*y+m[5]);
			x_dst=(MLong)(m[0]*x+m[1]*y+m[2]+0.5);
			y_dst=(MLong)(m[3]*x+m[4]*y+m[5]+0.5);
			u=xu-x_dst;
			v=yv-y_dst;

			ind=y*(pDst->lBlockLine)+x;
			ind_dst=y_dst*(pSrc->lBlockLine)+x_dst;
			/*
			if (y_dst<h_src-1&&x_dst<w_src-1&&y_dst>=0&&x_dst>=0)
			{
				pixel[0]=*(pSrcData+ind_dst);                          //image[x,y]
				pixel[1]=*(pSrcData+ind_dst+pSrc->lBlockLine);         //image[x,y+1]
				pixel[2]=*(pSrcData+ind_dst+1);                        //image[x+1,y]
				pixel[3]=*(pSrcData+ind_dst+pSrc->lBlockLine+1);       //image[x+1,y+1]
				*(pDstData+ind)=(1-u)*(1-v)*pixel[0]+(1-u)*v*pixel[1]+u*(1-v)*pixel[2]+u*v*pixel[3];
			}
			*/
			if (y_dst<h_src&&x_dst<w_src&&y_dst>=0&&x_dst>=0)
			{
				*(pDstData+ind)=*(pSrcData+ind_dst); 
			}

		}
	}	

//EXT:
	//FreeVectMem(hMemMgr,pDst);
//	return res;;
}

MVoid RotateImage_EX4(BLOCK *pSrc,BLOCK *pDst,MLong lAngle)
{
	
}