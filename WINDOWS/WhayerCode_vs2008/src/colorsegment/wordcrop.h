#ifndef _WORD_CROP_H_
#define _WORD_CROP_H_

#include "licomdef.h"
#include "colorsegment.h"
#include "areagraph.h"
#include "skinarea.h"
#include "limask.h"
#include "liblock.h"
#include "skinextract.h"
/*
typedef struct tag_BlobIndex
{
	MLong index;
	MDouble dVal;
	MDouble dConfidence;
	MRECT rtBlob;
}BLOBINDEX;

typedef struct tag_wordblob
{
	BLOBINDEX *pIndexList;
	MLong lIndexNum;

	MRECT rtWord;
}WORD_BLOB;
*/

typedef struct tag_RectConf
{
	MRECT rtRange;
	MDouble dConfi;
	MDouble dVal;
}RECT_CONF;

typedef enum tag_wordType
{
	WORD_LEAF=0,
	WORD_NODE=1,
}WORDTYPE;

///word_blob�����˵���blob���Ƕ��blob�����
typedef struct tag_wordblob
{	
	//Я������һLayer���׸�WORD_BLOB
	MLong lChildNum;
	struct tag_wordblob *pChildList;

	///ͬһLayer�µ�WORD_BLOB�б��������Ŀ���ϲ��Word_BLOB����	
	struct tag_wordblob *pNext;

	///�����leaf�����豣��Index��Ϣ�������node��Ҫ������
	MLong lIndex;///
	WORDTYPE type;///

	MChar dUnit;
	MBool dlabel;
	MDouble dVal;
	MDouble dConfidence;
	MRECT rtWord;
	
	MLong lBoundaryNum;
	MLong lGrayMean;
	MLong lSize;
	
}WORD_BLOB;

typedef struct tag_wordbloblist
{
	///used to store the word_blob buffer
	WORD_BLOB *pWordBlobBuffer;
	MLong lCurNum;
	MLong lMaxWordBlob;

	///
	WORD_BLOB* pFreeList;
	MLong lFreeNum;

	///word_blob tree, ����ֻ��һ�����ڵ������������Ҫ������ڵ�
	WORD_BLOB *pWordBlob;
	MLong lBlobNum;
}WORD_BLOB_LIST;

///


#define WordBlobInit		PX(WordBlobInit)
#define WordBlobUninit		PX(WordBlobUninit)
#define WordCrop			PX(WordCrop)
#define WordBlobHorizontal	PX(WordBlobHorizontal)
#define CharCrop			PX(CharCrop)
#define WordBlobGrayStatistics	PX(WordBlobGrayStatistics)

#define TAIL_OF_WORDBLOB(x, pTail)	{WORD_BLOB* pTmp=(WORD_BLOB*)(x);while(pTmp->pNext!=MNull)pTmp=pTmp->pNext;pTail=pTmp;}

#ifdef __cplusplus
extern "C" {
#endif

MRESULT	WordBlobInit(MHandle hMemMgr, WORD_BLOB_LIST *pWordBlobList, MLong lBlobNum);
MRESULT WordBlobUninit(MHandle hMemMgr, WORD_BLOB_LIST *pWordBlobList);
MRESULT WordCrop(MHandle hMemMgr, INFO_SKIN *pInfoSkin, WORD_BLOB_LIST* pWordList);
MBool WordBlobHorizontal(WORD_BLOB *pWordBlob);
MRESULT CharCrop(MHandle hMemMgr, MByte* pImgData, MLong lDataLine, MLong lWidth, MLong lHeight, 
	MLong lEstCharWidth,RECT_CONF* pRectList, MLong lMaxRectNum, MLong *plRectNum);
MRESULT WordBlobGrayStatistics(MHandle hMemMgr, INFO_SKIN *pInfoSkin, 
		WORD_BLOB* pWordBlob, MByte *pImgData, MLong lImgLine, 
		MLong lWidth, MLong lHeight, MLong *pMeanGrayVal, MLong *pMeanMaskGrayVal);
MRESULT	WordBlobSort(MHandle hMemMgr,JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelSeed,MLong *WordBlobIndex);
MRESULT CheckWordList(WORD_BLOB_LIST *pWordList);
#ifdef __cplusplus
}
#endif

#endif //_WORD_CROP_H_
