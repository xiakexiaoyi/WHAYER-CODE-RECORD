#include "wyVACommon.h"
#include "wyMatchRigidBodyStruct.h"

#ifndef _WYMATCHRIGIDBODY_
#define _WYMATCHRIGIDBODY_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif
//WYVA_OBJSIZE_MAX

int matchRigBodyTrainStep0(wy_picInfo *pic, wy_trainPMemUg *ddrMem, int *angle);

void hi3516RotateX_wy(wy_picInfo *outPic, wy_picInfo *inPic, int iAngle);
void hi3516BiLinearScalor(wy_picInfo *outPic, wy_picInfo *inPic);
void hi3516RefineDes(int *obPixelNum, wy_picInfo *maskPic, unsigned char *pDesData,int desW, int desH, unsigned char *pMaskDataL2);

int matchRigBodyTrainGetDes_wy(int angle, wy_trainPMemUg *ddrMem, wy_objCharacterDes *objDesArray, int rotateAngle, WYVA_FLOAT magRate);

 
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif





