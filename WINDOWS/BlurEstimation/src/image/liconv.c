/*!
* \file Liconv.c
* \brief  the function related to convolution 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/
#include "limem.h"
#include "limath.h"
#include "lidebug.h"
#include "liblock.h"
#include "liconv.h"




/// \brief  Liconv is used to calculate the convolution of 
/// two blocks
/// \param  conv_res the result
/// \param  BlockA the block to be convoluted
/// \param  BlockB the block to be convoluted
/// \return  error code
/// \对Uint8和float格式的block进行卷积运算
MRESULT LiConv(PBLOCK conv_res,PBLOCKEXT BlockA,PBLOCK BlockB)
{
	switch(BlockA->block.typeDataA)
	{
	case DATA_U8:
		return LiConv_U8(conv_res,BlockA,BlockB);
	case DATA_F32:
		return LiConv_FLOAT(conv_res,BlockA,BlockB);
	default:
		JASSERT(MFalse);
		return -1;
	}
}

/// \brief  Liconv_U8 is used to calculate the convolution of 
/// two blocks,the blockA is UINT8 and blockB is FLOAT
/// \param  conv_res the result
/// \param  BlockA the block to be convoluted,DATATYPE IS UINT 8
/// \param  BlockB the block to be convoluted,DATATYPE IS FLOAT
/// \return  error code
/// \将图像blockA和float型的block进行卷积
MRESULT LiConv_U8(PBLOCK conv_res,PBLOCKEXT BlockA,PBLOCK BlockB)
{ 
	MRESULT res=LI_ERR_NONE;
	MLong i=0;
	MLong j=0;
	MLong n=0;
	MLong m=0;
	MLong N1=BlockA->ext.bottom-BlockA->ext.top;
	MLong N2=BlockB->lHeight;
	MLong BlA=BlockA->block.lBlockLine;
	MLong M1=BlockA->ext.right-BlockA->ext.left;
	MLong M2=BlockB->lWidth;
	MLong BlB=BlockB->lBlockLine;
	MLong N=conv_res->lHeight;
	MLong M=conv_res->lWidth;
	MLong BlConv=conv_res->lBlockLine;
	MUInt8*pDataA=MNull;
	MFloat *pDataB=MNull;
	MFloat *pdataConv=MNull;
	MLong ext;
	pDataA=(MUInt8*)(BlockA->block.pBlockData)+BlockA->ext.left+(BlockA->ext.top)*(BlockA->block.lBlockLine);	
	pDataB=(MFloat*)(BlockB->pBlockData);	
	pdataConv=(MFloat*)(conv_res->pBlockData);
	ext=conv_res->lBlockLine-conv_res->lWidth;
    /// \calculate the two dimension convolution，二维卷积
	for(i=0;i<N;i++,pdataConv+=ext)
	for(j=0;j<M;j++,pdataConv++)
	{
		MFloat temp=0;
		for(n=0;n<N2;n++)
		for(m=0;m<M2;m++)
			
			if ((j-m)>=0&&(j-m)<M1&&(i-n)>=0&&(i-n)<N1)

				{
					
					temp+=pDataB[(N2-1-n)*BlB+M2-1-m]*pDataA[(i-n)*BlA+j-m];
				
					
				}
		*(pdataConv)=temp; 				
		
	}
	
	return res;
}

/// \brief  Liconv_FLOAT is used to calculate the convolution of 
/// two blocks,the blockA isFLOAT and blockB is FLOAT
/// \param  conv_res the result
/// \param  BlockA the block to be convoluted,DATATYPE IS FLOAT
/// \param  BlockB the block to be convoluted,DATATYPE IS FLOAT
/// \return  error code
MRESULT LiConv_FLOAT(PBLOCK conv_res,PBLOCKEXT BlockA,PBLOCK BlockB)
{ 
	MRESULT res=LI_ERR_NONE;
	MLong i=0;
	MLong j=0;
	MLong n=0;
	MLong m=0;
	MLong N1=BlockA->ext.bottom-BlockA->ext.top;
	MLong N2=BlockB->lHeight;
	MLong BlA=BlockA->block.lBlockLine;
	MLong M1=BlockA->ext.right-BlockA->ext.left;
	MLong M2=BlockB->lWidth;
	MLong BlB=BlockB->lBlockLine;
	MLong N=conv_res->lHeight;
	MLong M=conv_res->lWidth;
	MLong BlConv=conv_res->lBlockLine;
	MFloat*pDataA=MNull;
	MFloat *pDataB=MNull;
	MFloat *pdataConv=MNull;
	MLong ext;

	pDataA=(MFloat*)(BlockA->block.pBlockData)+BlockA->ext.left+(BlockA->ext.top)*(BlockA->block.lBlockLine);	
	pDataB=(MFloat*)(BlockB->pBlockData);	
	pdataConv=(MFloat*)(conv_res->pBlockData);
	ext=conv_res->lBlockLine-conv_res->lWidth;
    /// \two dimension convolution
	for(i=0;i<N;i++,pdataConv+=ext)
		for(j=0;j<M;j++,pdataConv++)
		{
			MFloat temp=0;
			for(n=0;n<N2;n++)
				for(m=0;m<M2;m++)

					if ((j-m)>=0&&(j-m)<M1&&(i-n)>=0&&(i-n)<N1)

					{

						temp+=pDataB[(N2-1-n)*BlB+M2-1-m]*pDataA[(i-n)*BlA+j-m];


					}
					*(pdataConv)=temp; 				

		}

		return res;
}