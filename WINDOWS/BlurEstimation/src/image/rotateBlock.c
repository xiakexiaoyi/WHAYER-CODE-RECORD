#include <math.h>
#include"rotateBlock.h"
#define ROTATE_PI 3.14159265358979323846
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
				*(pDstData+ind)=(1-u)*(1-v)*pixel[0]+(1-u)*v*pixel[1]+u*(1-v)*pixel[2]+u*v*pixel[3];
			}
		}
	}	

EXT:
	//FreeVectMem(hMemMgr,pDst);
	return res;;
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
	MFloat a = sin(angle);
	MFloat b = cos(angle); // sine and cosine of angle 
	MFloat xu,yv;
	MFloat u,v;
	MLong pixel[4];
	MRESULT res = LI_ERR_NONE;

	MLong w_src = pSrc->lWidth;  
	MLong h_src = pSrc->lHeight; 
	MFloat temp1=fabs(a);
	MFloat temp2=fabs(b);
	MFloat temp3=h_src * fabs(a) + w_src * fabs(b);
	MFloat temp4=h_src * temp1 + w_src * temp2;
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


	//GO(B_Create(hMemMgr, pDst, DATA_U8, w_dst, h_dst));
	pDst->lWidth=w_dst;
	pDst->lHeight=h_dst;
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
				*(pDstData+ind)=(1-u)*(1-v)*pixel[0]+(1-u)*v*pixel[1]+u*(1-v)*pixel[2]+u*v*pixel[3];
			}
		}
	}	

EXT:
	//FreeVectMem(hMemMgr,pDst);
	return res;;
}