

/*!
* \file Limatrix.c
* \brief  the function related to matrix 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/



#include "limath.h"
#include "lidebug.h"
#include "liblock.h"
#include "liconv.h"
#include"limatrix.h"
#include"stdlib.h"


/// \brief  Lirank calculate the rank of the block 
/// \param  hMemMgr  Handle
/// \param  Rank the rank of the matrix
/// \param  BlockSrc the block needed to get the rank 
/// \return  error code
MRESULT LiRank(MHandle hMemMgr,MLong* rank,PBLOCK BlockSrc)
{
    MLong res=LI_ERR_NONE;
    switch(BlockSrc->typeDataA)
    {
    case DATA_U8:
        return LiRank_U8(hMemMgr,rank,BlockSrc);
    case DATA_F32:
        return LiRank_F32(hMemMgr,rank,BlockSrc);
    default:
        JASSERT(MFalse);
        return -1;
    }
    return res;

}

/// \brief  Lirank calculate the rank of the block 
/// \param  hMemMgr  Handle
/// \param  Rank the rank of the matrix
/// \param  BlockSrc the block needed to get the rank ,
///  datatype is UINT8
/// \return  error code
MRESULT LiRank_U8(MHandle hMemMgr,MLong* rank,PBLOCK BlockSrc)
{ 
    MRESULT res=LI_ERR_NONE;
    MLong m=BlockSrc->lHeight;
    MLong n=BlockSrc->lWidth;
    MLong i,j,k,nn,is,js,l,ll,u,v;
    MLong q,d;
    BLOCK blockA={0};
    MUInt8* PdataA;
    MUInt8* Pdata; 


    GO(B_Create(hMemMgr,&blockA,DATA_U8,n,m));
    B_Cpy(&blockA,BlockSrc);
    Pdata=(MUInt8*)BlockSrc->pBlockData;
    PdataA=(MUInt8*)blockA.pBlockData;
    
    nn=m;
    if (m>=n) 
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
        nn=n;
    }
    k=0;
    for (l=0; l<=nn-1; l++)
    { 
        q=0.0;
    for (i=l; i<=m-1; i++)
    for (j=l; j<=n-1; j++)
    {
        ll=i*n+j; d=abs(*(PdataA+i*n+j));
        if (d>q) 
        {
            q=d; is=i; js=j;
        }
    }
    if (q+1.0==1.0)
    {
        *rank=k;
    }
    k=k+1;
    if (is!=l)
    {
        for (j=l; j<=n-1; j++)
        {
            u=l*n+j; v=is*n+j;	
            d=*(PdataA+u);
            *(PdataA+u)=*(PdataA+v); 
            *(PdataA+v)=d;
        }
    }
    if (js!=l)
    { 
        for (i=l; i<=m-1; i++)
        { 
            u=i*n+js; v=i*n+l;
            d=*(PdataA+u); 
            *(PdataA+u)=*(PdataA+v); 
            *(PdataA+v)=d;
        }
    }
    ll=l*n+l;
    for (i=l+1; i<=n-1; i++)
    { 
        d=*(PdataA+i*n+l)/(*(PdataA+ll));
        for (j=l+1; j<=n-1; j++)
        { 
            u=i*n+j;
            *(PdataA+u)=*(PdataA+u)-d*(*(PdataA+l*n+j));
        }
    }
    }
    *rank=k;

EXT:
    B_Release(hMemMgr,&blockA);
    return res;

}

/// \brief  Lirank calculate the rank of the block 
/// \param  hMemMgr  Handle
/// \param  Rank the rank of the matrix
/// \param  BlockSrc the block needed to get the rank ,
///  datatype is F32
/// \return  error code
MRESULT LiRank_F32(MHandle hMemMgr,MLong *rank,PBLOCK BlockSrc)
{ 
MRESULT res=LI_ERR_NONE;
MLong m=BlockSrc->lHeight;
MLong n=BlockSrc->lWidth;
MLong i,j,k,nn,is,js,l,ll,u,v;
MFloat q,d;
BLOCK blockA={0};
MFloat* PdataA;
MFloat* Pdata; 
GO(B_Create(hMemMgr,&blockA,DATA_F32,n,m));
B_Cpy(&blockA,BlockSrc);
Pdata=(MFloat*)BlockSrc->pBlockData;
PdataA=(MFloat*)blockA.pBlockData;
B_Cpy(&blockA,BlockSrc);
nn=m;
nn=m;
if (m>=n) 
{
    nn=n;
}
k=0;
for (l=0; l<=nn-1; l++)
{ 
    q=0.0;
    for (i=l; i<=m-1; i++)
    for (j=l; j<=n-1; j++)
    {
        ll=i*n+j; d=fabs(*(PdataA+i*n+j));
        if (d>q) 
        {
            q=d; is=i; js=j;
        }
    }
    if (q+1.0==1.0)
    {
        *rank=k;
    }
        k=k+1;
        if (is!=l)
        {
            for (j=l; j<=n-1; j++)
            {
                u=l*n+j; v=is*n+j;	
                d=*(PdataA+u); 
                *(PdataA+u)=*(PdataA+v);
                *(PdataA+v)=d;
            }
        }
        if (js!=l)
        { 
            for (i=l; i<=m-1; i++)
            { 
                u=i*n+js; v=i*n+l;
                d=*(PdataA+u); 
                *(PdataA+u)=*(PdataA+v); 
                *(PdataA+v)=d;
            }
        }
        ll=l*n+l;
        for (i=l+1; i<=n-1; i++)
        { 
            d=*(PdataA+i*n+l)/(*(PdataA+ll));
            for (j=l+1; j<=n-1; j++)
            {
                u=i*n+j;
                *(PdataA+u)=*(PdataA+u)-d*(*(PdataA+l*n+j));
            }
        }
}
*rank=k;
EXT:
B_Release(hMemMgr,&blockA);
return res;
}




/// \brief  LiTrace  calculate the trace sum of the block 
/// \param  hMemMgr  Handle
/// \param  trace the sum of the diagonal
/// \param  BlockSrc the block needed to get the trace ,
/// \return  error code
MRESULT LiTrace(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc)
{
    if (BlockSrc->lWidth!=BlockSrc->lHeight)
        {
            JASSERT(MFalse);
            return -1;
    }
    switch(BlockSrc->typeDataA)
    {
    case DATA_I8:
        return LiTrace_I8(hMemMgr,trace,BlockSrc);
    case DATA_F32:
        return LiTrace_F32(hMemMgr,trace,BlockSrc);
    case DATA_U8:
        return LiTrace_U8(hMemMgr,trace,BlockSrc);
    default:
        JASSERT(MFalse);
        return -1;
    }
}
/// \brief  LiTrace_I8  calculate the trace sum of the block 
/// \param  hMemMgr  Handle
/// \param  trace the sum of the diagonal
/// \param  BlockSrc the block needed to get the trace ,
/// datatype is INT8
/// \return  error code
MRESULT LiTrace_I8(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc)
{
    MLong res=LI_ERR_NONE;	
    MLong i=0;
    MLong j=0;
    MInt8 *data;
    MFloat temp=0;
    data=(MInt8*)BlockSrc->pBlockData;
    for(i=0;i<BlockSrc->lHeight;i++)
    {
        temp+=*(data+i+i*BlockSrc->lBlockLine);
    }


    *trace=temp;
    
    
        
    return res;
}

/// \brief  LiTrace_U8  calculate the trace sum of the block 
/// \param  hMemMgr  Handle
/// \param  trace the sum of the diagonal
/// \param  BlockSrc the block needed to get the trace ,
/// datatype is UINT8
/// \return  error code
MRESULT LiTrace_U8(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc)
{
    MLong res=LI_ERR_NONE;	
    MLong i=0;
    MLong j=0;
    MUInt8 *data;
    MFloat temp=0;
    
    data=(MUInt8*)BlockSrc->pBlockData;
    for(i=0;i<BlockSrc->lHeight;i++)
    {
        temp+=*(data+i+i*BlockSrc->lBlockLine);
    }

    *trace=temp;



    return res;
}
/// \brief  LiTrace_F32  calculate the trace sum of the block 
/// \param  hMemMgr  Handle
/// \param  trace the sum of the diagonal
/// \param  BlockSrc the block needed to get the trace ,
/// datatype is F32
/// \return  error code

MRESULT LiTrace_F32(MHandle hMemMgr,MFloat *trace,PBLOCK BlockSrc)
{
    MLong res=LI_ERR_NONE;	
    MLong i=0;
    MLong j=0;
    MFloat *data;
    MFloat temp=0.0;
    
    data=(MFloat*)BlockSrc->pBlockData;
    for(i=0;i<BlockSrc->lHeight;i++)
    {
        temp+=*(data+i+i*BlockSrc->lBlockLine);
    }


    *trace=temp;

    return res;
        return res;
}



/// \brief  LiSigular  calculate the singularities of the Hessenberg matrix 
/// \param BlockR the reality part of the singularities
/// \param BlockV the imaginary part of the singularities
/// \param  BlockSrc the source Hessenberg matrix
///  datatype is float
/// \return  error code
MRESULT LiSigular(PBLOCK BlockR,PBLOCK BlockV, PBLOCK Blocksrc)
{
    MLong res=LI_ERR_NONE;
    MLong m=Blocksrc->lWidth;
    MDouble eps=0.000001;
    MLong itr=60;
    MLong it,i,j,k,l,ii,jj,kk,ll;
    MDouble b,c,w,g,xy,p,q,r,x,s,e,f,z,y;
    MDouble temp,temp1,temp2,temp3;
    MFloat *data;
    MFloat *dataR;
    MFloat *dataV;
    MLong n=Blocksrc->lBlockLine;
    data=(MFloat*)Blocksrc->pBlockData;
    dataR=(MFloat*)BlockR->pBlockData;
    dataV=(MFloat*)BlockV->pBlockData;
    it=0; 
    while (m!=0)
    {
        l=m-1;
        temp=fabs((float)(*(data+l*n+l-1)));
        temp1=fabs((float)(*(data+(l-1)*n+l-1)));
        temp2=fabs((float)(*(data+l*n+l)));
        temp3=eps*(temp1+temp2);
        while((l>0)&&(fabs((float)(*(data+l*n+l-1)))>eps*(fabs((float)(*(data+(l-1)*n+l-1)))+fabs((float)(*(data+l*n+l))))))
        {
            l=l-1;
        }
        ii=(m-1)*n+m-1;
        jj=(m-1)*n+m-2;
        kk=(m-2)*n+m-1; 
        ll=(m-2)*n+m-2;
        if (l==m-1)
        {
            *(dataR+m-1)=*(data+(m-1)*n+m-1);
            *(dataV+m-1)=0.0;
            m=m-1;
            it=0;
        }
    else if (l==m-2)
    {  
        b=-(*(data+ii)+(*(data+ll)));
        c=(*(data+ii))*(*(data+ll))-(*(data+jj))*(*(data+kk));
        w=b*b-4.0*c;
        y=sqrt(fabs(w));
        if (w>0.0)
        { 
            xy=1.0;
            if (b<0.0)
            {
                xy=-1.0;
            }
            *(dataR+m-1)=(-b-(xy*y))/2.0;
            *(dataR+m-2)=c/(*(dataR+m-1));
            *(dataV+m-1)=0.0;
            *(dataV+m-2)=0.0;
        }
        else
        {
            *(dataR+m-1)=-b/2.0;
            *(dataR+m-2)=*(dataR+m-1);
            *(dataV+m-1)=y/2.0;
            *(dataV+m-2)=-(*(dataV+m-1));
            
    }
    m=m-2; it=0;
    }
    else
    { 
        if (it>=itr)
    { 
        printf("fail\n");
        return(-1);
    }
    it=it+1;
    for (j=l+2; j<=m-1; j++)
    {
        *(data+j*n+j-2)=0.0;
    }
    for (j=l+3; j<=m-1; j++)
    {
        *(data+j*n+j-3)=0.0;
    }
        
    for (k=l; k<=m-2; k++)
    { 
        if (k!=l)
        { 
            p=*(data+k*n+k-1);
            q=*(data+(k+1)*n+k-1);
            r=0.0;
    if (k!=m-2)
        {
            r=*(data+(k+2)*n+k-1);
        }
    }

    else
    { 
        x=*(data+ii)+(*(data+ll));
        y=(*(data+ll))*(*(data+ii))-(*(data+kk))*(*(data+jj));
        ii=l*n+l;
        jj=l*n+l+1;
        kk=(l+1)*n+l;
        ll=(l+1)*n+l+1;
        p=(*(data+ii))*(*(data+ii)-x)+(*(data+jj))*(*(data+kk))+y;
        q=(*(data+kk))*(*(data+ii)+(*(data+ll))-x);
        r=(*(data+kk))*(*(data+(l+2)*n+l+1));
    
    }
    if ((fabs(p)+fabs(q)+fabs(r))!=0.0)
    { 
        xy=1.0;
    if (p<0.0) xy=-1.0;
    s=xy*sqrt(p*p+q*q+r*r);
    if (k!=l)
    {
        *(data+k*n+k-1)=-s;
    }
    e=-q/s;
    f=-r/s; 
    x=-p/s;
    y=-x-f*r/(p+s);
    g=e*r/(p+s);
    z=-x-e*q/(p+s);
    for (j=k; j<=m-1; j++)
    {
        ii=k*n+j; 
        jj=(k+1)*n+j;
        p=x*(*(data+ii))+e*(*(data+jj));
        q=e*(*(data+ii))+y*(*(data+jj));
        r=f*(*(data+ii))+g*(*(data+jj));
        if (k!=m-2)
        { 
            kk=(k+2)*n+j;
            p=p+f*(*(data+kk));
            q=q+g*(*(data+kk));
            r=r+z*(*(data+kk));
            *(data+kk)=r;
            
    }
        *(data+jj)=q;
        *(data+ii)=p;
    
    }
    j=k+3;
    if (j>=m-1)
        {
            j=m-1;
        }
    for (i=l; i<=j; i++)
    { 
        ii=i*n+k; 
        jj=i*n+k+1;
        p=x*(*(data+ii))+e*(*(data+jj));
        q=e*(*(data+ii))+y*(*(data+jj));
        r=f*(*(data+ii))+g*(*(data+jj));
    if (k!=m-2)
    { 
        kk=i*n+k+2;
        p=p+f*(*(data+kk));
        q=q+g*(*(data+kk));
        r=r+z*(*(data+kk));
        *(data+kk)=r;
    }
    *(data+jj)=q;
    *(data+ii)=p;
    }
    }
    }
    }
    }
    
    return res;
}


/// \brief  LiHtrans  calculate  Hessenberg matrix 
/// \param BlockA  the matrix needed to be transformed
///  datatype is float
/// \return  error code
MRESULT LiHtrans(PBLOCK BlockA)
{
    if (BlockA->typeDataA !=DATA_F32)
    {
        JASSERT(MFalse);
        return -1;
    }
    if(BlockA->lHeight!=BlockA->lWidth)
    {
        JASSERT(MFalse);
        return -1;
    }
    else
    {
        return LiHtrans_U8(BlockA);
    }
    
}
/// \brief  LiHtrans_U8  calculate  Hessenberg matrix 
/// \param BlockA  the matrix needed to be transformed
///  datatype is UINT8
/// \return  error code

MRESULT LiHtrans_U8(PBLOCK BlockA)
{ 
    MLong res=LI_ERR_NONE;
    MLong  i,j,k,u,v;
    MLong n=BlockA->lWidth;
    MDouble d,t;
    MFloat *data;
    
    data=(MFloat*)BlockA->pBlockData;
    
    for (k=1; k<=n-2; k++)
    { 
        d=0.0;
    for (j=k; j<=n-1; j++)
    { 
        u=j*BlockA->lBlockLine+k-1; 
        t=*(data+u);
        if (fabs(t)>fabs(d))
        {
            d=t; i=j;
        }
    }
    if (fabs(d)+1.0!=1.0)
    {
        if (i!=k)
        {
            for (j=k-1; j<=n-1; j++)
    { 
        u=i*BlockA->lBlockLine+j; v=k*BlockA->lBlockLine+j;
        t=*(data+u);
        *(data+u)=*(data+v);
        *(data+v)=t;

    }
    for (j=0; j<=n-1; j++)
    { 
        u=j*BlockA->lBlockLine+i; v=j*BlockA->lBlockLine+k;
        t=*(data+u);
        *(data+u)=*(data+v);
        *(data+v)=t;
    
    }
    }
    for (i=k+1; i<=n-1; i++)
    { 
        u=i*BlockA->lBlockLine+k-1; 
        t=(*(data+u))/d;
        *(data+u)=0.0;
        
    for (j=k; j<=n-1; j++)
    { 
        v=i*BlockA->lBlockLine+j;
        *(data+v)=*(data+v)-t*(*(data+k*BlockA->lBlockLine+j));
    
    }
    for (j=0; j<=n-1; j++)
    { v=j*BlockA->lBlockLine+k;
    *(data+v)=*(data+v)+t*(*(data+j*BlockA->lBlockLine+i));
    
    }
    }
    }
    }
    return res;

}


/// \brief  Transpose  calculate  the transposition of the matrix
/// \param BlockRes  the matrix after transposition
/// \param BlockSrc the matrix needed to be transposition
/// \return  error code
MRESULT TransPose(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc)
{
    switch(BlockSrc->typeDataA)
    {
    case DATA_I8:
        return TransPose_I8(hMemMgr,BlockRes,BlockSrc);
    case DATA_U8:
        return TransPose_U8(hMemMgr,BlockRes,BlockSrc);
    case DATA_F32:
        return TransPose_F32(hMemMgr,BlockRes,BlockSrc);
    default:
        JASSERT(MFalse);
        return -1;
    }

}

/// \brief  Transpose_I8  calculate  the transposition of the matrix
/// \param BlockRes  the matrix after transposition
/// \param BlockSrc the matrix needed to be transposition
/// data type is INT8
/// \return  error code
MRESULT TransPose_I8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc)
{
    MRESULT res=LI_ERR_NONE;
    MInt8 *datasrc;
    MInt8 *datares;
    MLong i;
    MLong j;
    MLong ExtRes;
    datasrc=(MInt8*)BlockSrc->pBlockData;
    datares=(MInt8*)BlockRes->pBlockData;
    ExtRes=BlockRes->lBlockLine-BlockRes->lWidth;
    for(i=0;i<BlockRes->lHeight;i++,datares+=ExtRes)
    for(j=0;j<BlockRes->lWidth;j++,datares++)
    {
        *(datares)=*(datasrc+i+j*BlockSrc->lBlockLine);
    }


    return res;
}

/// \brief  Transpose_U8  calculate  the transposition of the matrix
/// \param BlockRes  the matrix after transposition
/// \param BlockSrc the matrix needed to be transposition
/// data type is UINT8
/// \return  error code
MRESULT TransPose_U8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc)
{
    MRESULT res=LI_ERR_NONE;
    MUInt8 *datasrc;
    MUInt8 *datares;
    MLong i,j,ExtRes;
    datasrc=(MUInt8*)BlockSrc->pBlockData;
    datares=(MUInt8*)BlockRes->pBlockData;
    ExtRes=BlockRes->lBlockLine-BlockRes->lWidth;
    for(i=0;i<BlockRes->lHeight;i++,datares+=ExtRes)
    for(j=0;j<BlockRes->lWidth;j++,datares++)
    {
        *(datares)=*(datasrc+i+j*BlockSrc->lBlockLine);
    }

        return res;
}

/// \brief  Transpose_F32  calculate  the transposition of the matrix
/// \param BlockRes  the matrix after transposition
/// \param BlockSrc the matrix needed to be transposition
/// data type is F32
/// \return  error code
MRESULT TransPose_F32(MHandle hMemMgr,PBLOCK BlockRes,PBLOCK BlockSrc)
{
    MRESULT res=LI_ERR_NONE;
    MFloat *datasrc;
    MFloat *datares;
    MLong i,j,ExtRes;
    datasrc=(MFloat*)BlockSrc->pBlockData;
    datares=(MFloat*)BlockRes->pBlockData;
    ExtRes=BlockRes->lBlockLine-BlockRes->lWidth;
    for(i=0;i<BlockRes->lHeight;i++,datares+=ExtRes)
    for(j=0;j<BlockRes->lWidth;j++,datares++)
    {
        *(datares)=*(datasrc+i+j*BlockSrc->lBlockLine);
    }

        return res;
}

MRESULT TransPoseEXT(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc)
{
    switch(BlockSrc->block.typeDataA)
    {
    case DATA_I8:
        return TransPoseEXT_I8(hMemMgr,BlockRes,BlockSrc);
    case DATA_U8:
        return TransPoseEXT_U8(hMemMgr,BlockRes,BlockSrc);
    case DATA_F32:
        return TransPoseEXT_F32(hMemMgr,BlockRes,BlockSrc);
    default:
        JASSERT(MFalse);
        return -1;
    }

}

MRESULT TransPoseEXT_I8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc)
{
    MRESULT res=LI_ERR_NONE;
    MInt8 *datasrc;
    MInt8 *datares;
    MLong i;
    MLong j;
    MLong ExtRes;
    datasrc=(MInt8*)BlockSrc->block.pBlockData+(BlockSrc->ext.top)*BlockSrc->block.lBlockLine+BlockSrc->ext.left;
    datares=(MInt8*)BlockRes->pBlockData;
    ExtRes=BlockRes->lBlockLine-BlockRes->lWidth;
    for(i=0;i<BlockRes->lHeight;i++,datares+=ExtRes)
        for(j=0;j<BlockRes->lWidth;j++,datares++)
        {
            *(datares)=*(datasrc+i+j*BlockSrc->block.lBlockLine);
        }


        return res;
}

MRESULT TransPoseEXT_U8(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc)
{
    MRESULT res=LI_ERR_NONE;
    MUInt8 *datasrc;
    MUInt8 *datares;
    MLong i,j,ExtRes;
    datasrc=(MUInt8*)BlockSrc->block.pBlockData+(BlockSrc->ext.top)*BlockSrc->block.lBlockLine+BlockSrc->ext.left;
    datares=(MUInt8*)BlockRes->pBlockData;
    ExtRes=BlockRes->lBlockLine-BlockRes->lWidth;
    for(i=0;i<BlockRes->lHeight;i++,datares+=ExtRes)
    for(j=0;j<BlockRes->lWidth;j++,datares++)
        {
            *(datares)=*(datasrc+i+j*BlockSrc->block.lBlockLine);
        }

        return res;
}

MRESULT TransPoseEXT_F32(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc)
{
    MRESULT res=LI_ERR_NONE;
    MFloat *datasrc;
    MFloat *datares;
    MLong i,j,ExtRes;
    datasrc=(MFloat*)BlockSrc->block.pBlockData+(BlockSrc->ext.top)*BlockSrc->block.lBlockLine+BlockSrc->ext.left;
    datares=(MFloat*)BlockRes->pBlockData;
    ExtRes=BlockRes->lBlockLine-BlockRes->lWidth;
    for(i=0;i<BlockRes->lHeight;i++,datares+=ExtRes)
        for(j=0;j<BlockRes->lWidth;j++,datares++)
        {
            *(datares)=*(datasrc+i+j*BlockSrc->block.lBlockLine);
        }

        return res;
}

/// \brief  Litrmul  calculate  the  product of the two matrix
/// \param blockres  the product matrix 
/// \param blocksrc1 the matrix needed to be calculate
/// \param blocksrc2 the matrix needed to be calculate
/// \return  error code
MRESULT Litrmul(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
   if (blocksrc1->typeDataA!=blocksrc2->typeDataA)
   {
       JASSERT(MFalse);
       return -1;
   }
   if(blocksrc1->typeDataA==DATA_I8)
   {
       return Litrmul_I8(blockres,blocksrc1,blocksrc2);
   }
   if (blocksrc1->typeDataA==DATA_U8)
   {
       return Litrmul_U8(blockres,blocksrc1,blocksrc2);
   }
   if(blocksrc1->typeDataA==DATA_F32)
   {
       return Litrmul_F32(blockres,blocksrc1,blocksrc2);
   }
    
    
    return res;
}

/// \brief  Litrmul_I8  calculate  the  product of the two matrix
/// \param blockres  the product matrix 
/// \param blocksrc1 the matrix needed to be calculate
/// datatype is INT8
/// \param blocksrc2 the matrix needed to be calculate
/// \return  error code
MRESULT Litrmul_I8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MInt8* datares;
    MInt8* datasrc1;
    MInt8* datasrc2;
    MLong i,j,l,Extsrc1,Extsrc2,Extres;
    datares=(MInt8*)blockres->pBlockData;
    datasrc1=(MInt8*)blocksrc1->pBlockData;
    datasrc2=(MInt8*)blocksrc2->pBlockData;
    Extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    Extsrc2=blocksrc2->lBlockLine-blocksrc2->lWidth;
    Extres=blockres->lBlockLine-blockres->lWidth;

    for (i=0;i<blockres->lHeight;i++,datares+=Extres)
    for(j=0;j<blockres->lWidth;j++,datares++)
    {
        *(datares)=0.0;

        for(l=0;l<blocksrc1->lWidth;l++)
        {
            *(datares)=*(datares)+*(datasrc1+i*blocksrc1->lBlockLine+l)*(*(datasrc2+l*blocksrc2->lBlockLine+j));
        }
    }
    
    return res;
}

/// \brief  Litrmul  calculate  the  product of the two matrix
/// \param blockres  the product matrix 
/// \param blocksrc1 the matrix needed to be calculate
/// datatype is UINT8
/// \param blocksrc2 the matrix needed to be calculate
/// \return  error code
MRESULT Litrmul_U8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MUInt8* datares;
    MUInt8* datasrc1;
    MUInt8* datasrc2;
    MLong i,j,l,Extsrc1,Extsrc2,Extres;
    datares=(MUInt8*)blockres->pBlockData;
    datasrc1=(MUInt8*)blocksrc1->pBlockData;
    datasrc2=(MUInt8*)blocksrc2->pBlockData;
    Extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    Extsrc2=blocksrc2->lBlockLine-blocksrc2->lWidth;
    Extres=blockres->lBlockLine-blockres->lWidth;

    for (i=0;i<blockres->lHeight;i++,datares+=Extres)
    for(j=0;j<blockres->lWidth;j++,datares++)
    {
        *(datares)=0.0;
        for(l=0;l<blocksrc1->lWidth;l++)
        {
            *(datares)=*(datares)+*(datasrc1+i*blocksrc1->lBlockLine+l)*(*(datasrc2+l*blocksrc2->lBlockLine+j));
        }
    }
    return res;
}


/// \brief  Litrmul_F32  calculate  the  product of the two matrix
/// \param blockres  the product matrix 
/// \param blocksrc1 the matrix needed to be calculate
/// datatype is F32
/// \param blocksrc2 the matrix needed to be calculate
/// \return  error code
MRESULT Litrmul_F32(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MFloat* datares;
    MFloat* datasrc1;
    MFloat* datasrc2;
    MLong i,j,l,Extsrc1,Extsrc2,Extres;
    datares=(MFloat*)blockres->pBlockData;
    datasrc1=(MFloat*)blocksrc1->pBlockData;
    datasrc2=(MFloat*)blocksrc2->pBlockData;
    
    Extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    Extsrc2=blocksrc2->lBlockLine-blocksrc2->lWidth;
    Extres=blockres->lBlockLine-blockres->lWidth;

    for (i=0;i<blockres->lHeight;i++,datares+=Extres)
    for(j=0;j<blockres->lWidth;j++,datares++)
    {
        *(datares)=0.0;
    
        for(l=0;l<blocksrc1->lWidth;l++)
        {
            *(datares)=*(datares)+*(datasrc1+i*blocksrc1->lBlockLine+l)*(*(datasrc2+l*blocksrc2->lBlockLine+j));
        }
    }
        return res;
}

MRESULT LitrmulEXT(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2)
{
    MLong res=LI_ERR_NONE;
    if (blocksrc1->block.typeDataA!=blocksrc2->block.typeDataA)
    {
        JASSERT(MFalse);
        return -1;
    }
    if(blocksrc1->block.typeDataA==DATA_I8)
    {
        return LitrmulEXT_I8(blockres,blocksrc1,blocksrc2);
    }
    if (blocksrc1->block.typeDataA==DATA_U8)
    {
        return LitrmulEXT_U8(blockres,blocksrc1,blocksrc2);
    }
    if(blocksrc1->block.typeDataA==DATA_F32)
    {
        return LitrmulEXT_F32(blockres,blocksrc1,blocksrc2);
    }


    return res;
}

MRESULT LitrmulEXT_I8(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MInt8* datares;
    MInt8* datasrc1;
    MInt8* datasrc2;
    MLong i,j,l,Extsrc1,Extsrc2,Extres;
    datares=(MInt8*)blockres->pBlockData;
    datasrc1=(MInt8*)blocksrc1->block.pBlockData+(blocksrc1->ext.top)*(blocksrc1->block.lBlockLine)+blocksrc1->ext.left;
    datasrc2=(MInt8*)blocksrc2->block.pBlockData+(blocksrc2->ext.top)*(blocksrc1->block.lBlockLine)+blocksrc2->ext.left;
    Extsrc1=blocksrc1->block.lBlockLine-blocksrc1->ext.right;
    Extsrc2=blocksrc2->block.lBlockLine-blocksrc2->ext.right;
    Extres=blockres->lBlockLine-blockres->lWidth;

    for (i=0;i<blockres->lHeight;i++,datares+=Extres)
        for(j=0;j<blockres->lWidth;j++,datares++)
        {
            *(datares)=0.0;

            for(l=0;l<(blocksrc1->ext.right-blocksrc1->ext.left);l++)
            {
                *(datares)=*(datares)+*(datasrc1+i*blocksrc1->block.lBlockLine+l)*(*(datasrc2+l*blocksrc2->block.lBlockLine+j));
            }
        }

        return res;
}

MRESULT LitrmulEXT_U8(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MUInt8* datares;
    MUInt8* datasrc1;
    MUInt8* datasrc2;
    MLong i,j,l,Extsrc1,Extsrc2,Extres;
    datares=(MUInt8*)blockres->pBlockData;
    datasrc1=(MUInt8*)blocksrc1->block.pBlockData+(blocksrc1->ext.top)*(blocksrc1->block.lBlockLine)+blocksrc1->ext.left;
    datasrc2=(MUInt8*)blocksrc2->block.pBlockData+(blocksrc2->ext.top)*(blocksrc1->block.lBlockLine)+blocksrc2->ext.left;
    Extsrc1=blocksrc1->block.lBlockLine-blocksrc1->ext.right;
    Extsrc2=blocksrc2->block.lBlockLine-blocksrc2->ext.right;
    Extres=blockres->lBlockLine-blockres->lWidth;

    for (i=0;i<blockres->lHeight;i++,datares+=Extres)
    for(j=0;j<blockres->lWidth;j++,datares++)
    {
            *(datares)=0.0;
            for(l=0;l<(blocksrc1->ext.right-blocksrc1->ext.left);l++)
            {
                *(datares)=*(datares)+*(datasrc1+i*blocksrc1->block.lBlockLine+l)*(*(datasrc2+l*blocksrc2->block.lBlockLine+j));
            }
        }
        return res;
}




MRESULT LitrmulEXT_F32(PBLOCK blockres,PBLOCKEXT blocksrc1,PBLOCKEXT blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MFloat* datares;
    MFloat* datasrc1;
    MFloat* datasrc2;
    MLong i,j,l,Extsrc1,Extsrc2,Extres;
    datares=(MFloat*)blockres->pBlockData;
    datasrc1=(MFloat*)blocksrc1->block.pBlockData+(blocksrc1->ext.top)*(blocksrc1->block.lBlockLine)+blocksrc1->ext.left;
    datasrc2=(MFloat*)blocksrc2->block.pBlockData+(blocksrc2->ext.top)*(blocksrc1->block.lBlockLine)+blocksrc2->ext.left;
    Extsrc1=blocksrc1->block.lBlockLine-blocksrc1->ext.right;
    Extsrc2=blocksrc2->block.lBlockLine-blocksrc2->ext.right;
    Extres=blockres->lBlockLine-blockres->lWidth;

    for (i=0;i<blockres->lHeight;i++,datares+=Extres)
    for(j=0;j<blockres->lWidth;j++,datares++)
    {
        *(datares)=0.0;
        for(l=0;l<(blocksrc1->ext.right-blocksrc1->ext.left);l++)
        {
            *(datares)=*(datares)+*(datasrc1+i*blocksrc1->block.lBlockLine+l)*(*(datasrc2+l*blocksrc2->block.lBlockLine+j));
        }
    }
        return res;
}





/// \brief  LiImTcol  Rearrange image blocks into columns.
/// \param blockres  the  rearranged image block
/// \param blocksrc the matrix needed to be rearranged
/// \param width the block width to be used
/// \param height the block height to be used
/// \return  error code
MRESULT LiImTcol(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height)
{
    MLong res=LI_ERR_NONE;
    MLong srcwidth,srcheight;
    srcheight=blocksrc->ext.bottom-blocksrc->ext.top;
    srcwidth=blocksrc->ext.right-blocksrc->ext.left;
    if (width>srcwidth||height>srcheight)
    {
    res=LI_ERR_INVALID_PARAM;
    return res;
    }
    if(blockres->lWidth!=(srcwidth-width+1)*(srcheight-height+1)||blockres->lHeight!=width*height)
    {
        res=LI_ERR_DATA_UNSUPPORT;
        return res;
    }
    if(blockres->typeDataA!=blocksrc->block.typeDataA)
    {
        res=LI_ERR_DATA_UNSUPPORT;
        return res;
    }
    
    switch(blocksrc->block.typeDataA)
    {
    case DATA_I8:
        return LiImTcol_I8(hMemMgr, blockres, blocksrc, width, height);
    case DATA_U8:
        return  LiImTcol_U8(hMemMgr, blockres, blocksrc, width, height);
    case DATA_F32:
        return  LiImTcol_F32(hMemMgr, blockres, blocksrc, width, height);
    default:
        JASSERT(MFalse);
        return -1;
    }
}

/// \brief  LiImTcol_I8  Rearrange image blocks into columns.
/// \param blockres  the  rearranged image block
/// \param blocksrc the matrix needed to be rearranged
/// datatype is I8
/// \param width the block width to be used
/// \param height the block height to be used
/// \return  error code
MRESULT LiImTcol_I8(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height)
{
    MLong res=LI_ERR_NONE;
    MLong i,j,n,m,n1,m1,n2,m2;
    MInt8* data;
    MInt8* datasrc;
    MLong Ext;
    m=blocksrc->ext.right-blocksrc->ext.left-width+1;
    n=blocksrc->ext.bottom-blocksrc->ext.top-height+1;
    data=(MInt8*)blockres->pBlockData;
    datasrc=(MInt8*)blocksrc->block.pBlockData+blocksrc->ext.left+(blocksrc->ext.top)*(blocksrc->block.lBlockLine);
    Ext=blockres->lBlockLine-blockres->lWidth;
    for(i=0;i<blockres->lHeight;i++,data+=Ext)
    for(j=0;j<blockres->lWidth;j++,data++)
    {
        n1=floor((float )(i/m));
        m1=i%m;
        n2=floor((float)(j/width));
        m2=j%width;
        *(data)=*(datasrc+m1*(blocksrc->block.lBlockLine)+n1+m2*blocksrc->block.lBlockLine+n2);
    }
    
    return res;

}

/// \brief  LiImTcol_U8  Rearrange image blocks into columns.
/// \param blockres  the  rearranged image block
/// \param blocksrc the matrix needed to be rearranged
/// datatype is UINT8
/// \param width the block width to be used
/// \param height the block height to be used
/// \return  error code
MRESULT LiImTcol_U8(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height)
{
    MLong res=LI_ERR_NONE;
    MLong i,j,n,m,n1,n2,m1,m2;
    MUInt8* data;
    MUInt8* datasrc;
    MLong Ext;
    m=blocksrc->ext.right-blocksrc->ext.left-width+1;
    n=blocksrc->ext.bottom-blocksrc->ext.top-height+1;
    data=(MUInt8*)blockres->pBlockData;
    datasrc=(MUInt8*)blocksrc->block.pBlockData+blocksrc->ext.left+(blocksrc->ext.top)*(blocksrc->block.lBlockLine);
    Ext=blockres->lBlockLine-blockres->lWidth;
    for(i=0;i<blockres->lHeight;i++,data+=Ext)
    for(j=0;j<blockres->lWidth;j++,data++)
    {
        n1=floor((float )(i/m));
        m1=i%m;
        n2=floor((float)(j/width));
        m2=j%width;
        *(data)=*(datasrc+m1*(blocksrc->block.lBlockLine)+n1+m2*blocksrc->block.lBlockLine+n2);
    }
        return res;

}

/// \brief  LiImTcol_F32  Rearrange image blocks into columns.
/// \param blockres  the  rearranged image block
/// \param blocksrc the matrix needed to be rearranged
/// datatype is F32
/// \param width the block width to be used
/// \param height the block height to be used
/// \return  error code
MRESULT LiImTcol_F32(MHandle hMemMgr,PBLOCK blockres,PBLOCKEXT blocksrc,MLong width,MLong height)
{
    MLong res=LI_ERR_NONE;
    MLong i,j,n,m,n1,n2,m1,m2;
    MFloat* data;
    MFloat* datasrc;
    MLong Ext;
    m=blocksrc->ext.right-blocksrc->ext.left-width+1;
    n=blocksrc->ext.bottom-blocksrc->ext.top-height+1;
    data=(MFloat*)blockres->pBlockData;
    datasrc=(MFloat*)blocksrc->block.pBlockData+blocksrc->ext.left+(blocksrc->ext.top)*(blocksrc->block.lBlockLine);
    Ext=blockres->lBlockLine-blockres->lWidth;
    for(i=0;i<blockres->lHeight;i++,data+=Ext)
    for(j=0;j<blockres->lWidth;j++,data++)
    {
        n1=floor((float )(i/m));
        m1=i%m;
        n2=floor((float)(j/width));
        m2=j%width;
        *(data)=*(datasrc+m1*(blocksrc->block.lBlockLine)+n1+m2*blocksrc->block.lBlockLine+n2);
    
    }
        return res;

}


/// \brief  LiVertCat  Vertical concatenation.
/// \param blockres  the  concatenation block
/// \param blocksrc1 source block
///\\param blocksrc2 source block
/// \return  error code
MRESULT LiVertCat(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
  if (blocksrc1->lWidth!=blocksrc2->lWidth||blockres->lWidth!=blocksrc1->lWidth)
  {
      res=LI_ERR_DATA_UNSUPPORT;
      return res;
  }
  if(blocksrc1->typeDataA!=blocksrc2->typeDataA||blocksrc1->typeDataA!=blockres->typeDataA)
  {
      res=LI_ERR_DATA_UNSUPPORT;
      return res;
  }
  if (blockres->lHeight!=blocksrc1->lHeight+blocksrc2->lHeight)
 {
     res=LI_ERR_USER_ABORT;
     return res;
  }
  switch(blocksrc1->typeDataA)
  {
  case DATA_I8:
      return LiVertCat_I8(blockres,blocksrc1,blocksrc2);
  case DATA_U8:
      return  LiVertCat_U8(blockres,blocksrc1,blocksrc2);
  case DATA_F32:
      return LiVertCat_F32(blockres,blocksrc1,blocksrc2);
  default:
      JASSERT(MFalse);
      return -1;
  }

}

/// \brief  LiVertCat_I8  Vertical concatenation.
/// \param blockres  the  concatenation block
/// \param blocksrc1 source block
/// datatype is INT8
///\\param blocksrc2 source block
/// \return  error code
MRESULT LiVertCat_I8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MLong i,j;
    MInt8 *datares;
    MInt8 *datasrc1;
    MInt8 *datasrc2;
    MLong extsrc1,extsrc2,extres;
    datasrc1=(MInt8*)blocksrc1->pBlockData;
    datasrc2=(MInt8*)blocksrc2->pBlockData;
    datares=(MInt8*)blockres->pBlockData;
    extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    extsrc2=blocksrc2->lBlockLine-blocksrc2->lWidth;
    extres=blockres->lBlockLine-blockres->lWidth;
    for(i=0;i<blocksrc1->lHeight;i++,datares+=extres,datasrc1+=extsrc1)
    for(j=0;j<blockres->lWidth;j++,datares++,datasrc1++)
    {
        *(datares)=*(datasrc1);
    }
    datares=(MInt8*)blockres->pBlockData+(blocksrc1->lHeight)*(blockres->lBlockLine);
    for(i=0;i<blocksrc2->lHeight;i++,datasrc2+=extsrc2,datares+=extres)
    for(j=0;j<blockres->lWidth;j++,datares++,datasrc2++)
    {
        *(datares)=*(datasrc2);
    }
    
    return res;
}

/// \brief  LiVertCat_U8  Vertical concatenation.
/// \param blockres  the  concatenation block
/// \param blocksrc1 source block
/// datatype is UINT8
///\\param blocksrc2 source block
/// \return  error code
MRESULT LiVertCat_U8(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MLong i,j;
    MUInt8 *datares;
    MUInt8 *datasrc1;
    MUInt8 *datasrc2;
    MLong extsrc1,extsrc2,extres;
    datasrc1=(MUInt8*)blocksrc1->pBlockData;
    datasrc2=(MUInt8*)blocksrc2->pBlockData;
    datares=(MUInt8*)blockres->pBlockData;
    extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    extsrc2=blocksrc2->lBlockLine-blocksrc2->lWidth;
    extres=blockres->lBlockLine-blockres->lWidth;
    for(i=0;i<blocksrc1->lHeight;i++,datares+=extres,datasrc1+=extsrc1)
    for(j=0;j<blockres->lWidth;j++,datares++,datasrc1++)
    {
        *(datares)=*(datasrc1);
    }
    datares=(MUInt8*)blockres->pBlockData+(blocksrc1->lHeight)*(blockres->lBlockLine);
    for(i=0;i<blocksrc2->lHeight;i++,datasrc2+=extsrc2,datares+=extres)
    for(j=0;j<blockres->lWidth;j++,datares++,datasrc2++)
    {
        *(datares)=*(datasrc2);
    }
    return res;
}

/// \brief  LiVertCat_F32  Vertical concatenation.
/// \param blockres  the  concatenation block
/// \param blocksrc1 source block
/// datatype is F32
///\\param blocksrc2 source block
/// \return  error code
MRESULT LiVertCat_F32(PBLOCK blockres,PBLOCK blocksrc1,PBLOCK blocksrc2)
{
    MLong res=LI_ERR_NONE;
    MLong i,j;
    MFloat *datares;
    MFloat *datasrc1;
    MFloat *datasrc2;
    MLong extsrc1,extsrc2,extres;
    datasrc1=(MFloat*)blocksrc1->pBlockData;
    datasrc2=(MFloat*)blocksrc2->pBlockData;
    datares=(MFloat*)blockres->pBlockData;
    extsrc1=blocksrc1->lBlockLine-blocksrc1->lWidth;
    extsrc2=blocksrc2->lBlockLine-blocksrc2->lWidth;
    extres=blockres->lBlockLine-blockres->lWidth;
    for(i=0;i<blocksrc1->lHeight;i++,datares+=extres,datasrc1+=extsrc1)
    for(j=0;j<blockres->lWidth;j++,datares++,datasrc1++)
    {
            *(datares)=*(datasrc1);
    }
    datares=(MFloat*)blockres->pBlockData+(blocksrc1->lHeight)*(blockres->lBlockLine);
    for(i=0;i<blocksrc2->lHeight;i++,datasrc2+=extsrc2,datares+=extres)
    for(j=0;j<blockres->lWidth;j++,datares++,datasrc2++)
    {
        *(datares)=*(datasrc2);
    }
    return res;
}


/// \brief  LiSum  calculate the sum of the matrix.
/// \param sum the sum of the matrix
/// \param blocksrc source block
/// \return  error code
MRESULT LiSum(MFloat *sum,PBLOCK blocksrc)
{
    MLong res=LI_ERR_NONE;
    switch(blocksrc->typeDataA)
    {
    case DATA_I8:
        return LiSum_I8(sum,blocksrc);
    case DATA_U8:
        return LiSum_U8(sum,blocksrc); 
    case DATA_F32:
        return LiSum_F32(sum,blocksrc);
    default:
        JASSERT(MFalse);
        return -1;
    }
    return res;
}

/// \brief  LiSum_I8  calculate the sum of the matrix.
/// \param sum the sum of the matrix
/// \param blocksrc source block
///  datatype is INT8
/// \return  error code
MRESULT LiSum_I8(MFloat *sum,PBLOCK blocksrc)
{
    MLong res=LI_ERR_NONE;
    MInt8* data;
    MFloat temp=0.0;
    MLong i,j;
    MLong Ext;
    data=(MInt8*)blocksrc->pBlockData;
    Ext=blocksrc->lBlockLine-blocksrc->lWidth;
    for(i=0;i<blocksrc->lHeight;i++,data+=Ext)
    for(j=0;j<blocksrc->lWidth;j++,data++)
    {
        temp+=*(data);
    }
    
    *sum=temp;
    return res;
}

/// \brief  LiSum_U8  calculate the sum of the matrix.
/// \param sum the sum of the matrix
/// \param blocksrc source block
///  datatype is UINT8
/// \return  error code
MRESULT LiSum_U8(MFloat *sum,PBLOCK blocksrc)
{
    MLong res=LI_ERR_NONE;
    MUInt8* data;
    MFloat temp=0.0;
    MLong i,j;
    MLong Ext;
    data=(MUInt8*)blocksrc->pBlockData;
    Ext=blocksrc->lBlockLine-blocksrc->lWidth;
    for(i=0;i<blocksrc->lHeight;i++,data+=Ext)
    for(j=0;j<blocksrc->lWidth;j++,data++)
    {
        temp+=*(data);
    }
        *sum=temp;
        return res;
}

/// \brief  LiSum_F32  calculate the sum of the matrix.
/// \param sum the sum of the matrix
/// \param blocksrc source block
///  datatype is F32
/// \return  error code
MRESULT LiSum_F32(MFloat *sum,PBLOCK blocksrc)
{
    MLong res=LI_ERR_NONE;
    MFloat* data;
    MFloat temp=0.0;
    MLong i,j;
    MLong Ext;
    data=(MFloat*)blocksrc->pBlockData;
    Ext=blocksrc->lBlockLine-blocksrc->lWidth;
    for(i=0;i<blocksrc->lHeight;i++,data+=Ext)
    for(j=0;j<blocksrc->lWidth;j++,data++)
    {
        temp+=*(data);
    }

        *sum=temp;
        return res;
}

/// \brief  LiSVD  calculate the svd of the matrix.
/// \param hMemMgr Handle
/// \param blockres  the svd of the matrix
/// \param blocksrc source block
/// \return  error code
MRESULT LiSVD(MHandle hMemMgr,PBLOCK blockres,PBLOCK blocksrc)
{
    MLong res=LI_ERR_NONE;
    MDouble *datasrc;
    MDouble* datau;
    MDouble* datav;
    MDouble* datas;
    MLong m,n,ka,n1;
    MDouble eps=0.000001;
    MLong  i,j,k,l,it,ll,kk,ix,iy,mm,nn,iz,m1,ks;
    MDouble d,dd,t,sm,sm1,em1,sk,ek,b,c,shh,fg[2],cs[2];
    MLong maxs;
    MDouble*s,*e,*w;
    MLong ws,hs,wu,hu,wv,hv;
    MLong extres,extsrc;
    ws=blocksrc->lWidth;
    hs=blocksrc->lHeight;
    wu=blocksrc->lHeight;
    hu=blocksrc->lHeight;
    wv=blocksrc->lWidth;
    hv=blocksrc->lWidth;
    m=blocksrc->lHeight;
    n=blocksrc->lWidth;
    n1=blocksrc->lBlockLine;
    maxs=MAX(blocksrc->lWidth,blocksrc->lHeight);
    datas=(MDouble*)blockres->pBlockData;
    datau=(MDouble*)blockres->pBlockData+maxs*blockres->lBlockLine;
    datav=(MDouble*)blockres->pBlockData+2*maxs*blockres->lBlockLine;
    datasrc=(MDouble*)blocksrc->pBlockData;
    ka=MAX(m,n)+100;	
    AllocVectMem(hMemMgr,s,ka,MDouble);
    AllocVectMem(hMemMgr,e,ka,MDouble);
    AllocVectMem(hMemMgr,w,ka,MDouble);
    extres=blockres->lBlockLine-blockres->lWidth;
    extsrc=blocksrc->lBlockLine-blocksrc->lWidth;
    
    for(i=0;i<blocksrc->lHeight;i++,datasrc+=extsrc,datas+=extres)
  {
      for(j=0;j<blockres->lWidth;j++,datas++,datasrc++)
      {
            *(datas)=(*(datasrc));
      }
   }
    datas=(MDouble*)blockres->pBlockData;
    datasrc=(MDouble*)blocksrc->pBlockData;
        it=60; k=n;
        if (m-1<n)
        {
            k=m-1;
        }
        l=m;
        if (n-2<m) 
        {
            l=n-2;
        }
        if (l<0) 
        {
            l=0;
        }
        ll=k;
        if (l>k)
        {
            ll=l;
        }
        if (ll>=1)
        { 
            for (kk=1; kk<=ll; kk++)
            {
                if (kk<=k)
                {
                    d=0.0;
                    for (i=kk; i<=m; i++)
                    { 
                        ix=(i-1)*n1+kk-1;
                        d=d+(*(datas+ix))*(*(datas+ix));
                    }
                    s[kk-1]=sqrt(d);
                    if (s[kk-1]!=0.0)
                    { 
                        ix=(kk-1)*n1+kk-1;
                        if (*(datas+ix)!=0.0)
                        { 
                            s[kk-1]=fabs(s[kk-1]);
                            if (*(datas+ix)<0.0) 
                            {
                                s[kk-1]=-s[kk-1];
                            }
                        }
                        for (i=kk; i<=m; i++)
                        { 
                            iy=(i-1)*n1+kk-1;
                            *(datas+iy)=*(datas+iy)/s[kk-1];
                        }
                        *(datas+ix)=1.0+(*(datas+ix));
                    }
                    s[kk-1]=-s[kk-1];
                }

                if (n>=kk+1)
                {
                    for (j=kk+1; j<=n; j++)
                    { 
                        if ((kk<=k)&&(s[kk-1]!=0.0))
                        { 
                            d=0.0;
                            for (i=kk; i<=m; i++)
                            { 
                                ix=(i-1)*n1+kk-1;
                                iy=(i-1)*n1+j-1;
                                d=d+(*(datas+ix))*(*(datas+iy));
                            }
                            d=-d/(*(datas+(kk-1)*n1+kk-1));
                            for (i=kk; i<=m; i++)
                            { 
                                ix=(i-1)*n1+j-1;
                                iy=(i-1)*n1+kk-1;
                                *(datas+ix)=*(datas+ix)+d*(*(datas+iy));
                            }
                        }
                        e[j-1]=*(datas+(kk-1)*n1+j-1);

                    }
                }
                if (kk<=k)
                {
                    for (i=kk; i<=m; i++)

                    { 
                        ix=(i-1)*blockres->lBlockLine+kk-1; iy=(i-1)*n1+kk-1;
                        *(datau+ix)=*(datas+iy);
                    }
                }
                if (kk<=l)
                { 
                    d=0.0;
                    for (i=kk+1; i<=n; i++)

                    {
                        d=d+e[i-1]*e[i-1];
                    }
                    e[kk-1]=sqrt(d);
                    if (e[kk-1]!=0.0)
                    { 
                        if (e[kk]!=0.0)
                        { 
                            e[kk-1]=fabs(e[kk-1]);
                            if (e[kk]<0.0) 
                            {
                                e[kk-1]=-e[kk-1];
                            }
                        }
                        for (i=kk+1; i<=n; i++)
                        {
                            e[i-1]=e[i-1]/e[kk-1];
                        }
                        e[kk]=1.0+e[kk];
                    }
                    e[kk-1]=-e[kk-1];
                    if ((kk+1<=m)&&(e[kk-1]!=0.0))
                    {
                        for (i=kk+1; i<=m; i++) 
                        {
                            w[i-1]=0.0;
                        }

                        for (j=kk+1; j<=n; j++)
                            for (i=kk+1; i<=m; i++)
                            {
                                w[i-1]=w[i-1]+e[j-1]*(*(datas+(i-1)*n1+j-1));
                            }
                            for (j=kk+1; j<=n; j++)
                                for (i=kk+1; i<=m; i++)
                                { 
                                    ix=(i-1)*n1+j-1;
                                    *(datas+ix)=*(datas+ix)-w[i-1]*e[j-1]/e[kk];
                                }
                    }
                    for (i=kk+1; i<=n; i++)
                    {
                        *(datav+(i-1)*blockres->lBlockLine+kk-1)=e[i-1];
                    }
                }
            }
        }
        mm=n;
        if (m+1<n) 
        {
            mm=m+1;
        }
        if (k<n) 
        {
            s[k]=*(datas+k*n1+k);
        }
        if (m<mm) 
        {
            s[mm-1]=0.0;
        }
        if (l+1<mm) 
        {
            e[l]=*(datas+l*n1+mm-1);
        }
        e[mm-1]=0.0;
        nn=m;
        if (m>n)
        {
            nn=n;
        }
        if (nn>=k+1)
        { 
            for (j=k+1; j<=nn; j++)
                for (i=1; i<=m; i++)
                {
                    *(datau+(i-1)*blockres->lBlockLine+j-1)=0.0;
                    *(datau+(j-1)*blockres->lBlockLine+j-1)=1.0;
                }
        }

        if (k>=1)
        {
            for (ll=1; ll<=k; ll++)
            {
                kk=k-ll+1;
                iz=(kk-1)*blockres->lBlockLine+kk-1;
                if (s[kk-1]!=0.0)
                { 
                    if (nn>=kk+1)
                    {
                        for (j=kk+1; j<=nn; j++)
                        {
                            d=0.0;
                            for (i=kk; i<=m; i++)
                            { 
                                ix=(i-1)*blockres->lBlockLine+kk-1;
                                iy=(i-1)*blockres->lBlockLine+j-1;
                                d=d+(*(datau+ix))*(*(datau+iy))/(*(datau+iz));
                            }
                            d=-d;
                            for (i=kk; i<=m; i++)
                            { 
                                ix=(i-1)*blockres->lBlockLine+j-1;
                                iy=(i-1)*blockres->lBlockLine+kk-1;
                                *(datau+ix)=*(datau+ix)+d*(*(datau+iy));
                            }
                        }
                    }
                    for (i=kk; i<=m; i++)
                    {
                        ix=(i-1)*blockres->lBlockLine+kk-1; 
                        *(datau+ix)=-*(datau+ix);
                    }
                    *(datau+iz)=1.0+*(datau+iz);
                    if (kk-1>=1)
                    {
                        for (i=1; i<=kk-1; i++)
                        {
                            *(datau+(i-1)*blockres->lBlockLine+kk-1)=0;
                        }
                    }
                }
                else
                { 
                    for (i=1; i<=m; i++)
                    {
                        *(datau+(i-1)*blockres->lBlockLine+kk-1)=0.0;
                        *(datau+(kk-1)*blockres->lBlockLine+kk-1)=1.0;
                    }
                }
            }
        }
        for (ll=1; ll<=n; ll++)
        {
            kk=n-ll+1; 
            iz=kk*blockres->lBlockLine+kk-1;
            if ((kk<=l)&&(e[kk-1]!=0.0))
            { 
                for (j=kk+1; j<=n; j++)
                {
                    d=0.0;
                    for (i=kk+1; i<=n; i++)
                    { 
                        ix=(i-1)*blockres->lBlockLine+kk-1;
                        iy=(i-1)*blockres->lBlockLine+j-1;
                        d=d+(*(datav+ix))*(*(datav+iy))/(*(datav+iz));
                    }
                    d=-d;
                    for (i=kk+1; i<=n; i++)
                    { 
                        ix=(i-1)*blockres->lBlockLine+j-1; 
                        iy=(i-1)*blockres->lBlockLine+kk-1;
                        *(datav+ix)=*(datav+ix)+d*(*(datav+iy));
                    }
                }
            }
            for (i=1; i<=n; i++)
            {
                *(datav+(i-1)*blockres->lBlockLine+kk-1)=0.0;
            }
            *(datav+iz-blockres->lBlockLine)=1.0;
        }
        for (i=1; i<=m; i++)
            for (j=1; j<=n; j++)
            {
                *(datas+(i-1)*n1+j-1)=0.0;
            }
            m1=mm; 
            it=60;
            while (1==1)
            { 
                if (mm==0)
                { 
                    ppp(blockres,e,s,(hs+hu),hv,wv,m,n);

                    FreeVectMem(hMemMgr,s);
                    FreeVectMem(hMemMgr,w);
                    FreeVectMem(hMemMgr,e);

                    return 0;
                }
                if (it==0)
                {
                    FreeVectMem(hMemMgr,s);
                    FreeVectMem(hMemMgr,w);
                    FreeVectMem(hMemMgr,e);
                    return(-1);
                }
                kk=mm-1;
                while ((kk!=0)&&(fabs(e[kk-1])!=0.0))
                {
                    d=fabs(s[kk-1])+fabs(s[kk]);
                    dd=fabs(e[kk-1]);
                    if (dd>eps*d) 
                    {
                        kk=kk-1;
                    }
                    else e[kk-1]=0.0;
                }
                if (kk==mm-1)
                { 
                    kk=kk+1;
                    if (s[kk-1]<0.0)
                    { 
                        s[kk-1]=-s[kk-1];
                        for (i=1; i<=n; i++)
                        { 
                            ix=(i-1)*blockres->lBlockLine+kk-1;
                            *(datav+ix)=-(*(datav+ix));
                        }
                    }
                    while ((kk!=m1)&&(s[kk-1]<s[kk]))
                    { 
                        d=s[kk-1];
                        s[kk-1]=s[kk];
                        s[kk]=d;
                        if (kk<n)
                        {
                            for (i=1; i<=n; i++)
                            {
                                ix=(i-1)*blockres->lBlockLine+kk-1;
                                iy=(i-1)*blockres->lBlockLine+kk;
                                d=*(datav+ix);
                                *(datav+ix)=*(datav+iy);
                                *(datav+iy)=d;
                            }
                        }
                        if (kk<m)
                        {
                            for (i=1; i<=m; i++)
                            {
                                ix=(i-1)*blockres->lBlockLine+kk-1;
                                iy=(i-1)*blockres->lBlockLine+kk;
                                d=*(datau+ix);
                                *(datau+ix)=*(datau+iy);
                                *(datau+iy)=d;
                            }
                        }
                        kk=kk+1;
                    }
                    it=60;
                    mm=mm-1;
                }
                else
                { 
                    ks=mm;
                    while ((ks>kk)&&(fabs(s[ks-1])!=0.0))
                    { 
                        d=0.0;
                        if (ks!=mm) 
                        {
                            d=d+fabs(e[ks-1]);
                        }
                        if (ks!=kk+1) 
                        {
                            d=d+fabs(e[ks-2]);
                        }
                        dd=fabs(s[ks-1]);
                        if (dd>eps*d) 
                        {
                            ks=ks-1;
                        }
                        else
                        {
                            s[ks-1]=0.0;
                        }
                    }
                    if (ks==kk)
                    { 
                        kk=kk+1;
                        d=fabs(s[mm-1]);
                        t=fabs(s[mm-2]);
                        if (t>d) 
                        {
                            d=t;
                        }
                        t=fabs(e[mm-2]);
                        if (t>d) 
                        {
                            d=t;
                        }
                        t=fabs(s[kk-1]);
                        if (t>d)
                        {
                            d=t;
                        }
                        t=fabs(e[kk-1]);
                        if (t>d)
                        {
                            d=t;
                        }
                        sm=s[mm-1]/d; 
                        sm1=s[mm-2]/d;
                        em1=e[mm-2]/d;
                        sk=s[kk-1]/d;
                        ek=e[kk-1]/d;
                        b=((sm1+sm)*(sm1-sm)+em1*em1)/2.0;
                        c=sm*em1;
                        c=c*c; 
                        shh=0.0;
                        if ((b!=0.0)||(c!=0.0))
                        { 
                            shh=sqrt(b*b+c);
                            if (b<0.0)
                            {
                                shh=-shh;
                            }
                            shh=c/(b+shh);
                        }
                        fg[0]=(sk+sm)*(sk-sm)-shh;
                        fg[1]=sk*ek;
                        for (i=kk; i<=mm-1; i++)
                        { 
                            sss(fg,cs);
                            if (i!=kk) 
                            {
                                e[i-2]=fg[0];
                            }
                            fg[0]=cs[0]*s[i-1]+cs[1]*e[i-1];
                            e[i-1]=cs[0]*e[i-1]-cs[1]*s[i-1];
                            fg[1]=cs[1]*s[i];
                            s[i]=cs[0]*s[i];
                            if ((cs[0]!=1.0)||(cs[1]!=0.0))
                            {
                                for (j=1; j<=n; j++)
                                {
                                    ix=(j-1)*blockres->lBlockLine+i-1;
                                    iy=(j-1)*blockres->lBlockLine+i;
                                    d=cs[0]*(*(datav+ix))+cs[1]*(*(datav+iy));
                                    *(datav+iy)=-cs[1]*(*(datav+ix))+cs[0]*(*(datav+iy));
                                    *(datav+ix)=d;
                                }
                            }
                            sss(fg,cs);
                            s[i-1]=fg[0];
                            fg[0]=cs[0]*e[i-1]+cs[1]*s[i];
                            s[i]=-cs[1]*e[i-1]+cs[0]*s[i];
                            fg[1]=cs[1]*e[i];
                            e[i]=cs[0]*e[i];
                            if (i<m)
                            {
                                if ((cs[0]!=1.0)||(cs[1]!=0.0))
                                {
                                    for (j=1; j<=m; j++)
                                    { 
                                        ix=(j-1)*blockres->lBlockLine+i-1;
                                        iy=(j-1)*blockres->lBlockLine+i;
                                        d=cs[0]*(*(datau+ix))+cs[1]*(*(datau+iy));
                                        *(datau+iy)=-cs[1]*(*(datau+ix))+cs[0]*(*(datau+iy));
                                        *(datau+ix)=d;
                                    }
                                }
                            }
                        }
                        e[mm-2]=fg[0];
                        it=it-1;
                    }
                    else
                    { 
                        if (ks==mm)
                        {
                            kk=kk+1;
                            fg[1]=e[mm-2]; 
                            e[mm-2]=0.0;
                            for (ll=kk; ll<=mm-1; ll++)
                            { 
                                i=mm+kk-ll-1;
                                fg[0]=s[i-1];
                                sss(fg,cs);
                                s[i-1]=fg[0];
                                if (i!=kk)
                                { 
                                    fg[1]=-cs[1]*e[i-2];
                                    e[i-2]=cs[0]*e[i-2];
                                }
                                if ((cs[0]!=1.0)||(cs[1]!=0.0))
                                {
                                    for (j=1; j<=n; j++)
                                    {
                                        ix=(j-1)*blockres->lBlockLine+i-1;
                                        iy=(j-1)*blockres->lBlockLine+mm-1;
                                        d=cs[0]*(*(datav+ix))+cs[1]*(*(datav+iy));
                                        *(datav+iy)=-cs[1]*(*(datav+ix))+cs[0]*(*(datav+iy));
                                        *(datav+ix)=d;
                                    }
                                }
                            }
                        }
                        else
                        {
                            kk=ks+1;
                            fg[1]=e[kk-2];
                            e[kk-2]=0.0;
                            for (i=kk; i<=mm; i++)
                            {
                                fg[0]=s[i-1];
                                sss(fg,cs);
                                s[i-1]=fg[0];
                                fg[1]=-cs[1]*e[i-1];
                                e[i-1]=cs[0]*e[i-1];
                                if ((cs[0]!=1.0)||(cs[1]!=0.0))
                                {
                                    for (j=1; j<=m; j++)
                                    {
                                        ix=(j-1)*blockres->lBlockLine+i-1;
                                        iy=(j-1)*blockres->lBlockLine+kk-2;
                                        d=cs[0]*(*(datau+ix))+cs[1]*(*(datau+iy));
                                        *(datau+iy)=-cs[1]*(*(datau+ix))+cs[0]*(*(datau+iy));
                                        *(datau+ix)=d;
                                    }
                                }
                            }
                        }
                    }
                }
            }
    
    
    EXT:
    return res;
}






static void ppp(PBLOCK blockres,MDouble*e,MDouble* s,MLong top,MLong height,MLong width,MLong m,MLong n)
{ 
    MLong i,j,p,q,n1;
    MDouble d;
    MDouble*datares;
    MDouble *datav;
    datares=(MDouble*)blockres->pBlockData;
    datav=(MDouble*)blockres->pBlockData+top*blockres->lBlockLine;
    n1=blockres->lBlockLine;

    if (m>=n) 
    {
        i=n;
    }
    else
    {
        i=m;
    }
    for (j=1; j<=i-1; j++)
    {
        *(datares+(j-1)*n1+j-1)=s[j-1];
        *(datares+(j-1)*n1+j)=e[j-1];
    }
    *(datares+(i-1)*n1+i-1)=s[i-1];
    if (m<n) 
    {
        *(datares+(i-1)*n1+i)=e[i-1];
    }
    for (i=1; i<=n-1; i++)
    for (j=i+1; j<=n; j++)
    { 
        p=(i-1)*blockres->lBlockLine+j-1;
        q=(j-1)*blockres->lBlockLine+i-1;
        d=*(datav+p);
        *(datares+p)=*(datav+q);
        *(datares+q)=d;
    }
return;
}

static void sss(MDouble fg[2],MDouble cs[2])

{ 
    double r,d;
    if ((fabs(fg[0])+fabs(fg[1]))==0.0)
    { 
        cs[0]=1.0;
        cs[1]=0.0;
        d=0.0;}
else 
{
    d=sqrt(fg[0]*fg[0]+fg[1]*fg[1]);
    if (fabs(fg[0])>fabs(fg[1]))
    { 
        d=fabs(d);
        if (fg[0]<0.0) 
        {
            d=-d;
        }
}
if (fabs(fg[1])>=fabs(fg[0]))
{
    d=fabs(d);
    if (fg[1]<0.0) 
    {
            d=-d;
    }
}
cs[0]=fg[0]/d; 
cs[1]=fg[1]/d;
}
r=1.0;
if (fabs(fg[0])>fabs(fg[1])) r=cs[1];
else
{
        if (cs[0]!=0.0) 
        {
            r=1.0/cs[0];
        }
        
}
fg[0]=d;
fg[1]=r;
return;
}

/// \brief  blobSort  sort the data. in ascend 
/// the datatype is DOUBLE
/// \param s the source data
/// \param n the length of the source data
/// \return  error code

MRESULT blobSort(MFloat *s, MLong n)
{
    MRESULT res=LI_ERR_NONE;
    MLong i;
    MLong j;
    MFloat value;
    for (i=0;i<n;i++)
    {
        for (j=i+1;j<n;j++)
        {
            if (s[i]>s[j])
            {
                value=s[i];
                s[i]=s[j];
                s[j]=value;
            }
        }
    }
    return res;
}
/// \brief  blobSort_U8  sort the data. in ascend 
/// the datatype is UINT8
/// \param s the source data
/// \param n the length of the source data
/// \return  error code
MRESULT blobSort_U8(MUInt8* s,MLong n)
{
    MRESULT res=LI_ERR_NONE;
    MLong i;
    MLong j;
    MUInt8 value;
    for (i=0;i<n;i++)
    {
        for (j=i+1;j<n;j++)
        {
            if (s[i]>s[j])
            {
                value=s[i];
                s[i]=s[j];
                s[j]=value;
            }
        }
    }
    return res;
}

 MRESULT QuickSort_U8(MUInt8 *p,MLong n)
 {
     MLong res=LI_ERR_NONE;
     MLong  m,i0,*i,s0,*s;
     
     i=&i0;
     if (n>10)
     { 
         isplit(p,n,i);
         m=i0;
         QuickSort_U8(p,m);
         s=(MLong*)(p+(i0+1));
         m=n-(i0+1);
         QuickSort_U8(s,m);
     }
     else blobSort_U8(p,n);
 

     return res;

 }

 static void isplit(MUInt8*p,MLong n,MLong *m)
 {
     MLong  i,j,k,l,t;
 i=0; j=n-1;
 k=(i+j)/2;
 if ((p[i]>=p[j])&&(p[j]>=p[k])) 
 {
     l=j;
 }
 else if((p[i]>=p[k])&&(p[k]>=p[j])) 
     {
         l=k;
 }
 else l=i;
 t=p[l]; p[l]=p[i];
 while (i!=j)
 {
     while ((i<j)&&(p[j]>=t)) j=j-1;
 if (i<j)
 { 
     p[i]=p[j]; i=i+1;
     while ((i<j)&&(p[i]<=t)) 
         {
             i=i+1;
     }
 if (i<j)
 {
     p[j]=p[i]; j=j-1;
 }
 }
 }
 p[i]=t; *m=i;
 return;
 }