/*!
* \file liblur.c
* \brief  the functions related to calculate the blur kernel 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/

#include"liblur.h"
#include"limath.h"
#include"LiErrFunc.h"
#include"limatrix.h"
#include"liedge.h"
#include"lidebug.h"
#include"liintegral.h"
#include"ligaussian.h"
//#include <time.h>
#define  NPIECE 2 //图像划分成NPIECE*NPIECE SeleBlock函数使用

/// \brief Liblur is used to calculate the p parameter for calculate the sigma 
/// \param  hMemMgr handle
/// \param  blockres the block saved p parameters
/// \param  blocksrc the original image block
/// \param  blocka the reblurred image a
/// \param  blockb the reblurred image b
/// \return  error code
/// \用再模糊之后的两张图像计算图像的局部最大最小值，梯度信息
MRESULT LiBlur(MHandle hMemMgr,PBLOCKEXT blockres, PBLOCKEXT blocksrc, PBLOCKEXT blockedge,PBLOCKEXT blocka,PBLOCKEXT blockb)
{
	MLong res=LI_ERR_NONE;
	MUInt8* datasrc;
	MUInt8*dataedge;
	MUInt8* dataa;
	MUInt8 *datab;
	MDouble *datares;
	MLong max1;
	MLong min1;
	MLong a=5;
	MLong i,j,k,l;
	MDouble dxa,dxb,dya,dyb,swa,swb,pwa,pwb,p,lwa,lwb,L;
	MFloat ea,eb;
	MLong  H;
	MFloat eps=0.000001;
	MLong width,height,blres,blsrc,bla,blb,bledge;
	MLong exte,extres;
	MLong temp;

	width=blocksrc->ext.right-blocksrc->ext.left;
	height=blocksrc->ext.bottom-blocksrc->ext.top;
	blsrc=blocksrc->block.lBlockLine;
	blres=blockres->block.lBlockLine;
	bla=blocka->block.lBlockLine;
	blb=blockb->block.lBlockLine;
	bledge=blockedge->block.lBlockLine;

	datasrc=(MUInt8*)blocksrc->block.pBlockData+blocksrc->ext.left+a+(blocksrc->ext.top+a)*blsrc;
	dataa=(MUInt8*)blocka->block.pBlockData+blocka->ext.left+a+(blocka->ext.top+a)*bla;
	datab=(MUInt8*)blockb->block.pBlockData+blockb->ext.left+a+(blockb->ext.top+a)*blb;
	dataedge=(MUInt8*)blockedge->block.pBlockData+blockedge->ext.left+a+(blockedge->ext.top+a)*bledge;
	datares=(MDouble*)blockres->block.pBlockData+blockres->ext.left+a+(blockres->ext.top+a)*blres;
	exte=blockedge->block.lBlockLine-blockedge->ext.right+blockedge->ext.left+2*a;//2016.5.25 by zmh +2*a
	extres=blockres->block.lBlockLine-blockres->ext.right+blockres->ext.left+2*a;//2016.5.25 by zmh +2*a

	/// \calculate the local maximum and local minimum
	/// \计算局部最大最小值，5*5的区块内
	for(i=a;i<height-a;i++,dataedge+=exte,datares+=extres)
	{
		for(j=a;j<width-a;j++,dataedge++,datares++)
		{
            //printf("%d ",*(dataedge));
			if(*(dataedge)==0)//canny边缘检测后 边缘为黑
			{
				max1=0; /// <the local maximum
				min1=255; /// < the local minimum

				for(k=i-a;k<=i+a;k++)
					for(l=j-a;l<=j+a;l++)
					{   
						temp=*(datasrc+l+k*blsrc);
						if (temp>max1)
						{
							max1=temp;
						}
						if(temp<min1)
						{
							min1=temp;
						}
					}
					H=0;
					H=max1-min1;
					dataa=(MUInt8*)blocka->block.pBlockData+blocka->ext.left+a+(blocka->ext.top+a+i)*bla+j;
					datab=(MUInt8*)blockb->block.pBlockData+blockb->ext.left+a+(blockb->ext.top+a+i)*bla+j;
					if(H!=0)
					{
						swa=0;
						swb=0;
						swa=2.0*(*(dataa)-min1)/H-1;
						swb=2.0*(*(datab)-min1)/H-1;
						if(fabs(swa)<1&&fabs(swb)<1)
						{
							ea=0;
							eb=0;
							Erfinv(&ea,swa);
							Erfinv(&eb,swb);
							if (fabs(eb)<fabs(ea))
							{
								pwa=0;
								pwb=0;
								pwa=exp((float)(-ea*ea));
								pwb=exp((float)(-eb*eb));
								if (fabs(pwa)>eps)
								{
									p=0;
									p=pwb/pwa;
									/// \calculate the gradient
									dxa=0;
									dya=0;
									dxb=0;
									dyb=0;
									dxa=*(dataa+1)-(*(dataa));
									dya=*(dataa+bla)-(*(dataa));
									dxb=*(datab+1)-(*(datab));
									dyb=*(datab+blb)-(*(datab));
									lwa=0;
									lwb=0;
									lwa=sqrt((float)(dxa*dxa+dya*dya));
									lwb=sqrt((float)(dxb*dxb+dyb*dyb));
									if(fabs(lwb)>eps)
									{
										L=0;
										L=lwa/lwb;
										*(datares)=p*p*L*L;
									}                        
								}
							}

						}
					}       
			}
		}
		//printf("\n\n");
	}
EXT:
	return res;
}


/// \brief Lisigam is used to calculate sigma using the relationship between sigmaa and 
/// sigmab and blockp
/// \param  hMemMgr handle
/// \param  sigma the wanted sigma
/// \param  blocksrc   the blockp
/// \param  sigmaa the reblur parameter a
/// \param  sigmab the reblur parameter b
/// \return  error code
/// \根据梯度信息以及两个再模糊的sigma计算最后的sigma，公式见论文
MRESULT Lisigma(MHandle hMemMgr,MFloat* sigma,PBLOCKEXT blocksrc,MLong sigmaa,MLong sigmab)
{
	MLong res=LI_ERR_NONE;
	BLOCK sig={0};
	MDouble * datasrc;
	MFloat* datasig;
	MLong i,j,m,m1; 
	MLong n=0;
	MFloat temp=0.0;
	MFloat temp1=0.0;
	MLong ext,extsig;
	MFloat sum=0.0;
	MFloat eps=0.000001;
	MLong width=blocksrc->ext.right-blocksrc->ext.left;
	MLong height=blocksrc->ext.bottom-blocksrc->ext.top;
	MLong blockline=blocksrc->block.lBlockLine;
	MFloat *s=MNull;

	//int nTime=0;

	GO(B_Create(hMemMgr,&sig,DATA_F32,width,height));
	B_Set(&sig,0);
	datasrc=(MDouble*)blocksrc->block.pBlockData+blocksrc->ext.left+blocksrc->ext.top*blocksrc->block.lBlockLine;
	datasig=(MFloat*)sig.pBlockData;
	ext=blocksrc->block.lBlockLine-blocksrc->ext.right;
	extsig=sig.lBlockLine-sig.lWidth;

	/// \using the sigmaa and sigma b calculate wanted sigma
	for(i=0;i<height;i++,datasrc+=ext,datasig+=extsig)
		for(j=0;j<width;j++,datasrc++,datasig++)
		{
			temp=*datasrc;
			if(fabs(temp)>eps&&fabs((temp-1))>eps)
			{
				temp1=fabs((float)((sigmab*sigmab-temp*sigmaa*sigmaa)/(temp-1)));
				*(datasig)=sqrt(temp1);
			}
		}


		datasig=(MFloat*)sig.pBlockData;
		/// \find the non-zero parameters and rearrange
		/// \找到所有的sigma，排序，去掉0元素，找中心的20%的sigma
		for (i=0;i<sig.lHeight;i++,datasig+=extsig)
			for(j=0;j<sig.lWidth;j++,datasig++)
			{
				temp=fabs(*(datasig));
				if(temp>eps)
				{
					n=n+1;
				}
			}
			if (n>eps)
			{
				AllocVectMem(hMemMgr,s,n,MFloat);
				m=0.5*n;
				m1=0.3*n;
				n=0;
				datasig=(MFloat*)sig.pBlockData;
				for (i=0;i<sig.lHeight;i++,datasig+=extsig)
					for(j=0;j<sig.lWidth;j++,datasig++)
					{
						if(fabs(*(datasig))>eps)
						{
							*(s+n)=*(datasig);
							n=n+1;
						}
					}
					///\ rearrange the parameters，冒泡排序
					//nTime=clock();
					blobSort(s,n);
					//nTime=clock()-nTime;
					//      printf( "%d blobSort run time =%dms\n", n,nTime);
					for(i=m1;i<m;i++)
					{
						sum=sum+s[i];

					}

					n=m-m1;
					*sigma=sum/n;
			}
			if (n<eps)
			{
				*sigma=50;
			}

EXT:
			FreeVectMem(hMemMgr,s);
			B_Release(hMemMgr,&sig);

			return res;
}



/// \brief SigmalSel is used to estimate the final sigma using the relationships
/// between the sigmas
/// \param  sigres the final sigma
/// \param  sigma1 the sigma estimated from the first reblur
/// \param  sigma2 the sigma estimated from the second reblur
/// \param  sigma3 the sigma estimated from the third reblur
/// \param sigma4 the sigma estimated from the forth reblur
/// \return  error code
/// \根据再模糊推算出来的sigma的值决定最后的sigma的值
MRESULT SigmaSel(MFloat *sigres,MFloat sigma1,MFloat sigma2)
{
	MLong res=LI_ERR_NONE;
	MFloat eps=0.00000001;
	/// \getting the final sigma
	if(sigma2<7.6&&sigma1>eps)
	{
		*sigres=sigma1;
		return res;
	}
	if(sigma2>7.6)
	{
		*sigres=sigma2;
		return res;
	}
	*sigres=50;
	return res;    
}

/// \brief SelePlainBlock  select the 1/16 most plain part in the original block
/// \param  hMemMgr Handle
/// \param  blocksrc the source image
/// \param  blockres the 1/16 most plain part
/// \return  error code
/// \找到图像中较平坦的1/16区域
MRESULT SelePlainBlock(MHandle hMemMgr,PBLOCKEXT blockres,PBLOCKEXT blocksrc)
{
	MLong res=LI_ERR_NONE;
	BLOCK blockedge={0};
	MUInt8* dataedge;
	MUInt8* datasrc;
	MLong i,j,l,m,w,h;
	MLong width,height;
	MLong a[4][4]={0};
	MLong min;
	MLong mini,minj;
	MLong ext;

	width=blocksrc->ext.right-blocksrc->ext.left;
	height=blocksrc->ext.bottom-blocksrc->ext.top;
	w=width/4; /// 宽度的1/4
	h=height/4; ///高度的1/4
	GO(B_Create(hMemMgr,&blockedge,DATA_U8,width,height));
	B_Set(&blockedge,0);
	dataedge=(MUInt8*)blockedge.pBlockData;
	datasrc=(MUInt8*)blocksrc->block.pBlockData+blocksrc->ext.left+blocksrc->ext.top*(blocksrc->block.lBlockLine);
	/// \detect the edge边缘检测
	GO(Edge(hMemMgr,datasrc,blocksrc->block.lBlockLine,width,height,dataedge,blockedge.lBlockLine,TYPE_CANNY));
	/// \ find the most plain part using the information of edge  边缘点最少的1/16区域
	for(i=0;i<4;i++)
		for(j=0;j<4;j++)
		{
			dataedge=(MUInt8*)blockedge.pBlockData+j*w+i*h*blockedge.lBlockLine;
			ext=blockedge.lBlockLine-blockedge.lWidth+j*w;
			for (l=i*h;l<(i+1)*h;l++,dataedge+=ext)
				for(m=j*w;m<(j+1)*w;m++,dataedge++)
				{
					if(*(dataedge)==0)
						a[i][j]=a[i][j]+1;
				}
		}

		/// \find the 1/16 parts which contains least edge points
		min=a[0][0];
		mini=0;
		minj=0;
		for(i=0;i<4;i++)
			for(j=0;j<4;j++)
			{
				if(a[i][j]<min)
				{
					min=a[i][j];
					mini=i;
					minj=j;
				}
			}

			blockres->ext.top=blocksrc->ext.top+mini*h;
			blockres->ext.bottom=blockres->ext.top+h;
			blockres->ext.left=blocksrc->ext.left+minj*w;
			blockres->ext.right=blockres->ext.left+w;

EXT:
			B_Release(hMemMgr,&blockedge);
			return res;


}

/// \brief SelePlainBlock  select the 1/4 part of the block which contains the 
/// most edge points
/// \param  hMemMgr Handle
/// \param  block1  the source image
/// \param  block2 the edgemap
/// \return  error code
/// \找到边缘点个数最多的1/4的区块
MRESULT SeleBlock(MHandle hMemMgr,PBLOCKEXT block1,PBLOCKEXT block2)
{
	MLong res=LI_ERR_NONE;
	MUInt8* dataedge;
	MUInt8* datasrc;
	MLong i,j,l,m;
	//const int N=4;
	MLong w=(block1->ext.right-block1->ext.left)/NPIECE;
	MLong h=(block1->ext.bottom-block1->ext.top)/NPIECE;
	MLong a[NPIECE][NPIECE]={0};
	MLong max;
	MLong maxi,maxj;
	MLong ext;
	MLong  edgeline=0;
	MLong edgewidth=block2->ext.right-block2->ext.left;
	MLong edgeheight=block2->ext.bottom-block2->ext.top;
	BLOCK blockint={0};
	MUInt32 *dataint=MNull;
	MLong itemp,jtemp;
	MLong temp=0;
	MLong maxint=0;
	MLong resp=0;
	MLong s=0;
	itemp=0;
	jtemp=0;
	dataedge=(MUInt8*)block2->block.pBlockData+block2->ext.left+(block2->ext.top)*block2->block.lBlockLine;
	/// \using the information of edge points to find the max
	for(i=0;i<NPIECE;i++)
		for(j=0;j<NPIECE;j++)
		{
			//s=1-j;
			for (m=i*h;m<(i+1)*h;m++)
				for(l=j*w;l<(j+1)*w;l++)
				{
					temp=*(dataedge+l+m*block2->block.lBlockLine);
					if(temp==0) //canny边缘检测后 边缘为黑
						a[i][j]=a[i][j]+1;		
				}
				/*dataedge=(MUInt8*)block2->block.pBlockData+block2->ext.left+(block2->ext.top)*block2->block.lBlockLine+j*w+i*h*block2->block.lBlockLine;
				ext=block2->block.lBlockLine-block2->ext.right+block2->ext.left+s*w;
				for (m=i*h;m<(i+1)*h;m++,dataedge+=ext)
				for(l=j*w;l<(j+1)*w;l++,dataedge++)
				{
				if(*(dataedge)==0)
				a[i][j]=a[i][j]+1;
				}*/
		}

		/// \find the 1/4 part which contain	s the most edge points 
		max=a[0][0];
		maxi=0;
		maxj=0;
		for(i=0;i<NPIECE;i++)
			for(j=0;j<NPIECE;j++)
			{
				if(a[i][j]>max)
				{
					max=a[i][j];
					maxi=i;
					maxj=j;
				}
			}

			/// \set the position
			block1->ext.top=block1->ext.top+maxi*h;
			block1->ext.bottom=block1->ext.top+h;
			block1->ext.left=block1->ext.left+maxj*w;
			block1->ext.right=block1->ext.left+w;
			block2->ext.top=block2->ext.top+maxi*h;
			block2->ext.bottom=block2->ext.top+h;
			block2->ext.left=block2->ext.left+maxj*w;
			block2->ext.right=block2->ext.left+w;

EXT:
			//     B_Release(hMemMgr,&blockint);
			return res;
}

MRESULT CalSigmaTwoGaussBlur(MHandle hMemMgr,MFloat *pSigma,PBLOCKEXT pBlocksrc,PBLOCKEXT pBlockedge,
							 MLong sigmaa,MLong sigmab)
{
	MLong res=LI_ERR_NONE;
	BLOCK imga={0};
	BLOCK imgb={0};
	BLOCK blockp={0};
	BLOCKEXT ImgA={0};
	BLOCKEXT ImgB={0};
	BLOCKEXT BlockP; 

	/// 再模糊图像，用sigma=1和sigma=2的参数模糊图像，这两个参数是由实验测得
	/// \dealing with a1，b1
	GO(B_Create(hMemMgr,&imga,DATA_U8,pBlocksrc->ext.right-pBlocksrc->ext.left,pBlocksrc->ext.bottom-pBlocksrc->ext.top));
	GO(B_Create(hMemMgr,&imgb,DATA_U8,pBlocksrc->ext.right-pBlocksrc->ext.left,pBlocksrc->ext.bottom-pBlocksrc->ext.top));
	B_Set(&imga,0);
	B_Set(&imgb,0);
	ImgA.block=imga;
	ImgA.ext.left=0;
	ImgA.ext.right=imga.lWidth;
	ImgA.ext.top=0;
	ImgA.ext.bottom=imga.lHeight;
	ImgB.block=imgb;
	ImgB.ext.left=0;
	ImgB.ext.right=imgb.lWidth;
	ImgB.ext.top=0;
	ImgB.ext.bottom=imgb.lHeight;

	///用sigma=a1进行在模糊，存储在imgA中
	GO(LiGaussianBlur(hMemMgr,&ImgA,pBlocksrc,sigmaa));
	//PrintBmpEx((MUInt8*)imga.pBlockData,imga.lBlockLine,DATA_U8,imga.lWidth,imga.lHeight,1,"c:\\imga.bmp");
	///用sigma=b1进行在模糊，存储在imgB中
	GO(LiGaussianBlur(hMemMgr,&ImgB,pBlocksrc,sigmab));

	GO(B_Create(hMemMgr,&blockp,DATA_F64,pBlocksrc->ext.right-pBlocksrc->ext.left,pBlocksrc->ext.bottom-pBlocksrc->ext.top));
	//    PrintBmpEx((MUInt8*)imgb.pBlockData,imgb.lBlockLine,DATA_U8,imgb.lWidth,imgb.lHeight,1,"c:\\imgb.bmp");
	B_Set(&blockp,0);
	ImgA.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	ImgB.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	BlockP.block=blockp;
	BlockP.ext.left=0;
	BlockP.ext.right=MIN(ImgA.ext.right,ImgB.ext.right);
	BlockP.ext.top=0;
	BlockP.ext.bottom=ImgA.ext.bottom;

	///计算两幅图像之间的梯度区别，根据matlab算法改编  BlockP 梯度图 blockimg1灰度图  blockedge边缘图 ImgA ImgB模糊图像 这几个图像大小一致
	GO(LiBlur(hMemMgr,&BlockP,pBlocksrc,pBlockedge,&ImgA,&ImgB));
	///根据两幅图像的梯度信息，还有sigmaa，sigmab获得预测图像的sigma
	GO(Lisigma(hMemMgr,pSigma,&BlockP,sigmaa,sigmab));

EXT:
	B_Release(hMemMgr,&imga);
	B_Release(hMemMgr,&imgb);
	B_Release(hMemMgr,&blockp);
	return res;
}

MRESULT CalImgPieceWeight(MHandle hMemMgr,MFloat *pWeight,PBLOCKEXT pBlockedge)
{
	MLong res=LI_ERR_NONE;
	MLong i=0;
	MLong j=0;
	MUInt8*dataedge;
	MLong exte;
	MLong temp;
	MFloat sumWeight=0.0; 

	dataedge=(MUInt8*)pBlockedge->block.pBlockData+pBlockedge->ext.left+(pBlockedge->ext.top)*(pBlockedge->block.lBlockLine);//边缘数据的起始指针
	exte=pBlockedge->block.lBlockLine-pBlockedge->ext.right+pBlockedge->ext.left;//换行指针移动的大小

	for(i=pBlockedge->ext.top;i<pBlockedge->ext.bottom;i++,dataedge+=exte)
	{
		for(j=pBlockedge->ext.left;j<pBlockedge->ext.right;j++,dataedge++)
		{
			if(*dataedge==0)
			{
				sumWeight=sumWeight+1.0;
			}
		}
	}
	*pWeight=sumWeight;
EXT:
	return res;
}