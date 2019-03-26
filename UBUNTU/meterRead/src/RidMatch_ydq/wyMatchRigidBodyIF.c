#include <stdio.h>
#include <stdlib.h>
#include "wyVAFunc.h"
#include "wyVACommon.h"
#include "wyMatchRigidBody.h"
#include "wyMatchRigidBodyStruct.h"
#include "wyMatchRigidBodyIF.h"
#include <string.h>
//#include <unistd.h>
//#include "wy_hi3516.h"
#include "sdkMacro.h"

/*******************************file format*************************************************
* head:    (Widht)XX XX XX XX   (Height)XX XX XX XX    (pixel number)XX XX XX XX    (angle)XX XX XX XX        
* data (Widht*Height):XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*
*****************************************************************************************/
int saveDesInFile(wy_objCharacterDes *objDesArray[], int arrayNum, char *fileName)
{
    int i;
    FILE *fOutprt=NULL;
    unsigned char head[16], arrayNumU8;
    if(NULL==fileName)
    {
        HOST_PRINT("saveDesInFile fileName is NULL!\n");
        return WY_IN_PARA_ERR;
    }

    fOutprt = fopen(fileName, "wb");
    if(NULL == fOutprt)
    {
        HOST_PRINT("saveDesInFile fopen error!\n");
        return WY_INNER_ERR;
    }

    arrayNumU8 = arrayNum&0xff;
    fwrite(&arrayNumU8, sizeof(unsigned char), 1, fOutprt);
    for(i=0;i<arrayNum;i++)
    {
        head[0] = (objDesArray[i]->obRecWidthS&0xff000000)>>24;
        head[1] = (objDesArray[i]->obRecWidthS&0x00ff0000)>>16;
        head[2] = (objDesArray[i]->obRecWidthS&0x0000ff00)>>8;
        head[3] = objDesArray[i]->obRecWidthS&0x000000ff;

        head[4] = (objDesArray[i]->obRecHeightS&0xff000000)>>24;
        head[5] = (objDesArray[i]->obRecHeightS&0x00ff0000)>>16;
        head[6] = (objDesArray[i]->obRecHeightS&0x0000ff00)>>8;
        head[7] = objDesArray[i]->obRecHeightS&0x000000ff;

        head[8] =   (objDesArray[i]->obPixelNum&0xff000000)>>24;
        head[9] =   (objDesArray[i]->obPixelNum&0x00ff0000)>>16;
        head[10] = (objDesArray[i]->obPixelNum&0x0000ff00)>>8;
        head[11] = objDesArray[i]->obPixelNum&0x000000ff;

        head[12] = (objDesArray[i]->angle&0xff000000)>>24;
        head[13] = (objDesArray[i]->angle&0x00ff0000)>>16;
        head[14] = (objDesArray[i]->angle&0x0000ff00)>>8;
        head[15] = objDesArray[i]->angle&0x000000ff;

        fwrite(head, sizeof(unsigned char), 16, fOutprt);
        fwrite(objDesArray[i]->pData, sizeof(unsigned char), objDesArray[i]->obRecHeightS*objDesArray[i]->obRecWidthS, fOutprt);
    }

    if(fOutprt)
        fclose(fOutprt);

    return  WY_SUCCESS;
}

int readDesFrmFile(wy_objCharacterDes *objDesArray[],int *arrayNumR, char *fileName)
{
    int i;
    FILE *fInprt=NULL;
    unsigned char head[16], arrayNumU8;
    int arrayNum;

    if(NULL==fileName)
    {
        HOST_PRINT("readDesFrmFile fileName is NULL!\n");
        return WY_IN_PARA_ERR;
    }

    fInprt = fopen(fileName, "rb");
    if(NULL == fInprt)
    {
        HOST_PRINT("readDesFrmFile fopen error!\n");
        return WY_INNER_ERR;
    }
   
    fread(&arrayNumU8, sizeof(unsigned char), 1, fInprt);
    arrayNum = (int)arrayNumU8;
    if(arrayNum>36)
    {
        fclose(fInprt);
        HOST_PRINT("readDesFrmFile arrayNum bigger 36!\n");
        return WY_IN_PARA_ERR;
    }

    for(i=0;i<arrayNum;i++)
    {
        fread(head, sizeof(unsigned char), 16, fInprt);
        objDesArray[i]->obRecWidthS = (head[0]<<24)|(head[1]<<16)|(head[2]<<8)|head[3]; 
        objDesArray[i]->obRecHeightS = (head[4]<<24)|(head[5]<<16)|(head[6]<<8)|head[7]; 
        objDesArray[i]->obPixelNum = (head[8]<<24)|(head[9]<<16)|(head[10]<<8)|head[11]; 
        objDesArray[i]->angle =  (head[12]<<24)|(head[13]<<16)|(head[14]<<8)|head[15];         
        fread(objDesArray[i]->pData, sizeof(unsigned char), objDesArray[i]->obRecHeightS*objDesArray[i]->obRecWidthS, fInprt);
    }
   
    if(fInprt)
        fclose(fInprt);

    *arrayNumR = arrayNum;
    return  WY_SUCCESS;
}


int hi3516MMZMalloc_wy(unsigned int *u32PhyAddr, void **u32VirtAddr, unsigned int size)
{
  
#if(0)
    if(!size)
    {
        HOST_PRINT("[%s - %s - %d] size is NULL!\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return WY_INNER_ERR;
    }

    if(HI_SUCCESS!=HI_MPI_SYS_MmzAlloc_Cached(u32PhyAddr, u32VirtAddr, "User",HI_NULL,size))
    {
        HOST_PRINT("[%s - %s - %d]  error!\n", 
            __FILE__, __FUNCTION__, __LINE__);

        return WY_INNER_ERR;
    }
#else
    *u32VirtAddr = malloc(size);
#endif
    return  WY_SUCCESS;
}

int hi3516MMZFree_wy(unsigned int u32PhyAddr, void *u32VirtAddr, unsigned int size)
{


#if(0)
    if((!u32PhyAddr)||(!u32VirtAddr))
    {
        HOST_PRINT("[%s - %s - %d]  inPara is NULL!\n", 
            __FILE__, __FUNCTION__, __LINE__);
        return WY_INNER_ERR;
    }

    if(HI_SUCCESS!=HI_MPI_SYS_MmzFree(u32PhyAddr, u32VirtAddr))
    {
        HOST_PRINT("[%s - %s - %d]  error!\n", 
            __FILE__, __FUNCTION__, __LINE__);

        return WY_INNER_ERR;
    }
#else
    free(u32VirtAddr);
#endif

    return  WY_SUCCESS;
}

int matchRigBodyInit_wy(wy_objCharacterDes *objDesArray[], int arrayNum)
{
    int i, malloc_err=0;
    wy_objCharacterDes *tmp;
 
    if(!objDesArray)
    {
        HOST_PRINT("matchRigBodyInit_wy objDesArray is NULL!\n");//integer
        return WY_IN_PARA_ERR;
    }

    tmp = (wy_objCharacterDes *)malloc(arrayNum*sizeof(wy_objCharacterDes));
    if(!tmp)
    {
        HOST_PRINT("matchRigBodyInit_wy DspSdk_memAlloc error!\n");//integer
        return WY_INNER_ERR;
    }

        
    for(i=0;i<arrayNum;i++)
    {
        objDesArray[i] = tmp + i;
        objDesArray[i]->pData = (unsigned char *)malloc(WY_OBJDESSIZE_MAX*WY_OBJDESSIZE_MAX*sizeof(unsigned char));
        if(!objDesArray[i]->pData)
            malloc_err = 1;
    }

    if(malloc_err)
    { 
       HOST_PRINT("matchRigBodyInit_wy malloc_err!\n");//integer
        goto EXIT0;
    }	
 
    return WY_SUCCESS;	
EXIT0:
    matchRigBodyExit_wy(objDesArray, arrayNum);
    return WY_FAILURE;	
}

int matchRigBodyExit_wy(wy_objCharacterDes *objDesArray[], int arrayNum)
{
    int i;
    wy_objCharacterDes *objDesArrayZero;

    if(!objDesArray)
    {
        HOST_PRINT("matchRigBodyExit_wy objDesArray is NULL!\n");//integer
        return WY_IN_PARA_ERR;
    }

    for(i=0;i<arrayNum;i++)
    {
        if(objDesArray[i]->pData)
            free(objDesArray[i]->pData);
    }
    objDesArrayZero = objDesArray[0];

    for(i=1;i<arrayNum;i++)
    {
         if(objDesArrayZero>objDesArray[i])
                objDesArrayZero = objDesArray[i];
    }

    if(objDesArrayZero)	   
       free(objDesArrayZero);

    return WY_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
int matchRigBodyTrainInit_wy(wy_trainPMemUg **ddrMemPtr)
{
    wy_picInfo *objMaskPic, *sfObjPic;
   // wy_maskByteInfo *maskByteInfo;
    wy_picInfo *objPic;
    wy_picInfo *rotateObjPic; 
    wy_picInfo *rotateMaskPic;
    wy_picInfo *salorSfObjPic;
    wy_trainPMemUg *ddrMem;

    ddrMem = (wy_trainPMemUg *)malloc(sizeof(wy_trainPMemUg));
    if(!ddrMem)
    {
        HOST_PRINT("matchRigBodyTrainInit_wy DspSdk_memAlloc error!\n");//integer
        return WY_FAILURE;
    }
    memset((void *)ddrMem, 0, sizeof(wy_trainPMemUg));
    
    objMaskPic = &ddrMem->objMaskPic;
    sfObjPic = &ddrMem->sfObjPic;
   // maskByteInfo = &ddrMem->maskByteInfo;
    objPic = &ddrMem->objPic;
    rotateObjPic = &ddrMem->rotateObjPic; 
    rotateMaskPic = &ddrMem->rotateMaskPic;
    salorSfObjPic = &ddrMem->salorSfObjPic;

    ddrMem->objMaskPic.stride = WYVA_OBJSIZE_MAX;
    ddrMem->sfObjPic.stride = WYVA_OBJSIZE_MAX;
            
    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&(objMaskPic->u32PhyAddr), 
		     ((void *)(&objMaskPic->byteBufAddr)), WYVA_OBJSIZE_MAX*WYVA_OBJSIZE_MAX))
    {
        HOST_PRINT("matchRigBodyTrainInit_wy DspSdk_memAlloc error!\n");//integer
        goto EXIT0;
    }
        
   // sfObjPic->byteBufAddr = (unsigned char *)malloc(WYVA_OBJSIZE_MAX*WYVA_OBJSIZE_MAX);
    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&(sfObjPic->u32PhyAddr), 
		     ((void *)(&sfObjPic->byteBufAddr)), WYVA_OBJSIZE_MAX*WYVA_OBJSIZE_MAX))
    {
        HOST_PRINT("matchRigBodyTrainInit_wy DspSdk_memAlloc error!\n");//integer
        
        goto EXIT0;
    }
//---------------------------------------------------------------------------------------------
   // maskByteInfo->byteBufAddr = (unsigned char *)malloc(WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX);
  /* 
    if(!maskByteInfo->byteBufAddr)
    {
        HOST_PRINT("[%s - %s - %d] DspSdk_memAlloc error!\n", 
                                  __FILE__, __FUNCTION__, __LINE__);//integer
        goto EXIT0;
                                  
    }*/

    ddrMem->lx = (int *)malloc(WYVA_OBJSIZE_MAX*WYVA_OBJSIZE_MAX*sizeof(int));
    if(!ddrMem->lx)
    {
        HOST_PRINT("matchRigBodyTrainInit_wy DspSdk_memAlloc error!\n");//integer
        goto EXIT0; 
    }

    ddrMem->ly = (int *)malloc(WYVA_OBJSIZE_MAX*WYVA_OBJSIZE_MAX*sizeof(int));
    if(!ddrMem->ly)
    {
        HOST_PRINT("matchRigBodyTrainInit_wy DspSdk_memAlloc error!\n");//integer
        goto EXIT0;
    }

    objPic->stride = WYVA_OBJSIZE_MAX+8;//((tmp1+8+2)>>3)<<3;    
   // objPic->byteBufAddr = (unsigned char *)malloc(objPic->stride*(WYVA_OBJSIZE_MAX+2));
    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&objPic->u32PhyAddr, 
		((void *)(&objPic->byteBufAddr)), (WYVA_OBJSIZE_MAX+8)*(WYVA_OBJSIZE_MAX+2)))
    {
        HOST_PRINT("matchRigBodyTrainInit_wy DspSdk_memAlloc error!\n");//integer
        goto EXIT0;
    }

    rotateObjPic->stride = WYVA_ROTATE_SIZE_MAX;
  
    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&rotateObjPic->u32PhyAddr, 
		((void *)(&rotateObjPic->byteBufAddr)), WYVA_ROTATE_SIZE_MAX*WYVA_ROTATE_SIZE_MAX))
    {
        HOST_PRINT("matchRigBodyTrainInit_wy rotatePic.byteBufAddr malloc error!\n");//integer
        goto EXIT0;
    }

    rotateMaskPic->stride = WYVA_ROTATE_SIZE_MAX;
                                           
    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&rotateMaskPic->u32PhyAddr, 
		((void *)(&rotateMaskPic->byteBufAddr)), WYVA_ROTATE_SIZE_MAX*WYVA_ROTATE_SIZE_MAX))
    {
        HOST_PRINT("hi3516MMZMalloc_wy rotateMaskPic.byteBufAddr malloc error!\n");//integer
        goto EXIT0;
    }

    salorSfObjPic->stride = WYVA_SCALOR_SIZE_MAX;
                                           
    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&salorSfObjPic->u32PhyAddr, ((void *)(&salorSfObjPic->byteBufAddr)),
		                                                             WYVA_SCALOR_SIZE_MAX*WYVA_SCALOR_SIZE_MAX))
    {
        HOST_PRINT("hi3516MMZMalloc_wy rotateMaskPic.byteBufAddr malloc error!\n");//integer
        goto EXIT0;
    }
    
    ddrMem->magAngStride = WYVA_SCALOR_SIZE_MAX;
	if(WY_SUCCESS!=hi3516MMZMalloc_wy(&ddrMem->u32PhyAddrAge, ((void *)(&ddrMem->pAngle)), 
		                        WYVA_SCALOR_SIZE_MAX*WYVA_SCALOR_SIZE_MAX*sizeof(unsigned char)))
    {
        HOST_PRINT("hi3516MMZMalloc_wy pAngle malloc error!\n");//integer
        goto EXIT0;
    }

	if(WY_SUCCESS!=hi3516MMZMalloc_wy(&ddrMem->u32PhyAddrMag, ((void *)(&ddrMem->pMag)), 
		                        WYVA_SCALOR_SIZE_MAX*WYVA_SCALOR_SIZE_MAX*sizeof(unsigned short)))
    {
        HOST_PRINT("hi3516MMZMalloc_wy pMag malloc error!\n");//integer
        goto EXIT0;
    }

    ddrMem->pMask = (unsigned char *)malloc(16*1024*sizeof(unsigned char));

    if(!ddrMem->pMask)
    {
        HOST_PRINT("pMask malloc error!\n");//integer
        goto EXIT0;
    }
    
    *ddrMemPtr = ddrMem;
    return WY_SUCCESS;
    
EXIT0:	
    matchRigBodyTrainExit_wy(ddrMem);
    return WY_FAILURE;
}

int matchRigBodyTrainExit_wy(wy_trainPMemUg *ddrMem)
{
    wy_picInfo *objMaskPic, *sfObjPic;
   // wy_maskByteInfo *maskByteInfo;
    wy_picInfo *objPic;
    wy_picInfo *rotateObjPic; 
    wy_picInfo *rotateMaskPic;
    wy_picInfo *salorSfObjPic;
    int *lx; 
    int *ly;
    unsigned char *pAngle;
    unsigned short *pMag;
    unsigned char *pMask;
    
    if(!ddrMem)
        return WY_FAILURE;
    objMaskPic = &ddrMem->objMaskPic;
    sfObjPic = &ddrMem->sfObjPic;
  //  maskByteInfo = &ddrMem->maskByteInfo;
    objPic = &ddrMem->objPic;
    rotateObjPic = &ddrMem->rotateObjPic; 
    rotateMaskPic = &ddrMem->rotateMaskPic;
    salorSfObjPic = &ddrMem->salorSfObjPic;
    lx = ddrMem->lx;
    ly = ddrMem->ly;
    pAngle = ddrMem->pAngle;
    pMag = ddrMem->pMag;
    pMask = ddrMem->pMask;
    
    if(objMaskPic->byteBufAddr)
        if(WY_SUCCESS!=hi3516MMZFree_wy(objMaskPic->u32PhyAddr, (void *)(objMaskPic->byteBufAddr), WYVA_OBJSIZE_MAX*WYVA_OBJSIZE_MAX))
        {
            HOST_PRINT("hi3516MMZFree_wy objMaskPic free error!\n");//integer
            return WY_FAILURE;
        }
      // free((void *)(objMaskPic->byteBufAddr));

    if(sfObjPic->byteBufAddr)
        if(WY_SUCCESS!=hi3516MMZFree_wy(sfObjPic->u32PhyAddr, (void *)(sfObjPic->byteBufAddr), WYVA_OBJSIZE_MAX*WYVA_OBJSIZE_MAX))
        {
            HOST_PRINT("hi3516MMZFree_wy sfObjPic free error!\n");//integer
            return WY_FAILURE;
        }

    //maskByteInfo->byteBufAddr = (unsigned char *)DspSdk_memAlloc(MEM_NO_CACHE, WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX, 16);  
    //if(maskByteInfo->byteBufAddr)
        //free((void *)(maskByteInfo->byteBufAddr));

    if(lx)
        free((void *)lx);

    if(ly)
        free((void *)ly);
                                                                                             
    if(objPic->byteBufAddr)	
        if(WY_SUCCESS!=hi3516MMZFree_wy(objPic->u32PhyAddr, (void *)(objPic->byteBufAddr), (WYVA_OBJSIZE_MAX+8)*(WYVA_OBJSIZE_MAX+2)))
        {
            HOST_PRINT("hi3516MMZFree_wy objPic free error!\n");//integer
            return WY_FAILURE;
        }
                                           
    if(rotateObjPic->byteBufAddr)
        if(WY_SUCCESS!=hi3516MMZFree_wy(rotateObjPic->u32PhyAddr, (void *)(rotateObjPic->byteBufAddr), 
            WYVA_ROTATE_SIZE_MAX*WYVA_ROTATE_SIZE_MAX*sizeof(unsigned char)))
        {
            HOST_PRINT("hi3516MMZFree_wy rotateObjPic free error!\n");//integer
            return WY_FAILURE;
        }
                                       
    if(rotateMaskPic->byteBufAddr)
        if(WY_SUCCESS!=hi3516MMZFree_wy(rotateMaskPic->u32PhyAddr, (void *)(rotateMaskPic->byteBufAddr), 
                                           WYVA_ROTATE_SIZE_MAX*WYVA_ROTATE_SIZE_MAX*sizeof(unsigned char)))
        {
            HOST_PRINT("hi3516MMZFree_wy rotateMaskPic free error!\n");//integer
            return WY_FAILURE;
        }

                                               
    if(salorSfObjPic->byteBufAddr)
        if(WY_SUCCESS!=hi3516MMZFree_wy(salorSfObjPic->u32PhyAddr, (void *)(salorSfObjPic->byteBufAddr),  
            WYVA_SCALOR_SIZE_MAX*WYVA_SCALOR_SIZE_MAX*sizeof(unsigned char)))
        {
            HOST_PRINT("hi3516MMZFree_wy salorSfObjPic free error!\n");//integer
            return WY_FAILURE;
        }  

    if(pAngle)
        if(WY_SUCCESS!=hi3516MMZFree_wy(ddrMem->u32PhyAddrAge, (void *)(pAngle), 
            WYVA_SCALOR_SIZE_MAX*WYVA_SCALOR_SIZE_MAX*sizeof(unsigned char)))
        {
            HOST_PRINT("hi3516MMZFree_wy pAngle free error!\n");//integer
            return WY_FAILURE;
        }

    if(pMag)
        if(WY_SUCCESS!=hi3516MMZFree_wy(ddrMem->u32PhyAddrMag, (void *)(pMag),  
                       WYVA_SCALOR_SIZE_MAX*WYVA_SCALOR_SIZE_MAX*sizeof(unsigned short)))
        {
            HOST_PRINT("hi3516MMZFree_wy pMag free error!\n");//integer
            return WY_FAILURE;
        }        

    if(pMask)
        free((void *)pMask);
        
    free((void *)ddrMem);

    return WY_SUCCESS;
}

int matchRigBodyTrain_wy(wy_maskByteInfo *maskByteInfo, wy_picInfo *pic, 
                          wy_objCharacterDes *objDesArray[], wy_trainPMemUg *ddrMem, int rotate)
{

    int ret = WY_SUCCESS;
    int angle = 0, i, rotateNum;

	//xsd
	int rotateAngle[5] = {0, 10, 350, 20, 340};		// 0, 10, -10, 20, -20
                
    if(!objDesArray)
    {
        HOST_PRINT("matchRigBodyTrain_wy objDesArray is NULL!\n");
        return WY_IN_PARA_ERR;
    }
	
	ddrMem->maskByteInfo.byteBufAddr = maskByteInfo->byteBufAddr;
	ddrMem->maskByteInfo.width = maskByteInfo->width;
	ddrMem->maskByteInfo.height = maskByteInfo->height;
	ddrMem->maskByteInfo.startX = ddrMem->maskByteInfo.startY = 0;
	
 /*   
    if((unsigned int)maskBitInfo->bitBufAddr&WYVA_ALIGN16)
    {
        HOST_PRINT("[%s - %s - %d] bitBufAddr should align to 16:%x!\n",
                      __FILE__, __FUNCTION__, __LINE__, (unsigned int )(maskBitInfo->bitBufAddr));
        return WY_IN_PARA_ERR;
    }
     */
     /*
    if((maskBitInfo->width&0xf)||(maskBitInfo->height&0xf))
    {
        HOST_PRINT("[%s - %s - %d] width and height should be integer times of 16!\n", 
                                  __FILE__, __FUNCTION__, __LINE__);//integer
        return WY_IN_PARA_ERR; 
    }*/
        
    if((pic->width>WYVA_PIC_WIDTH_MAX)||(pic->height>WYVA_PIC_HEIGHT_MAX))
    {
        HOST_PRINT("matchRigBodyTrain_wy size should not be great than 720*576!\n");//integer
        return WY_IN_PARA_ERR; 
    }
      
    if(matchRigBodyTrainStep0(pic, ddrMem, &angle)<WY_SUCCESS)
    {
        HOST_PRINT("matchRigBodyTrainStep0 matchRigBodyTrainStep0 error!\n");//integer
        ret = WY_FAILURE;								  
        goto EXIT0;
    }


    //------------------------------------    
    // objDesArray[0]->angle = angle;
    //------------------------------------
    //i=0
    //i=1
    //---------------------------------------
    // (angle-10)<-180      angle+180 
    // (angle-10)<-90        angle+180
    //---------------------------------------

//    rotateNum = rotate ? 36:1;
	rotateNum = rotate ? 3:1;

    for(i=0;i<rotateNum;i++)
    {   
        //qdma
//        matchRigBodyTrainGetDes_wy(angle, ddrMem,  objDesArray[i], i*10, 0.9);
		matchRigBodyTrainGetDes_wy(angle, ddrMem,  objDesArray[i], rotateAngle[i], 0.9);
		//usleep(100);	  
    }
    

    //-------------------------------get discripe here------------------------
EXIT0:

    return ret;  
}

int matchRidBodyProcessInit_wy(wy_matchPMemUg **ddRmemUgPtr)
{
    //unsigned short *pAngle;
//	unsigned short *pMag;
    wy_picInfo *sfPic;
    wy_matchPMemUg *ddRmemUg;

    ddRmemUg = (wy_matchPMemUg *)malloc(sizeof(wy_matchPMemUg));
    if(!ddRmemUg)
    {
        HOST_PRINT("matchRidBodyProcessInit_wy DspSdk_memAlloc error!\n");//integer
        return WY_FAILURE;
    }
    memset((void *)ddRmemUg, 0, sizeof(wy_matchPMemUg));

    sfPic = &ddRmemUg->sfPic;
    ddRmemUg->magAngStride = WYVA_PIC_WIDTH_MAX;

    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&ddRmemUg->u32PhyAddrAge, ((void *)(&ddRmemUg->pAngle)), 
        WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX*sizeof(unsigned char)))
    {
        HOST_PRINT("hi3516MMZMalloc_wy pAngle malloc error!\n");//integer
        goto EXIT0;
    }

    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&ddRmemUg->u32PhyAddrMag, ((void *)(&ddRmemUg->pMag)), 
        WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX*sizeof(unsigned short)))
    {
        HOST_PRINT("hi3516MMZMalloc_wy pMag malloc error!\n");//integer
        goto EXIT0;
    }

    ddRmemUg->pBinVal= (unsigned char *)malloc(16*1024*sizeof(unsigned char));
    if(!ddRmemUg->pBinVal)
    {
        HOST_PRINT("hi3516MMZMalloc_wy pBinVal malloc error!\n");//integer
        goto EXIT0;
    }


//-----------------------------------------------------------------------------------------------------------------------------------------
    sfPic->stride = WYVA_PIC_WIDTH_MAX;//((tmp1+8+2)>>3)<<3;    

    if(WY_SUCCESS!=hi3516MMZMalloc_wy(&sfPic->u32PhyAddr, 
        ((void *)(&sfPic->byteBufAddr)), WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX))
    {
        HOST_PRINT("hi3516MMZMalloc_wy DspSdk_memAlloc error!\n");//integer
        goto EXIT0;
    }

    *ddRmemUgPtr = ddRmemUg;
    return WY_SUCCESS;
EXIT0:	
    matchRidBodyProcessExit_wy(ddRmemUg);
    return WY_FAILURE;
}

int matchRidBodyProcessExit_wy(wy_matchPMemUg *ddRmemUg)
{
   if(!ddRmemUg)
    return WY_FAILURE;
   
    if(ddRmemUg->pAngle)
        if(WY_SUCCESS!=hi3516MMZFree_wy(ddRmemUg->u32PhyAddrAge, (void *)(ddRmemUg->pAngle), 
        WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX*sizeof(unsigned char)))
        {
            HOST_PRINT("hi3516MMZFree_wy pAngle free error!\n");//integer
            return WY_FAILURE;
        }

    if(ddRmemUg->pMag)
        if(WY_SUCCESS!=hi3516MMZFree_wy(ddRmemUg->u32PhyAddrMag, (void *)(ddRmemUg->pMag),  
           WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX*sizeof(unsigned short)))
        {
               HOST_PRINT("hi3516MMZFree_wy pMag free error!\n");//integer
               return WY_FAILURE;
        }

   if(ddRmemUg->pBinVal)  
       free((void *)(ddRmemUg->pBinVal));
   

   if(ddRmemUg->sfPic.byteBufAddr)
       if(WY_SUCCESS!=hi3516MMZFree_wy(ddRmemUg->sfPic.u32PhyAddr, (void *)(ddRmemUg->sfPic.byteBufAddr),  
           WYVA_PIC_WIDTH_MAX*WYVA_PIC_HEIGHT_MAX*sizeof(unsigned char)))
       {
           HOST_PRINT("hi3516MMZFree_wy salorSfObjPic free error!\n");//integer
           return WY_FAILURE;
       }
             
   free((void *)ddRmemUg);
   
   return WY_SUCCESS;

}

int matchRigBodyProcess_wy(int *lx, int *ly, int *angle, wy_picInfo *pic, 
                 wy_objCharacterDes *objDesArray[], wy_matchPMemUg *memUg, WYVA_FLOAT matchVal, int rotate, WYVA_FLOAT *resMatchRate)
{
    int i, maxW, maxH;
    int lylx, rotateNum, tempDesIdx;
    wy_objCharacterDes *objCharacterDesTmp;
    unsigned char *binValL2 = memUg->pBinVal; 
    int numMatched, numMatchMax, retMatchNum;
	double matRate, matRateC;
#if(WY_DEBUG)
    FILE *fileOutFp=NULL; 
    int k,j;
    unsigned char magVal0, magVal1;
#endif  


#if(WY_DEBUG)

    fileOutFp = fopen("process512x288.yuv", "ab+");
    if(!fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
        for(j=0;j<pic->height;j++)
        {
            fwrite(pic->byteBufAddr+j*pic->stride, sizeof(unsigned char), pic->width, fileOutFp);
        }
        
		fflush(fileOutFp);
        fclose(fileOutFp);
    }
#endif

    memUg->sfPic.width = pic->width-2;
    memUg->sfPic.height = pic->height-2;
	
    smoothFilter_wyVA(pic->byteBufAddr, memUg->sfPic.byteBufAddr, 
                    pic->width, pic->height, pic->stride, memUg->sfPic.stride); //00000000000000000
//    _Smooth_U8_xsd(pic->byteBufAddr, memUg->sfPic.byteBufAddr, 
//               		pic->width, pic->height, pic->stride, memUg->sfPic.stride);
    //usleep(5);
    //xsd
/*    *lx=15;
	*ly=15;
	*angle=15;
	*resMatchRate = 0.9;
	return WY_OB_MATCH;		*/
#if(WY_DEBUG)

    fileOutFp = fopen("sf510x286.yuv", "ab+");
    if(!fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
        for(j=1;j<memUg->sfPic.height-1;j++)
        {
            fwrite(memUg->sfPic.byteBufAddr+j* memUg->sfPic.stride+1, sizeof(unsigned char), memUg->sfPic.width-2, fileOutFp);
        }
        
        fclose(fileOutFp);
    }
#endif

#if(HI3516_WY)
	HI_MPI_SYS_MmzFlushCache(memUg->sfPic.u32PhyAddr,memUg->sfPic.byteBufAddr,
	                                         memUg->sfPic.stride*memUg->sfPic.height);
	// check point
//	WY_SDK_ERR("va=%d, haha\n", vaEngaged);
	if(!vaEngaged)
	{
		WY_SDK_ERR("exit\n");
		return WY_FAILURE;
	}
    if(WY_SUCCESS!=calAngleAndMagMatrix_wyVA(memUg->sfPic.u32PhyAddr, memUg->u32PhyAddrAge,memUg->u32PhyAddrMag, 
                           memUg->sfPic.width, memUg->sfPic.height, memUg->sfPic.stride, memUg->magAngStride))
    return WY_FAILURE;
#else
	calAngleAndMagMatrix_wyVA((unsigned int)(memUg->sfPic.byteBufAddr), (unsigned int)(memUg->pAngle),
		(unsigned int)(memUg->pMag), memUg->sfPic.width, memUg->sfPic.height, memUg->sfPic.stride,  memUg->magAngStride);
#endif

//xsd
/*    *lx=11;
	*ly=11;
	*angle=11;
	*resMatchRate = 0.9;
	return WY_OB_MATCH;	*/

#if(WY_DEBUG)
    fileOutFp = fopen("angle.dat", "ab+");
    if(!fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
        for(k=0;k< (memUg->sfPic.height-2);k++)
        {

                fwrite(memUg->pAngle+k*memUg->magAngStride, sizeof(unsigned char),memUg->sfPic.width-2, fileOutFp);
        }

        fclose(fileOutFp);
    }


    fileOutFp = fopen("mag.dat", "ab+");
    if(!fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
        for(k=0;k< (memUg->sfPic.height-2);k++)
        {
            for(j=0;j<(memUg->sfPic.width-2);j++)
            {

                magVal0 = (unsigned char)(*(memUg->pMag + k*memUg->magAngStride + j)&0xff);
                magVal1 = (unsigned char)((*(memUg->pMag + k*memUg->magAngStride + j)&0xff00)>>8);

                fwrite(&magVal0, sizeof(unsigned char),1, fileOutFp);
                fwrite(&magVal1, sizeof(unsigned char),1, fileOutFp);

            }
        }

        fclose(fileOutFp);
    }

#endif

#if(WY_DEBUG)
    fileOutFp = fopen("sf510x286After.yuv", "ab+");
    if(!fileOutFp)
    {
        printf("file open error!");
    }

    if(fileOutFp)
    {
        for(j=1;j<memUg->sfPic.height-1;j++)
        {
            fwrite(memUg->sfPic.byteBufAddr+j* memUg->sfPic.stride+1, sizeof(unsigned char), memUg->sfPic.width-2, fileOutFp);
        }
        
        fclose(fileOutFp);
    }
#endif

#if(HI3516_WY)
	// check point
//	WY_SDK_ERR("va=%d, haha\n", vaEngaged);
	if(!vaEngaged)
	{
		WY_SDK_ERR("exit\n");
		return WY_FAILURE;
	}
    getMaxBinMatrix(binValL2, memUg->pAngle+memUg->magAngStride+1,memUg->pMag+memUg->magAngStride+1, 
                        memUg->magAngStride, memUg->sfPic.width-2,memUg->sfPic.height-2);
#else
    getMaxBinMatrix(binValL2, memUg->pAngle,memUg->pMag, 
					memUg->magAngStride, memUg->sfPic.width-2,memUg->sfPic.height-2);
#endif
	//xsd
/*	*lx=12;
   	*ly=12;
	*angle=12;
	*resMatchRate = 0.9;
	return WY_OB_MATCH; */

    maxW = (memUg->sfPic.width-2)/5;
    maxH = (memUg->sfPic.height-2)/5;
//    rotateNum = rotate ? 36:1;
	rotateNum = rotate?3:1;

#if(WY_DEBUG)		 
	   fileOutFp = fopen("binDes.dat", "ab+");
	   if(fileOutFp)
	   {
		   printf("file open error!");
	   }

	   if(fileOutFp)
	   {
		   fwrite(binValL2, sizeof(unsigned char),maxW*maxH, fileOutFp);
		   fclose(fileOutFp);
	   }
#endif

	matRate = 0;
	tempDesIdx = -1;
    for(i=0;i<rotateNum;i++)
    {   		
        numMatched = (int)(objDesArray[i]->obPixelNum*matchVal);
		numMatchMax = (int)(objDesArray[i]->obPixelNum*0.85);
		
        objCharacterDesTmp = objDesArray[i];

        lylx = desMatchProcess_wyVA(maxW, maxH, binValL2, objCharacterDesTmp->obRecWidthS,  
                              objCharacterDesTmp->obRecHeightS, objCharacterDesTmp->pData, numMatched, &retMatchNum);
		//usleep(10);
		matRateC = ((double)retMatchNum)/objDesArray[i]->obPixelNum;
//		lylx = 1;
//		matRateC = 0.95;

		if(matRateC > *resMatchRate)
			*resMatchRate = matRateC;
		
		//printf("********matchVal %f matRateC %f retMatchNum %d %d\n", matchVal, matRateC, retMatchNum, objCharacterDesTmp->angle);
        if(-1!=lylx && retMatchNum >= numMatchMax)
        {			 
            *lx = (lylx&0xFFFF)*5 + (objCharacterDesTmp->obRecWidthS*5>>1) + 2; 
            *ly = (lylx>>16)*5 + (objCharacterDesTmp->obRecHeightS*5>>1) + 2; 
            *angle = objCharacterDesTmp->angle;

            objDesArray[i] = objDesArray[0];
            objDesArray[0] = objCharacterDesTmp; 
            return WY_OB_MATCH;
        } 
		else if(-1!=lylx && matRateC>matRate)
		{
            *lx = (lylx&0xFFFF)*5 + (objCharacterDesTmp->obRecWidthS*5>>1) + 2; 
            *ly = (lylx>>16)*5 + (objCharacterDesTmp->obRecHeightS*5>>1) + 2; 
            *angle = objCharacterDesTmp->angle;
			matRate = matRateC;
			tempDesIdx = i;
		}
		
    }

	if(tempDesIdx>-1)	
	{
		objCharacterDesTmp = objDesArray[tempDesIdx];
		objDesArray[tempDesIdx] = objDesArray[0];
		objDesArray[0] = objCharacterDesTmp; 
		return WY_OB_MATCH;
	}
	
    return WY_OB_NMATCH;
}

























