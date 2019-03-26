/*!
* \file Liimfilter.c
* \brief  the function related to filter the image 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/

#include "limath.h"
#include "lidebug.h"
#include "liblock.h"
#include "liconv.h"
#include"liimfilter.h"




/// \brief  ImFilter  ImFilter the block using the  
/// \param  hMemMgr  Handle
/// \param  BlockRes the block after filtering
/// \param  BlockSrc the block needed to be filtered 
/// \param  BlockFilter the block filter
/// \return  error code

MRESULT ImFilter(MHandle hMemMgr,PBLOCK BlockRes,PBLOCKEXT BlockSrc,PBLOCK BlockFilter)
{
	MLong res=LI_ERR_NONE;
	BLOCK conv_res={0};
	MLong width,height,dheight,dwidth,i,j;
	MLong Ext,Extres;
	MFloat *pdata,*datares;
    /// \calculated the width and height after  filtering
	width=(BlockSrc->ext.right-BlockSrc->ext.left)+BlockFilter->lWidth-1;
	height=(BlockSrc->ext.bottom-BlockSrc->ext.top)+BlockFilter->lHeight-1;
	dheight=floor((long double)(height-BlockRes->lHeight)/2);
	dwidth=floor((long double)(width-BlockRes->lWidth)/2);
    /// \calculate the two dimension convolution
	GO (B_Create(hMemMgr,&conv_res,DATA_F32,width,height));
	LiConv(&conv_res,BlockSrc,BlockFilter);
	pdata=(MFloat*)conv_res.pBlockData+dheight*conv_res.lBlockLine+dwidth;
	datares=(MFloat*)BlockRes->pBlockData;
	Ext=conv_res.lBlockLine-conv_res.lWidth+dwidth;
	Extres=BlockRes->lBlockLine-BlockRes->lWidth;
    /// \get the same size of the source block
	for(i=0;i<BlockRes->lHeight;i++,pdata+=Ext,datares+=Extres)
	for(j=0;j<BlockRes->lWidth;j++,pdata++,datares++)
	{
		*(datares)=*(pdata);
	}
	

	
EXT:
	B_Release(hMemMgr,&conv_res);
	return res;
}


