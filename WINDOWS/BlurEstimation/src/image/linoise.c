/*!
* \file LiNoise.c
* \brief  the function related to noiselevel 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/
#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include "linoise.h"
#include "liconv.h"
#include"liimfilter.h"
#include"limatrix.h"
#include"litimer.h"

/*单帧图像的噪声估计是根据NOISE LEVEL ESTIMATION USING WEAK TEXTURED PATCHES OF
A SINGLE NOISY IMAGE文章的matlab源码进行更改的，其算法原理可行，代码验证可行，
    但是阈值部分没有根据实际数据进行验证，而且该算法需要较多时间*/

/// \brief  NoiseLevel  Estimate the noiselevel of the input block 
/// \param hMemMgr handle
/// \param Noisel estimated noiselevel
/// \param blocka the source block
/// \param patchsize the patch size of the block
/// \param decim decimation factor.
/// \param condf confidence interval to determin the threshold for the weak texture
/// \the default is 0.99
///\param itr number of iteration default is 2
/// \return  error code
MRESULT NoiseLevel( MHandle hMemMgr, MFloat* Noisel,PBLOCKEXT BlockA,MLong patchsize,MLong decim,MFloat condf,MLong itr)
{
    MLong res=LI_ERR_NONE;
    MLong width,height,rank,i,j;
    MFloat trace,tau0,r,t,tau;
    MDouble n;
    MFloat sig2=0.0;
    MLong EffWidth=-1;
    MLong EffWidth1=-1;
    MFloat* data1;
    MFloat *data2;
    MFloat *data3;
    MUInt8* datatest,*datatest1;
    MFloat *pDataH,*pDataV;
    MFloat *ph1,*pv1;
    MDouble *datadou;
    MLong exth,extv;
    MLong tempwidth,temphight;
    BLOCK BlockH={0};
    BLOCK BlockV={0};
    BLOCK   imgH={0};
    BLOCK   imgV={0};	
    BLOCK TH={0};
    BLOCK THT={0};
    BLOCK TVT={0};
    BLOCK TV={0};
    BLOCK X={0};
    BLOCK XH={0};
    BLOCK XV={0};
    BLOCK DD={0};
    BLOCK DDH={0};
    BLOCK DDV={0};
    BLOCK XT={0};
    BLOCK XXT={0};
    BLOCK XXT1={0};
    BLOCK BlockHV={0};
    BLOCK X1T={0};
    BLOCK X1TT={0};
    BLOCK X1TT0={0};
    BLOCK SUV1={0};
    BLOCK SUV2={0};
    BLOCKEXT imgV1={0};
    BLOCKEXT imgH1={0};
    BLOCKEXT sum={0};
    BLOCKEXT sum1={0};
    BLOCKEXT X1={0};
    BLOCKEXT X1Text={0};	
    
    width=BlockA->ext.right-BlockA->ext.left;
    height=BlockA->ext.bottom-BlockA->ext.top;
    
    /// \Initial H,V matrix
    GO(B_Create(hMemMgr,&BlockH,DATA_F32,3,1));
    GO(B_Create(hMemMgr,&BlockV,DATA_F32,1,3));
    pDataH=(MFloat*)(BlockH.pBlockData);
    pDataV=(MFloat*)(BlockV.pBlockData);
    *(pDataH)=-0.5;
    *(pDataH+1)=0;
    *(pDataH+2)=0.5;
    *(pDataV)=-0.5;
    *(pDataV+BlockV.lBlockLine)=0;
    *(pDataV+2*BlockV.lBlockLine)=0.5;
    GO(B_Create(hMemMgr,&imgH,DATA_F32,width,height));
    GO(B_Create(hMemMgr,&imgV,DATA_F32,width,height));
    GO(ImFilter(hMemMgr,&imgH,BlockA,&BlockH));
    GO(ImFilter(hMemMgr,&imgV,BlockA,&BlockV));
    imgV1.block=imgV;
    imgV1.ext.left=0;
    imgV1.ext.right=imgV.lWidth;
    imgV1.ext.top=1;
    imgV1.ext.bottom=imgV.lHeight-1;
    imgH1.block=imgH;
    imgH1.ext.top=0;
    imgH1.ext.bottom=imgH.lHeight;
    imgH1.ext.left=1;
    imgH1.ext.right=imgV.lWidth-1;
    ph1=(MFloat*)imgH1.block.pBlockData+imgH1.ext.left+(imgH1.ext.top)*(imgH1.block.lBlockLine);
    exth=imgH1.block.lBlockLine-imgH1.ext.right+imgH1.ext.left;
    tempwidth=imgH1.ext.right-imgH1.ext.left;
    temphight=imgH1.ext.bottom-imgH1.ext.top;
    for(i=0;i<temphight;i++,ph1+=exth)
    for(j=0;j<tempwidth;j++,ph1++)
    {
        *(ph1)=(*(ph1))*(*(ph1));
    }
    pv1=(MFloat*)imgV1.block.pBlockData+imgV1.ext.left+(imgV1.ext.top)*(imgV1.block.lBlockLine);
    extv=imgV1.block.lBlockLine-imgV1.ext.right+imgV1.ext.left;
    tempwidth=imgV1.ext.right-imgV1.ext.left;
    temphight=imgV1.ext.bottom-imgV1.ext.top;
    for(i=0;i<temphight;i++,pv1+=extv)
    for(j=0;j<tempwidth;j++,pv1++)
    {
            *(pv1)=(*(pv1))*(*(pv1));
    }
    

/// \calculate rank,trace andtau0
    TH.lWidth=patchsize*patchsize;
    TH.lHeight=(patchsize-BlockH.lHeight+1)*(patchsize-BlockH.lWidth+1);
    TV.lWidth=patchsize*patchsize;
    TV.lHeight=(patchsize-BlockV.lHeight+1)*(patchsize-BlockV.lWidth+1);
    GO(B_Create(hMemMgr,&TH,DATA_F32,TH.lWidth,TH.lHeight));
    GO(B_Create(hMemMgr,&TV,DATA_F32,TV.lWidth,TV.lHeight));
    B_Set(&TH,0);
    B_Set(&TV,0);
    GO(Convmtx(&TH,&BlockH,patchsize));
    GO(Convmtx(&TV,&BlockV,patchsize));
    GO(B_Create(hMemMgr,&THT,DATA_F32,TH.lHeight,TH.lWidth));
    GO(B_Create(hMemMgr,&TVT,DATA_F32,TV.lHeight,TV.lWidth));
    GO(TransPose(hMemMgr,&THT,&TH));
    GO(TransPose(hMemMgr,&TVT,&TV));
    GO(B_Create(hMemMgr,&DD,DATA_F32,patchsize*patchsize,patchsize*patchsize));
    GO(B_Create(hMemMgr,&DDH,DATA_F32,patchsize*patchsize,patchsize*patchsize));
    GO(B_Create(hMemMgr,&DDV,DATA_F32,patchsize*patchsize,patchsize*patchsize));
    GO(Litrmul(&DDH,&THT,&TH));
    GO(Litrmul(&DDV,&TVT,&TV));
    data1=(MFloat*)DD.pBlockData;
    data2=(MFloat*)DDH.pBlockData;
    data3=(MFloat*)DDV.pBlockData;
    exth=DDH.lBlockLine-DDH.lWidth;
    for(i=0;i<DDH.lHeight;i++,data1+=exth,data2+=exth,data3+=exth)
    for(j=0;j<DDH.lWidth;j++,data1++,data2++,data3++)
    {
        *(data1)=*(data2)*(*(data3));
    }
    
    GO(	LiRank(hMemMgr,&rank,&DD));
    GO(LiTrace(hMemMgr,&trace,&DD));
    rank=rank;
    r=rank/2.0;
    t=2.0*trace/rank;
    GO(LiGamInv(&tau0,condf,r,t));

    //// \calculate the  covariance matrix
    GO(B_Create(hMemMgr,&X,BlockA->block.typeDataA,(width-patchsize+1)*(height-patchsize+1),patchsize*patchsize));
    GO(B_Create(hMemMgr,&XH,imgH1.block.typeDataA,((imgH1.ext.right-imgH1.ext.left)-patchsize+2+1)*((imgH1.ext.bottom-imgH1.ext.top)-patchsize+1),patchsize*(patchsize-2)));
    GO(B_Create(hMemMgr,&XV,imgV1.block.typeDataA,((imgV1.ext.right-imgV1.ext.left)-patchsize+1)*((imgV1.ext.bottom-imgV1.ext.top)-patchsize+2+1),patchsize*(patchsize-2)));
    GO(LiImTcol(hMemMgr,&X,BlockA,patchsize,patchsize));
    GO(LiImTcol(hMemMgr,&XH,&imgH1,patchsize-2,patchsize));
    GO(LiImTcol(hMemMgr,&XV,&imgV1,patchsize,patchsize-2));
    GO(B_Create(hMemMgr,&BlockHV,XH.typeDataA,XH.lWidth,XH.lHeight+XV.lHeight));
    GO(LiVertCat(&BlockHV,&XH,&XV));
    data1=(MFloat*)BlockHV.pBlockData;
    exth=BlockHV.lBlockLine-BlockHV.lWidth;
    for(i=0;i<BlockHV.lWidth;i++,data1++)
    {
        data2=data1;
        for(j=0;j<BlockHV.lHeight;j++,data2+=exth)
        {
            *(data1)+=*(data2);
        }
    }
    sum.block=BlockHV;
    sum.ext.top=0;
    sum.ext.left=0;
    sum.ext.right=BlockHV.lWidth;
    sum.ext.bottom=1;


/// \noise level estimation
    
   GO(B_Create(hMemMgr,&XT,X.typeDataA,X.lHeight,X.lWidth));
   GO(TransPose(hMemMgr,&XT,&X));
   GO(B_Create(hMemMgr,&XXT,X.typeDataA,X.lHeight,X.lHeight));
   GO (B_Create(hMemMgr,&XXT1,DATA_F64,XXT.lWidth,XXT.lHeight));
   GO(Litrmul(&XXT,&X,&XT));
    datatest=(MUInt8*)XXT.pBlockData;
    datadou=(MDouble*)XXT1.pBlockData;
    n=X.lWidth-1.0;
    exth=XXT1.lBlockLine-XXT1.lWidth;
    extv=XXT.lBlockLine-XXT.lWidth;
    for(i=0;i<XXT1.lHeight;i++,datatest+=extv,datadou+=exth)
    for(j=0;j<XXT1.lWidth;j++,datatest++,datadou++)
    {
        *(datadou)=*(datatest)/n;
    }
    width=MAX(XXT1.lWidth,XXT1.lHeight);
    GO(B_Create(hMemMgr,&SUV1,DATA_F64,width,3*width));
    B_Set(&SUV1,0);
    GO(LiSVD(hMemMgr,&SUV1,&XXT1));
    sig2=*((MDouble*)SUV1.pBlockData);
    sig2=sqrt((float) (sig2));
    
    /// \Weak Texture selection

    tau=tau0*sig2;
    
    datatest1=(MUInt8*)X.pBlockData;
    data1=(MFloat*)sum.block.pBlockData+sum1.ext.top*(sum.block.lBlockLine)+sum1.ext.left;
    data2=(MFloat*)sum.block.pBlockData+sum1.ext.top*(sum.block.lBlockLine)+sum1.ext.left;
    exth=sum.block.lBlockLine-sum.ext.right;
    extv=X.lBlockLine-X.lWidth;
    for(i=0;i<sum.block.lWidth;i++,data1++,datatest1++)
        {
            if(*(data1)<tau)
            {
                EffWidth++;
                *(data2+EffWidth)=*(data1);
                datatest=(MUInt8*)X.pBlockData;
                for(j=0;j<X.lHeight;j++,datatest1+=extv,datatest+=extv)
                {
                    *(datatest+EffWidth)=*(datatest1);
                }
            }
    }
    if(EffWidth!=-1)
    {
        X1.block=X;
        X1.ext.top=0;
        X1.ext.bottom=X.lHeight;
        X1.ext.left=0;
        X1.ext.right=EffWidth;
        /// \noise level estimation
        GO(B_Create(hMemMgr,&X1T,X1.block.typeDataA,(X1.ext.bottom-X1.ext.top),(X1.ext.right-X1.ext.left)));
        GO(TransPoseEXT(hMemMgr,&X1T,&X1));
        GO(B_Create(hMemMgr,&X1TT,X1.block.typeDataA,(X1.ext.bottom-X1.ext.top),(X1.ext.bottom-X1.ext.top)));
        X1Text.block=X1T;
        X1Text.ext.top=0;
        X1Text.ext.left=0;
        X1Text.ext.right=X1T.lWidth;
        X1Text.ext.bottom=X1T.lHeight;
        GO(LitrmulEXT(&X1TT,&X1,&X1Text));
        n=(X1.ext.right-X1.ext.left)-1;
        GO(B_Create(hMemMgr,&X1TT0,DATA_F64,X1TT.lWidth,X1TT.lHeight));
        datadou=(MDouble*)X1TT0.pBlockData;
        datatest1=(MUInt8*)X1TT.pBlockData;
        exth=X1TT0.lBlockLine-X1TT0.lWidth;
        extv=X1TT.lBlockLine-X1TT.lWidth;
        for(i=0;i<X1TT0.lHeight;i++,datadou+=exth,datatest1+=extv)
        for(j=0;j<X1TT.lWidth;j++,datadou++,datatest1++)
        {
            *(datadou)=*(datatest1)/n;
        }
        width=MAX(X1TT0.lWidth,X1TT0.lHeight);
        GO(B_Create(hMemMgr,&SUV2,DATA_F64,width,3*width));
        B_Set(&SUV2,0);
        GO(LiSVD(hMemMgr,&SUV2,&X1TT0));
        sig2=*((MDouble*)SUV2.pBlockData);
        sig2=sqrt((float) (sig2));
    }
        *Noisel=sig2;
EXT:
    B_Release(hMemMgr,&BlockH);
    B_Release(hMemMgr,&BlockV);
    B_Release(hMemMgr,&imgH);
    B_Release(hMemMgr,&imgV);
    B_Release(hMemMgr,&TH);
    B_Release(hMemMgr,&TV);
    B_Release(hMemMgr,&THT);
    B_Release(hMemMgr,&TVT);
    B_Release(hMemMgr,&DDH);
    B_Release(hMemMgr,&DDV);
    B_Release(hMemMgr,&X);
    B_Release(hMemMgr,&XH);
    B_Release(hMemMgr,&XT);
    B_Release(hMemMgr,&XV);
    B_Release(hMemMgr,&XXT);
    B_Release(hMemMgr,&XXT1);
    B_Release(hMemMgr,&SUV1);
    B_Release(hMemMgr,&SUV2);	
    return res;


    
}


/// \brief  NoiseLevelFrames  Estimate the noiselevel of the input blocks using 
/// the difference between two frames
/// \param hMemMgr handle
/// \param Noisel estimated noiselevel
/// \param blocksrc1 the source block1
/// \param blocksrc2 the source block2
/// \return  error code
MRESULT NoiseLevelFrames(MHandle hMemMgr,MFloat* NoiseL,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MUInt8* datasrc1,*datasrc2;
    MInt8* data;
    MLong i,j,ext,n,l,k;
    MFloat sum[4][4]={0};
    MFloat mean[4][4]={0};
    MFloat var[4][4]={0};
    MFloat std[4][4]={0};
    MFloat temp,stdmin;

    MLong tempw=blocksrc1->lWidth/4;
    MLong temph=blocksrc1->lHeight/4;
    BLOCK block={0};
    if ((blocksrc1->lHeight!=blocksrc2->lHeight)&&(blocksrc1->lWidth!=blocksrc2->lWidth))
    {
        res=LI_ERR_IMAGE_SIZE_UNMATCH;
        return res;
    }
    GO(B_Create(hMemMgr,&block,DATA_I8,blocksrc1->lWidth,blocksrc1->lHeight));
    datasrc1=(MUInt8*)blocksrc1->pBlockData;
    datasrc2=(MUInt8*)blocksrc2->pBlockData;
    data=(MInt8*)block.pBlockData;
    ext=blocksrc1->lBlockLine-blocksrc1->lWidth;
    for(i=0;i<blocksrc1->lHeight;i++,datasrc1+=ext,datasrc2+=ext,data+=ext)
    for(j=0;j<blocksrc1->lWidth;j++,datasrc1++,datasrc2++,data++)
    {
        *data=*datasrc2-*datasrc1;
    }
    /*求和*/
    data=(MInt8*)block.pBlockData;
    for(i=0;i<4;i++)
    for(j=0;j<4;j++)
    {
        for(l=i*temph;l<(i+1)*temph;l++)
        for(k=j*tempw;k<(j+1)*tempw;k++)
        {  
            temp=*(data+l*block.lBlockLine+k);
            sum[i][j]=temp+sum[i][j];
         }
      
    }
    /// \calculate the means
    n=tempw*temph;
    for(i=0;i<4;i++)
    for(j=0;j<4;j++)
    {
        mean[i][j]=sum[i][j]/n;
    }
    /// \calculate the variance
    data=(MInt8*)block.pBlockData;
    for(i=0;i<4;i++)
    for(j=0;j<4;j++)
    {
        for(l=i*temph;l<(i+1)*temph;l++)
        for(k=j*tempw;k<(j+1)*tempw;k++)
                {  
                    temp=*(data+l*block.lBlockLine+k)-mean[i][j];
                    temp=temp*temp;
                    var[i][j]+=temp;
                }

        }
    /// \calculate the standard variable
    for(i=0;i<4;i++)
    for(j=0;j<4;j++)
    {
        var[i][j]=var[i][j]/n;
        std[i][j]=sqrt(var[i][j]);
     }
    /// \find the minimum std
    stdmin=std[0][0];
    for(i=0;i<4;i++)
    for(j=0;j<4;j++)
    {
		//printf("the noise is =%f\n",std[i][j]);
            if (std[i][j]<stdmin)
            {
				
                stdmin=std[i][j];
            }
}
  
    *NoiseL=stdmin;
EXT:
    B_Release(hMemMgr,&block);
    return res;

}


/*****************************************************************************
 函 数 名  : Convmtx
 功能描述  : 求协方差矩阵
 输入参数  : 
 PBLOCK Blockres  输出的协方差矩阵
 PBLOCK BlockA    原矩阵
 MLong patchsize  块大小
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月23日
    作    者   :hmy
    修改内容   : 新生成函数

*****************************************************************************/

/// \brief  Convtmtxl Find the covariance matrix
/// \param hMemMgr Handle
/// \param Blockres the covariance matrix
/// \param BlockA the source matrix
/// \param patchsize the patch size of the block
/// \return  error code
MRESULT Convmtx(PBLOCK Blockres, PBLOCK BlockA,MLong patchsize)
{  
    MLong res=LI_ERR_NONE;
    MLong i=0;
    MLong j=0;
    MLong m;
    MLong n;
    MLong p;
    MLong k=0;
    MLong w;
    MFloat* data;
    MFloat *data1;
    data1=(MFloat*)Blockres->pBlockData;
    data=(MFloat*)BlockA->pBlockData;
    m=patchsize-BlockA->lHeight+1;
    n=patchsize-BlockA->lWidth+1;
    
    for(i=0;i<m;i++)
    for(j=0;j<n;j++)
    {
        for(p=0;p<BlockA->lHeight;p++) 
        for (w=0;w<BlockA->lWidth;w++)

            {
                *(data1+k*Blockres->lBlockLine+(i+p)*n+j+w)=*(data+p*BlockA->lBlockLine+w);
                
            }
            k=k+1;
    }
    
    return res;
        
}

/*****************************************************************************
 函 数 名  : WeakTextSEL
 功能描述  : 弱纹理区域选择
 输入参数  : 
 PBLOCK blockres1   选择的弱纹理区域
 PBLOCK blockres2   选择的弱纹理区域
 MLong* EffWidth    有效宽度
 PBLOCK blocksrc1   待选择原图像1
 PBLOCK blocksrc2   带选择原图像2
 MFloat Threshhold  阈值
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月23日
    作    者   :hmy
    修改内容   : 新生成函数

*****************************************************************************/
/// \brief WeakTextSEL select the weak texture part
/// \param blockres1 the weak texture part1
/// \param blockres2 the weak texture part2
/// \param EffWidth the effective width
/// \param blocksrc1 the source block1
/// \param blocksrc2 the source block2
/// \param Threshold the threshold to choose the weak texture part
/// \return  error code

MRESULT WeakTextSEL(PBLOCK blockres1,PBLOCK blockres2,MLong* EffWidth,PBLOCK blocksrc1,PBLOCK blocksrc2,MFloat Threshhold)
{
    MLong res=LI_ERR_NONE;
    MLong i,j,n;
    MFloat* datares1;
    MUInt8 *datares2;
    MFloat *datasrc1;
    MUInt8 *datasrc2;
    MLong Eff=0;
    datasrc1=(MFloat*)blocksrc1->pBlockData;
    datasrc2=(MUInt8*)blocksrc2->pBlockData;
    datares1=(MFloat*)blockres1->pBlockData;
    datares2=(MUInt8*)blockres2->pBlockData;
    for(i=0;i<blocksrc1->lHeight;i++)
    for(j=0;j<blocksrc1->lWidth;j++)
    {
        if ((*(datasrc1+i*blocksrc1->lBlockLine+j))>Threshhold)
        {
            Eff=Eff+1;
            *(datares1+i*blockres1->lBlockLine+Eff)=*(datasrc1+i*blocksrc1->lBlockLine+j);
            for(n=0;n<blocksrc2->lHeight;n++)
            {
                *(datares2+n*blockres2->lBlockLine+Eff)=*(datasrc2+n*blocksrc2->lBlockLine+j);
            }
        }
    }
    *EffWidth=Eff;
    return res;
}