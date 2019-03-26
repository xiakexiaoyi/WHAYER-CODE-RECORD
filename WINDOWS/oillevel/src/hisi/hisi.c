#include"hisi.h"
#include"gradient.h"
#include <string.h>

MVoid HISI_createGradAngle( const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag, MUInt16 *pAngle)
{
	//pImgSrc 为源地址，lImgLine 为stride，lWidth为宽，lHeight 为高，
	//pMag输出幅值指针，pAngle为输出角度指针
	//pImgX水平方向输出数据指针
	//pImgY竖直方向输出数据指针
	/*模板系数为[-1,0,1
				   -2,0,2
				   -1,0,1]
   //采用HI_S32 HI_MPI_IVE_CANNY看得到的结果效果如何，如果不好就采用HI_MPI_IVE_SOBEL先获得水平与竖直方向的数据，再通过计算获得梯度角度以及幅值 
   //函数主要是获得角度以及幅值。
	*/
	CreateGradAngleImg(pImgSrc,lImgLine,lWidth,lHeight,pImgX,pImgY,pMag,pAngle);
}

MVoid HISI_copy(MUInt8 * ap_pix_old, MUInt8 * ap_pix_new ,MLong num)
{
	//ap_pix_old为源地址
	//ap_pix_new为目的地址
	//num为字节数为32的倍数，可以在MatchRigidBody.c中的649行修改，只要保证num值大于lheight*lWidth就行
	//采用HI_MPI_IVE_DMA 
	memcpy(ap_pix_new,ap_pix_old,num);
}
