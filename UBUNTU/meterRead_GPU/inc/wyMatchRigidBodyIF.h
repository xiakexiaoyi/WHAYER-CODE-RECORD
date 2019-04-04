#include "wyMatchRigidBodyStruct.h"
#include "wyVACommon.h"

#ifndef _WYMATCHRIGIDBODYIF_
#define _WYMATCHRIGIDBODYIF_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

 int saveDesInFile(wy_objCharacterDes *objDesArray[], int arrayNum, char *fileName);
 int readDesFrmFile(wy_objCharacterDes *objDesArray[],int *arrayNumR,char *fileName);
 int matchRigBodyInit_wy(wy_objCharacterDes *objDesArray[], int arrayNum);
 int matchRigBodyExit_wy(wy_objCharacterDes *objDesArray[], int arrayNum);
 int matchRigBodyTrainInit_wy(wy_trainPMemUg **ddrMemPtr);
 int matchRigBodyTrainExit_wy(wy_trainPMemUg *ddrMem);
 int matchRigBodyTrain_wy(wy_maskByteInfo *maskByteInfo, wy_picInfo *pic,     
                          wy_objCharacterDes *objDesArray[], wy_trainPMemUg *ddrMem, int rotate);
 int matchRidBodyProcessInit_wy(wy_matchPMemUg **ddRmemUgPtr);
 int matchRidBodyProcessExit_wy(wy_matchPMemUg *ddRmemUg);
 int matchRigBodyProcess_wy(int *lx, int *ly, int *angle, wy_picInfo *pic, wy_objCharacterDes *objDesArray[], 
                                    wy_matchPMemUg *memUg, WYVA_FLOAT matchVal, int rotate, WYVA_FLOAT *resMatchRate);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif











































