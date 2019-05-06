#include"hisi.h"
#include"gradient.h"
#include <string.h>

MVoid HISI_createGradAngle( const MUInt8 *pImgSrc, MInt32 lImgLine, MInt32 lWidth, MInt32 lHeight,MInt16 *pImgX, MInt16 *pImgY, MUInt16 *pMag, MUInt16 *pAngle)
{
	//pImgSrc ΪԴ��ַ��lImgLine Ϊstride��lWidthΪ��lHeight Ϊ�ߣ�
	//pMag�����ֵָ�룬pAngleΪ����Ƕ�ָ��
	//pImgXˮƽ�����������ָ��
	//pImgY��ֱ�����������ָ��
	/*ģ��ϵ��Ϊ[-1,0,1
				   -2,0,2
				   -1,0,1]
   //����HI_S32 HI_MPI_IVE_CANNY���õ��Ľ��Ч����Σ�������þͲ���HI_MPI_IVE_SOBEL�Ȼ��ˮƽ����ֱ��������ݣ���ͨ���������ݶȽǶ��Լ���ֵ 
   //������Ҫ�ǻ�ýǶ��Լ���ֵ��
	*/
	CreateGradAngleImg(pImgSrc,lImgLine,lWidth,lHeight,pImgX,pImgY,pMag,pAngle);
}

MVoid HISI_copy(MUInt8 * ap_pix_old, MUInt8 * ap_pix_new ,MLong num)
{
	//ap_pix_oldΪԴ��ַ
	//ap_pix_newΪĿ�ĵ�ַ
	//numΪ�ֽ���Ϊ32�ı�����������MatchRigidBody.c�е�649���޸ģ�ֻҪ��֤numֵ����lheight*lWidth����
	//����HI_MPI_IVE_DMA 
	memcpy(ap_pix_new,ap_pix_old,num);
}
