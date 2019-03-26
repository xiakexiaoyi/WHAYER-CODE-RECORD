#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include <stdio.h>
#include <stdlib.h>
#include "wordcrop.h"
#include "litrimfun.h"
#include "liimage.h"
#include "lierrdef.h"
#include "limem.h"
#include "lidebug.h"
#include "limath.h"
#include "limask.h"
#include "skinarea.h"
#include "skinextract.h"
#include "bbgeometry.h"
#include "math.h"
#include "lisort.h"
#include "bbedge.h"
#include "histogram.h"
#include <math.h>
#ifdef ENABLE_PRIOR 
#include "priorarea.h"
#endif

#ifdef ENABLE_DEBUG
extern JLabelData *g_pLabelData;
extern MLong g_lLabelLine, g_lWidth, g_lHeight;
#endif

MRESULT	WordBlobSort(MHandle hMemMgr,JINFO_AREA_GRAPH *pInfoGraph, MLong lLabelSeed,MLong *WordBlobIndex)
{
	MRESULT res = LI_ERR_NONE;
	MLong i,j,temp;
	MDWord k =0;
	MDWord *DistanceWithBlobs = MNull;
	JINFO_AREA *pAreaSeed = pInfoGraph->pInfoArea + lLabelSeed;
	AllocVectMem(hMemMgr,DistanceWithBlobs,pInfoGraph->lLabelNum,MDWord);
	JMemCpy(DistanceWithBlobs,pInfoGraph->pInfoArea[lLabelSeed].pDistanceWithBlobs,pInfoGraph->lLabelNum*sizeof(MDWord));

	for(j=0;j<pInfoGraph->lLabelNum-1;j++)
	{
		for(i=0;i<pInfoGraph->lLabelNum-1-j;i++)
		{
			if(DistanceWithBlobs[i]>DistanceWithBlobs[i+1])
			{
				temp = WordBlobIndex[i];
				k = DistanceWithBlobs[i];
				WordBlobIndex[i] = WordBlobIndex[i+1];
				DistanceWithBlobs[i] = DistanceWithBlobs[i+1];
				WordBlobIndex[i+1] = temp;
				DistanceWithBlobs[i+1] = k;
			}
		}
	}


EXT:
	FreeVectMem(hMemMgr,DistanceWithBlobs);
	return res;
}

///\@param1	hMemMgr:handle for memory allocate
///\@param2 pWordBlobList:返回的指针句柄
///\@param3 lMaxBlob:结构体中需分配的最大的字串数目
///\@param4 lMaxIndex:结构体中需分配每个字串最大包含的索引数目，该索引值是JINFO_AREA_GRAPH中的索引值
///\@return value: 错误的返回值
MRESULT	WordBlobInit(MHandle hMemMgr, WORD_BLOB_LIST *pWordBlobList, MLong lBlobNum)
{
	MRESULT res = LI_ERR_NONE;
	MLong lMaxBlobNum = 0;
	if (pWordBlobList==MNull || lBlobNum<=0)
		{res = LI_ERR_INVALID_PARAM;goto EXT;}

	///根据lBlobNum，计算最大的wordblob 数目，最大满足两两组合形成一个hauffman树
	lMaxBlobNum = 0;
	lBlobNum *=lBlobNum;
	while(lBlobNum>0)
		{lMaxBlobNum+=lBlobNum; lBlobNum/=2;}

	AllocVectMem(hMemMgr, pWordBlobList->pWordBlobBuffer, lMaxBlobNum, WORD_BLOB);
	SetVectZero(pWordBlobList->pWordBlobBuffer, lMaxBlobNum*sizeof(WORD_BLOB));
	pWordBlobList->lMaxWordBlob = lMaxBlobNum;
	pWordBlobList->lCurNum = 0;
	pWordBlobList->pWordBlob = MNull;
	pWordBlobList->lBlobNum = 0;
	pWordBlobList->lFreeNum=0;

	//append all node to freelist
	while(pWordBlobList->lCurNum<pWordBlobList->lMaxWordBlob)
	{
		WORD_BLOB *pTmpWord = pWordBlobList->pWordBlobBuffer+pWordBlobList->lCurNum;
		if (!pWordBlobList->pFreeList)
		{
			pWordBlobList->pFreeList = pTmpWord;
			pWordBlobList->lFreeNum++;
		}
		else
		{
			pTmpWord->pNext = pWordBlobList->pFreeList;
			pWordBlobList->pFreeList = pTmpWord;
			pWordBlobList->lFreeNum++;
		}
		pWordBlobList->lCurNum++;
	}

EXT:
	return res;
}

MRESULT WordBlobUninit(MHandle hMemMgr, WORD_BLOB_LIST *pWordBlobList)
{
	if (pWordBlobList && pWordBlobList->pWordBlobBuffer)
		FreeVectMem(hMemMgr, pWordBlobList->pWordBlobBuffer);
	return LI_ERR_NONE;
}

/*
///从左到右进行排序
MLong	IndexCmp(const MVoid *p1, const MVoid *p2)
{
	BLOBINDEX*_p1 = (BLOBINDEX *)p1;
	BLOBINDEX *_p2 = (BLOBINDEX *)p2;

	JASSERT(_p1);
	JASSERT(_p2);

	
	if (_p1->index>=_p2->index)return 1;
	else 
		return -1;		
}

///升序排列
MLong rectPosCmp(const MVoid *p1, const MVoid *p2)
{
	BLOBINDEX*_p1 = (BLOBINDEX *)p1;
	BLOBINDEX *_p2 = (BLOBINDEX *)p2;

	JASSERT(_p1);
	JASSERT(_p2);
	
	if (_p1->rtBlob.left >_p2->rtBlob.left) return 1;
	else 
		return -1;
}
*/

///对所有叶子WordBlob按lIndex升序排列，利于比较
MVoid AscendingLeafWordBlob(WORD_BLOB *pWordBlob)
{
	WORD_BLOB *pHead = MNull, *pTmp=MNull, *pTmpPrev, *pTmp2, *pTmp3;
	if (!pWordBlob || pWordBlob->lChildNum<=0)return;

	if (pWordBlob->type == WORD_LEAF)return;

	pHead = pWordBlob->pChildList;
	pWordBlob->pChildList = pWordBlob->pChildList->pNext;
	pHead->pNext = MNull;

	pTmp2 = pWordBlob->pChildList;
	while(pTmp2)
	{
		pTmp3 = pTmp2;
		pTmp2 = pTmp3->pNext;
		pTmp3->pNext = MNull;

		pTmp = pHead;
		pTmpPrev = MNull;

		while(pTmp)
		{
			///compare the index value
			if (pTmp->lIndex > pTmp3->lIndex)break;
			pTmpPrev = pTmp;
			pTmp = pTmp->pNext;
		}
		if (!pTmpPrev)
		{	pHead = pTmp3;pHead->pNext = pTmp;}
		else
		{
			pTmpPrev->pNext = pTmp3;
			pTmp3->pNext = pTmp;
		}
	}

	pWordBlob->pChildList = pHead;
}


///
MLong PositionCmp(const MVoid *p1, const MVoid *p2)
{
	WORD_BLOB *_p1 = (WORD_BLOB*)p1;
	WORD_BLOB *_p2 = (WORD_BLOB*)p2;

	if (_p1->rtWord.left < _p2->rtWord.left)
		return -1;
	else	if (_p1->rtWord.left > _p2->rtWord.left)
		return 1;
	else 	return 0;
}
MVoid AscendingWordBlob(WORD_BLOB *pWordBlob, MLong (*cmp)(const MVoid*, const MVoid*))
{
	WORD_BLOB *pHead = MNull, *pTmp=MNull, *pTmpPrev, *pTmp2, *pTmp3;
	if (!pWordBlob || pWordBlob->lChildNum<=0)return;

	if (pWordBlob->type == WORD_LEAF)return;

	pHead = pWordBlob->pChildList;
	pWordBlob->pChildList = pWordBlob->pChildList->pNext;
	pHead->pNext = MNull;

	pTmp2 = pWordBlob->pChildList;
	while(pTmp2)
	{
		pTmp3 = pTmp2;
		pTmp2 = pTmp3->pNext;
		pTmp3->pNext = MNull;

		pTmp = pHead;
		pTmpPrev = MNull;

		while(pTmp)
		{
			///compare the index value
			//if (pTmp->lIndex > pTmp3->lIndex)break;
			if (cmp((MVoid*)pTmp, (MVoid*)pTmp3)>0)break;
			pTmpPrev = pTmp;
			pTmp = pTmp->pNext;
		}
		if (!pTmpPrev)
		{	pHead = pTmp3;pHead->pNext = pTmp;}
		else
		{
			pTmpPrev->pNext = pTmp3;
			pTmp3->pNext = pTmp;
		}
	}

	pWordBlob->pChildList = pHead;
}


MBool IsEqualWordBlob(WORD_BLOB *pWordBlob1, WORD_BLOB* pWordBlob2)
{
	if (pWordBlob1->type!=pWordBlob2->type)return MFalse;
	if (pWordBlob1->type==WORD_LEAF)
	{
		return pWordBlob1->lIndex==pWordBlob2->lIndex;
	}
	if (pWordBlob1->lChildNum!=pWordBlob2->lChildNum)
		return MFalse;
	else
	{
		WORD_BLOB* pCurBlob1 = pWordBlob1->pChildList;
		WORD_BLOB* pCurBlob2 = pWordBlob2->pChildList;
		
		while (pCurBlob1&&pCurBlob2)
		{
			if (!IsEqualWordBlob(pCurBlob1, pCurBlob2))
				break;
			pCurBlob1 = pCurBlob1->pNext;
			pCurBlob2 = pCurBlob2->pNext;
		}
		if (pCurBlob1 || pCurBlob2)
			return MFalse;
		else
			return MTrue;
	}
}

MVoid FreeWordBlob(WORD_BLOB_LIST *pWordBlobList, WORD_BLOB *pCurWordBlob)
{
	WORD_BLOB *pTmp, *pTmp2;
	if (!pWordBlobList || !pCurWordBlob)return;
	if (pCurWordBlob->type == WORD_NODE)
	{
		pTmp = pCurWordBlob->pChildList;
		while (pTmp)
		{
			pTmp2 = pTmp;
			pTmp = pTmp->pNext;
			FreeWordBlob(pWordBlobList, pTmp2);
		}
	}

	pCurWordBlob->pNext =	pWordBlobList->pFreeList;
	pWordBlobList->pFreeList = pCurWordBlob;
	pWordBlobList->lFreeNum++;
	return;
}

///sort the index
MVoid UpdateWordBlobList(WORD_BLOB_LIST *pWordBlobList)
{
	WORD_BLOB *pCurWordBlob, *pHead, *pTmp, *pTmp2, *pTmp3;
	if (!pWordBlobList || !pWordBlobList->pWordBlob)return;

	pCurWordBlob =  pWordBlobList->pWordBlob;
	for (int i=0; i<pWordBlobList->lBlobNum && pCurWordBlob; i++)
	{
		//排序，以便于后续的一对一匹配
		AscendingLeafWordBlob(pCurWordBlob);///这个函数还需要调试
		pCurWordBlob = pCurWordBlob->pNext;
	}
	
	///delete the same blob list
	pHead = pWordBlobList->pWordBlob;
	pWordBlobList->pWordBlob = pWordBlobList->pWordBlob->pNext;
	pHead->pNext = MNull;

	pTmp = pWordBlobList->pWordBlob;
	while (pTmp)
	{
		pTmp3 = pTmp;
		pTmp = pTmp->pNext;
		pTmp3->pNext = MNull;
		
		pTmp2 = pHead;
		while (pTmp2)
		{
			if (IsEqualWordBlob(pTmp3, pTmp2))
			{
				FreeWordBlob(pWordBlobList, pTmp3);
				pWordBlobList->lBlobNum--;
				break;
			}
			pTmp2 = pTmp2->pNext;
		}
		if (!pTmp2)///如果无重叠
		{
			pTmp3->pNext = pHead;
			pHead = pTmp3;
		}
	}
	pWordBlobList->pWordBlob = pHead;
	
	return;
}

///更新WordBlob的Rectangle 信息
MVoid UpdateWordInfo(INFO_SKIN *pGraph, WORD_BLOB *pWordBlob, MLong lExtPercent)
{
	WORD_BLOB *pTmp;
	if (!pGraph || !pWordBlob)return;
	if (pWordBlob->type == WORD_LEAF)
	{
		pWordBlob->rtWord.left = pGraph->infoGraph.pInfoArea[pWordBlob->lIndex].rtArea.left;
		pWordBlob->rtWord.right = pGraph->infoGraph.pInfoArea[pWordBlob->lIndex].rtArea.right;
		pWordBlob->rtWord.top = pGraph->infoGraph.pInfoArea[pWordBlob->lIndex].rtArea.top ;
		pWordBlob->rtWord.bottom = pGraph->infoGraph.pInfoArea[pWordBlob->lIndex].rtArea.bottom;
		pWordBlob->lSize = pGraph->infoGraph.pInfoArea[pWordBlob->lIndex].lSize;

		///增加BoundaryNum的数量统计，用于后续的confidence计算
		pWordBlob->lBoundaryNum = pGraph->infoGraph.pInfoArea[pWordBlob->lIndex].lBoundaryNum;

		///外延lExtPercentage像素
		if (lExtPercent>0)
		{
			MLong lWidthExt = (pWordBlob->rtWord.right -pWordBlob->rtWord.left )*lExtPercent/100;
			MLong lHeightExt = (pWordBlob->rtWord.bottom-pWordBlob->rtWord.top )*lExtPercent/100;
			lWidthExt = MAX(lWidthExt, 2);
			lHeightExt = MAX(lHeightExt,2);
			pWordBlob->rtWord.left = MAX(0, pWordBlob->rtWord.left - lWidthExt);
			pWordBlob->rtWord.top = MAX(0, pWordBlob->rtWord.top -lHeightExt);
			pWordBlob->rtWord.right = MIN(pGraph->lWidth, pWordBlob->rtWord.right +lWidthExt);
			pWordBlob->rtWord.bottom= MIN(pGraph->lHeight,pWordBlob->rtWord.bottom+lHeightExt);
		}
			
		return;
	}

	pTmp = pWordBlob->pChildList;
	while(pTmp)
	{
		UpdateWordInfo(pGraph, pTmp, lExtPercent);
		if (pTmp==pWordBlob->pChildList)
		{
			pWordBlob->rtWord.left = pTmp->rtWord.left;
			pWordBlob->rtWord.right = pTmp->rtWord.right;
			pWordBlob->rtWord.top = pTmp->rtWord.top;
			pWordBlob->rtWord.bottom = pTmp->rtWord.bottom;

			///增加BoundaryNum的数量统计，用于后续的confidence计算
			pWordBlob->lBoundaryNum = pTmp->lBoundaryNum;
			pWordBlob->lSize = pTmp->lSize;
		}
		else
		{
			pWordBlob->rtWord.left = MIN(pWordBlob->rtWord.left,pTmp->rtWord.left);
			pWordBlob->rtWord.right = MAX(pTmp->rtWord.right,pWordBlob->rtWord.right );
			pWordBlob->rtWord.top = MIN(pTmp->rtWord.top,pWordBlob->rtWord.top);
			pWordBlob->rtWord.bottom = MAX(pTmp->rtWord.bottom,pWordBlob->rtWord.bottom);

			///++所有合法的BoundaryNum
			pWordBlob->lBoundaryNum += pTmp->lBoundaryNum;
			pWordBlob->lSize += pTmp->lSize;
		}
		pTmp = pTmp->pNext;
	}
		
}

MVoid IndexTraversal(WORD_BLOB *pWordBlob, MLong *pMemIndex, MLong lMemLength, MLong *pIndexNum)
{
	WORD_BLOB *pTmp;
	if (!pWordBlob)return;
	if (pWordBlob->type == WORD_LEAF)
	{
		if (pMemIndex && pIndexNum && *pIndexNum<lMemLength)
		{
			pMemIndex[*pIndexNum] = pWordBlob->lIndex;
		}
		if (pIndexNum)
			(*pIndexNum)++;
		return;
	}
	///如果是WORD_NODE
	if (!pWordBlob->pChildList || pWordBlob->lChildNum<=0)return;
	///广度遍历
	pTmp = pWordBlob->pChildList;
	while(pTmp)
	{
		IndexTraversal(pTmp, pMemIndex, lMemLength, pIndexNum);
		pTmp=pTmp->pNext;
	}
}

MRESULT MergeWordBlob(MHandle hMemMgr, INFO_SKIN *pInfoSkin, 
	WORD_BLOB *pBlobTo, WORD_BLOB* pBlobFrom, MBool *pResult)
{
	MRESULT res = LI_ERR_NONE;
	MLong i, j;
	MLong lMean, lNum;
	WORD_BLOB  *pTmp1, *pTmp2,*ChildBlob;
	MLong lIndexNumFrom, lIndexNumTo;
	MLong *pMem1=MNull, *pMem2=MNull;
	MLong lMemLength1, lMemLength2;
	MLong lMinDist;
	MLong lY1, lCb1, lCr1, lY2,lCb2, lCr2;
	MLong lSizeFrom, lSizeTo;
	MLong lMaxRate;
	MCOLORREF ref;
	//将BlobFrom放入到pBlobTo的ChildList中去比较

	///比较的条件

	///overlapping region
	if(pBlobTo->lChildNum>=2)
	{
		ChildBlob = pBlobTo->pChildList;
		while(ChildBlob)
		{
			vGetOverLappingRate(pBlobFrom->rtWord,ChildBlob->rtWord,MNull, &lMaxRate);
			if (lMaxRate>MAX_RT_OVERLAPPINGRATE)
			{
				if (pResult)*pResult=MFalse;goto EXT;
			}
			ChildBlob=ChildBlob->pNext;
		}			
	}
	else
	{
		vGetOverLappingRate(pBlobFrom->rtWord, pBlobTo->rtWord, MNull, &lMaxRate);
		if (lMaxRate>MAX_RT_OVERLAPPINGRATE)
		{
			if (pResult)*pResult=MFalse;goto EXT;
		}		
	}
	
	///1.alike height
	lMean = 0;
	lNum = 0;
	pTmp1 = pBlobTo->pChildList;////host, 与pBlobTo的每一个child进行比较
	pTmp2 = pBlobFrom;

	while (pTmp1)
	{
		lMean += pTmp1->rtWord.bottom - pTmp1->rtWord.top;
		lNum++;
		pTmp1=pTmp1->pNext;
	}
	if (lNum!=0)
		lMean /=lNum;
	if (OffsetValue(lMean, pBlobFrom->rtWord.bottom-pBlobFrom->rtWord.top, MAX_MEAN_OFFSET)<0)///平均高度偏移太大
		{
		if (pResult)*pResult=MFalse;
		goto EXT;
		};

	///2.alike distance
	///目的是为了寻找两个blob之间最近的距离
	///retireve the counts of Index
	lMemLength1= 0;
	lMemLength2= 0;
	IndexTraversal(pBlobFrom, MNull, 0, &lMemLength1);
	IndexTraversal(pBlobTo, MNull, 0, &lMemLength2);
	AllocVectMem(hMemMgr, pMem1, lMemLength1, MLong);
	SetVectZero(pMem1, lMemLength1*sizeof(MLong));
	AllocVectMem(hMemMgr, pMem2, lMemLength2, MLong);
	SetVectZero(pMem2, lMemLength2*sizeof(MLong));

	//accually retireve the index
	lIndexNumFrom = 0;
	lIndexNumTo = 0;
	IndexTraversal(pBlobFrom, pMem1, lMemLength1, &lIndexNumFrom);
	IndexTraversal(pBlobTo, pMem2, lMemLength2, &lIndexNumTo);

	///
	lMinDist = 0x7FFFFFFF;
	for (i=0; i<lIndexNumTo; i++)
	{
		for (j=0; j<lIndexNumFrom; j++)
		{
			if (lMinDist>pInfoSkin->infoGraph.pInfoArea[pMem2[i]].pDistanceWithBlobs[pMem1[j]])
				lMinDist = pInfoSkin->infoGraph.pInfoArea[pMem2[i]].pDistanceWithBlobs[pMem1[j]];
		}
	}
	lMinDist = (MLong)sqrt((MDouble)lMinDist);
	///pDistnaceWithBlobs里面的数据是平方的
	if (!VALID_RELATIVE_DISTANCE(lMinDist, lMean))
		{
		if (pResult)*pResult=MFalse;
		goto EXT;
		}

	///3.alike color
	lY1=lY2=lCb1=lCb2=lCr1=lCr2=0;
	lSizeFrom=0;lSizeTo=0;
	for (i=0; i<lIndexNumFrom; i++)
	{
		lY1+=pInfoSkin->infoGraph.pInfoArea[pMem1[i]].lY;
		lCb1+=pInfoSkin->infoGraph.pInfoArea[pMem1[i]].lCb;
		lCr1+=pInfoSkin->infoGraph.pInfoArea[pMem1[i]].lCr;
		lSizeFrom+=pInfoSkin->infoGraph.pInfoArea[pMem1[i]].lSize;
	}
	ref = PIXELVAL(lY1/lSizeFrom, lCb1/lSizeFrom, lCr1/lSizeFrom);
	for (i=0;i<lIndexNumTo; i++)
	{
		lY2+=pInfoSkin->infoGraph.pInfoArea[pMem2[i]].lY;
		lCb2+=pInfoSkin->infoGraph.pInfoArea[pMem2[i]].lCb;
		lCr2+=pInfoSkin->infoGraph.pInfoArea[pMem2[i]].lCr;
		lSizeTo+=pInfoSkin->infoGraph.pInfoArea[pMem2[i]].lSize;
	}
	if (GetColorDist(lY2/lSizeTo, lCb2/lSizeTo, lCr2/lSizeTo, ref)>MAX_COLOR_DIST*7/8)
		{
		if (pResult)*pResult=MFalse;
		goto EXT;
		}
	///
	
	if (pResult) *pResult = MTrue;
	
EXT:
	if (res<0)
		if (pResult)*pResult=MFalse;
	FreeVectMem(hMemMgr, pMem1);
	FreeVectMem(hMemMgr, pMem2);
	return res;
}


MRESULT WordBlobReorganize(MHandle hMemMgr, INFO_SKIN *pInfoSkin, WORD_BLOB_LIST *pWordList)
{
	MRESULT res = LI_ERR_NONE;
	MBool bReorganize = MFalse;
	MBool bMerged;
	WORD_BLOB *pTmp, *pTmp2, *pTmp3, *pCurBlob, *pTmp4,*pTmp5;
	if (!pInfoSkin || !pWordList || pWordList->lBlobNum<=0){res = LI_ERR_INVALID_PARAM;goto EXT;}

	pCurBlob = pWordList->pWordBlob;
	JASSERT(pCurBlob);

	///基础层的blob只增不删
	do
	{
		bReorganize = MFalse;
		pTmp = pCurBlob;
		while (pTmp)
		{
			pTmp2 = pTmp;
			pTmp=pTmp->pNext;

			///用当前的blob再去遍历对比整个bloblist
			pTmp3 = pCurBlob;
			while(pTmp3)
			{
				pTmp4 = pTmp3;
				pTmp3=pTmp3->pNext;

				if (pTmp2==pTmp4)continue;///如果是相同的
				//judge && reorganize
				GO(MergeWordBlob(hMemMgr, pInfoSkin, pTmp2, pTmp4, &bMerged));
				if (bMerged)	
				{///execute merge operation
					///borrow a free node
					WORD_BLOB *pNewBlob = pWordList->pFreeList;
					if (pWordList->pFreeList)
					{
						pWordList->pFreeList = pWordList->pFreeList->pNext;
						pWordList->lFreeNum --;
					}
					if(pNewBlob) 
					{
						///把所有的child都带上
						JMemCpy(pNewBlob, pTmp4, sizeof(WORD_BLOB));
						pNewBlob->pNext = MNull;
						///insert it to destination, but keep the original
						///这里的问题：应该是childnum数目大的node去合并数目小的
						///以下所示的是tmp2去合并tmp4
						pTmp2->lChildNum++;
						pNewBlob->pNext = pTmp2->pChildList;
						pTmp2->pChildList = pNewBlob;
						pTmp2->rtWord.left = MIN(pTmp2->rtWord.left, pNewBlob->rtWord.left);
						pTmp2->rtWord.right = MAX(pTmp2->rtWord.right, pNewBlob->rtWord.right);
						pTmp2->rtWord.top = MIN(pTmp2->rtWord.top, pNewBlob->rtWord.top);
						pTmp2->rtWord.bottom = MAX(pTmp2->rtWord.bottom, pNewBlob->rtWord.bottom);
					}
					pTmp3 = pCurBlob;
					bReorganize = MTrue;
				}
			}
		}
	}while(bReorganize);
	
EXT: 
	return res;
}


MBool WordBlobHorizontal(WORD_BLOB *pWordBlob)
{
	//判断各个childblob之间rtBlob的关系，参考前后blob之间的关系即可
	WORD_BLOB *pChildBlob;
	MBool bretVal;
	if (!pWordBlob)return MFalse;
	pChildBlob = pWordBlob->pChildList;

	bretVal = MTrue;
	while (pChildBlob && pChildBlob->pNext)
		{
			MPOINT pt1, pt2;
			MDouble dtheta;
			pt1.x = (pChildBlob->rtWord.left+pChildBlob->rtWord.right)/2;
			pt1.y = (pChildBlob->rtWord.top+pChildBlob->rtWord.bottom)/2;
			pt2.x = (pChildBlob->pNext->rtWord.left+pChildBlob->pNext->rtWord.right)/2;
			pt2.y = (pChildBlob->pNext->rtWord.top+pChildBlob->pNext->rtWord.bottom)/2;
			dtheta = vCalculateTheta(pt1, pt2);
			if (dtheta>V_PI/4 && dtheta<V_PI*3/4) 
			{
				bretVal = MFalse;
				break;
			}
			
			pChildBlob = pChildBlob->pNext;
		}
	return bretVal;
}

typedef enum tag_mergeStatus
{
	UNKNOWN = 0,
	HORIZTONTAL_MERGE,
	VERTICAL_MERGE,
}MERGE_STATUS;

MRESULT WordCrop(MHandle hMemMgr, INFO_SKIN *pInfoSkin, WORD_BLOB_LIST* pWordList)
{
	///first, traversal the area list, to form blob list with nearby height
	MRESULT res = LI_ERR_NONE;
	MLong i, j, k;
	MPOINT ptG1, ptG2;
	MLong lLeafNum = 0;
	MBool bMerge = MFalse;
	MCOLORREF crRef = 0;
	JINFO_AREA *pCurArea=MNull;
	MERGE_STATUS lMergeStatus = UNKNOWN;

	JASSERT(pInfoSkin);
	JASSERT(pWordList);

	JINFO_AREA_GRAPH *pInfoGraph = &pInfoSkin->infoGraph;
	WORD_BLOB_LIST SwapWordBlob = {0};
	WORD_BLOB *pCurBlob;

	MLong *BlobIndex =MNull ;
	AllocVectMem(hMemMgr, BlobIndex, pInfoGraph->lLabelNum, MLong);

	/// 
	lLeafNum=0;
	for (i=0; i<pInfoGraph->lLabelNum; i++)
		if (pInfoGraph->pInfoArea[i].lFlag>0)lLeafNum++;

	///as a swap region
	AllocVectMem(hMemMgr, SwapWordBlob.pWordBlobBuffer, lLeafNum, WORD_BLOB);
	SetVectZero(SwapWordBlob.pWordBlobBuffer, lLeafNum*sizeof(WORD_BLOB));
	SwapWordBlob.lMaxWordBlob = lLeafNum;

	pWordList->lBlobNum = 0;
	pWordList->pWordBlob = MNull;
	pWordList->lCurNum = 0;
	for (i=0; i<pInfoGraph->lLabelNum; i++)
	{
		if (pInfoGraph->pInfoArea[i].lFlag<=0)continue;
		for (MLong k=0; k<pInfoGraph->lLabelNum; k++)
			BlobIndex[k] = k;
		WordBlobSort(hMemMgr,pInfoGraph, i,BlobIndex);
		///every area, look for nearby blob list
		SwapWordBlob.lCurNum= 0;
		SwapWordBlob.pWordBlobBuffer[SwapWordBlob.lCurNum].lIndex = i;
		SwapWordBlob.lCurNum++;
		lMergeStatus = UNKNOWN;
		do
		{
			bMerge = MFalse;
			for (j=0; j<pInfoGraph->lLabelNum-1; j++)
			{
				JINFO_AREA *pAlikeArea = pInfoGraph->pInfoArea+BlobIndex[j];
				MLong lMean=0,lMean1=0, lVariance=0, lDist=0;
				MLong lMinDist=0x7FFFFFFF,lMinDistX=-1;
				MLong lMaxRate = 0;
				
				 if (pAlikeArea->lFlag<=0)continue;
				 
				///check if swapwordblob has contain the index
				for(k=0; k<SwapWordBlob.lCurNum; k++)
					if (BlobIndex[j]==SwapWordBlob.pWordBlobBuffer[k].lIndex)break;
				if (k<SwapWordBlob.lCurNum)continue;

				///some conditions
				///compute mean && std variance value
				for (k=0; k<SwapWordBlob.lCurNum; k++)
				{
					MLong lTmp =pInfoGraph->pInfoArea[SwapWordBlob.pWordBlobBuffer[k].lIndex].rtArea.bottom
						-pInfoGraph->pInfoArea[SwapWordBlob.pWordBlobBuffer[k].lIndex].rtArea.top; 		
					lMean += lTmp;	
				}
				lMean /= SwapWordBlob.lCurNum;
				for (k=0; k<SwapWordBlob.lCurNum; k++)
				{
					MLong lTmp =pInfoGraph->pInfoArea[SwapWordBlob.pWordBlobBuffer[k].lIndex].rtArea.bottom
						-pInfoGraph->pInfoArea[SwapWordBlob.pWordBlobBuffer[k].lIndex].rtArea.top; 
					lVariance += (lTmp-lMean)*(lTmp-lMean);
				}

				if (SwapWordBlob.lCurNum<=1)
					lVariance = -1;
				else
					lVariance = sqrt(lVariance*1.0/(SwapWordBlob.lCurNum-1));
				///1.alike height, && alike relative y position && alike relative distance
				if (OffsetValue(lMean, pAlikeArea->rtArea.bottom-pAlikeArea->rtArea.top, MAX_MEAN_OFFSET)<0)continue;///平均高度偏移太大
				if (lVariance>VALID_STD_VARIANCE_OFFSET(lMean, SwapWordBlob.lCurNum))continue;
				for (k=0; k<SwapWordBlob.lCurNum; k++)
				{
					if (pAlikeArea->pDistanceWithBlobs[SwapWordBlob.pWordBlobBuffer[k].lIndex] < lMinDist)
					{
						lMinDist = pAlikeArea->pDistanceWithBlobs[SwapWordBlob.pWordBlobBuffer[k].lIndex] ;
						lMinDistX = SwapWordBlob.pWordBlobBuffer[k].lIndex;
						
					}
				}
				if(lMinDistX<0) 
					continue;
				///alike color
				pCurArea = pInfoGraph->pInfoArea + lMinDistX;
				crRef = PIXELVAL(pCurArea->lY/pCurArea->lSize, 
					pCurArea->lCb/pCurArea->lSize, pCurArea->lCr/pCurArea->lSize);
				lDist = GetColorDist(pAlikeArea->lY/pAlikeArea->lSize, 
					pAlikeArea->lCb/pAlikeArea->lSize, pAlikeArea->lCr/pAlikeArea->lSize, crRef);
				if (lDist>MAX_COLOR_DIST*7/8)
					continue;
				
				///overlapping rate
				vGetOverLappingRate(pAlikeArea->rtArea, pCurArea->rtArea, 
					MNull, &lMaxRate);
				if (lMaxRate>MAX_RT_OVERLAPPINGRATE)continue;

				///alike relative distance
				lMinDist = (MLong)sqrt((MDouble)lMinDist);
				///pDistnaceWithBlobs里面的数据是平方的
				if (!VALID_RELATIVE_DISTANCE(lMinDist, lMean))continue;

				///keep center distance a fixed value
				ptG1.x = (pAlikeArea->rtArea.right + pAlikeArea->rtArea.left)/2;
				ptG1.y = (pAlikeArea->rtArea.bottom+pAlikeArea->rtArea.top)/2;
				ptG2.x = (pCurArea->rtArea.right + pCurArea->rtArea.left)/2;
				ptG2.y = (pCurArea->rtArea.bottom+pCurArea->rtArea.top)/2; 
				if (vDistance2L(ptG1,ptG2)<lMean/3)continue;

				///y position of alike blob
				///为了后续的继续组合，这里不限制组合的方向
				//DrawCurrentArea(g_pLabelData, g_lLabelLine, g_lWidth, g_lHeight, 
			 	//			pInfoGraph->pInfoArea, lMinDistX, ".\\area_cur.bmp");
				//DrawCurrentArea(g_pLabelData, g_lLabelLine, g_lWidth, g_lHeight, 
	 				//pInfoGraph->pInfoArea, j, ".\\area_alike.bmp");
				if (lMergeStatus == HORIZTONTAL_MERGE)
				{
					if (ABS(pAlikeArea->rtArea.bottom - pCurArea->rtArea.bottom)>lMean*MAX_MEAN_OFFSET/100)
					{
						continue;
					}
				}
				else if (lMergeStatus == VERTICAL_MERGE)
				{
					
					if (ABS((pAlikeArea->rtArea.right+pAlikeArea->rtArea.left)/2
						-(pCurArea->rtArea.right+pCurArea->rtArea.left)/2) 
						>MAX(pAlikeArea->rtArea.right-pAlikeArea->rtArea.left,
							pCurArea->rtArea.right-pCurArea->rtArea.left))continue;
							
				}
				
				///add to the blob list
				if (SwapWordBlob.lCurNum>=SwapWordBlob.lMaxWordBlob)continue;
				SwapWordBlob.pWordBlobBuffer[SwapWordBlob.lCurNum].lIndex= BlobIndex[j];
				SwapWordBlob.lCurNum++;
				bMerge = MTrue;
				if (SwapWordBlob.lCurNum==2)
				{
					MDouble Theta;
					//ptG1.x = (pAlikeArea->rtArea.right + pAlikeArea->rtArea.left)/2;
					//ptG1.y = (pAlikeArea->rtArea.bottom+pAlikeArea->rtArea.top)/2;
					//ptG2.x = (pCurArea->rtArea.right + pCurArea->rtArea.left)/2;
					//ptG2.y = (pCurArea->rtArea.bottom+pCurArea->rtArea.top)/2; 
					Theta = vCalculateTheta(ptG1, ptG2);
					if (Theta<V_PI/4 || Theta >V_PI*3/4) 
					lMergeStatus = HORIZTONTAL_MERGE;
					else
					lMergeStatus = VERTICAL_MERGE;
				}
			}
		}while(bMerge);

		///judge the blob list
		if (SwapWordBlob.lCurNum>=pInfoSkin->lMinWordLength
			&&SwapWordBlob.lCurNum<=pInfoSkin->lMaxWordLength)
		{
			if (pWordList->lFreeNum>SwapWordBlob.lCurNum+1)///remaining word_blob
			{
				///形成一个WordBlob Node
				WORD_BLOB *pNewList = pWordList->pFreeList;
				pWordList->pFreeList = pNewList->pNext;///
				pWordList->lFreeNum--;
				JMemSet(pNewList, 0, sizeof(WORD_BLOB));
				pNewList->type = WORD_NODE;
				
				while(SwapWordBlob.lCurNum>0)
				{
					WORD_BLOB *pCurWordBlob = pWordList->pFreeList;
					pWordList->pFreeList = pCurWordBlob->pNext;
					pWordList->lFreeNum--;
					pCurWordBlob->pNext = MNull;
					JMemCpy(pCurWordBlob, SwapWordBlob.pWordBlobBuffer+SwapWordBlob.lCurNum-1, sizeof(WORD_BLOB));

					pCurWordBlob->pNext = pNewList->pChildList;
					pNewList->pChildList = pCurWordBlob;
					pNewList->lChildNum++;
					SwapWordBlob.lCurNum--;
				}
				///
				pNewList->pNext = pWordList->pWordBlob;
				pWordList->pWordBlob = pNewList;
				pWordList->lBlobNum++;
			}
		}
	}
	
	///merge the pWordList
	//delete the same list 
	
	UpdateWordBlobList(pWordList);
	pCurBlob = pWordList->pWordBlob;
	while(pCurBlob)
	{
		///组合需要rtWord信息，但是不做Extend
		UpdateWordInfo(pInfoSkin, pCurBlob, 0);
		pCurBlob = pCurBlob->pNext;
	}
	///重新组织WordBlob，有时候，可能是多层组织方式，比如两个1组成一个大的1
	///然后再与其他的进行组织
	///只要更新了最小的单元，就需要继续组合，直到没有新的最小单元出现
	///这个组合有问题
	///first only handle two bloblists re-organize
	 GO(WordBlobReorganize(hMemMgr, pInfoSkin, pWordList));
	 GO(CheckWordList(pWordList));
	// GO(WordBlobReorganize(hMemMgr, pInfoSkin, pWordList));
	///update rectangle information
	pCurBlob = pWordList->pWordBlob;
	while (pCurBlob)
	{
		///更新组合后的rtWord信息，并做Extend
		UpdateWordInfo(pInfoSkin, pCurBlob, 10);
		///对blob进行排序，从左到右
		AscendingWordBlob(pCurBlob, PositionCmp);
		pCurBlob = pCurBlob->pNext;
	}

EXT:
	FreeVectMem(hMemMgr, SwapWordBlob.pWordBlobBuffer);
	FreeVectMem(hMemMgr, BlobIndex);
	return res;
}

///Double Checke
MRESULT CheckWordList(WORD_BLOB_LIST *pWordList)
{
	MRESULT res = LI_ERR_NONE;
	WORD_BLOB *pWordBlob,*pChildBlob,*pChildBlob1,*pChildBlob2;
	pWordBlob = pWordList->pWordBlob;
	MLong ChildNum ,ChildNum1,ChildNum2;
	while(pWordBlob)
	{
		ChildNum = 0;
		pChildBlob = pWordBlob->pChildList;
		while(pChildBlob)
		{
			ChildNum++ ;
			if(ChildNum>pWordBlob->lChildNum)
			{
				pChildBlob = MNull;
				continue;
			}
			if(pChildBlob->type == WORD_NODE)
			{
				ChildNum1 = 0;
				pChildBlob1=pChildBlob->pChildList;
				while(pChildBlob1)
				{
					ChildNum1++;
					if(ChildNum1 == pChildBlob->lChildNum && pChildBlob1->pNext)
					{
						pChildBlob1->pNext = MNull;
						continue;
					}
					if(pChildBlob1->type == WORD_NODE)
					{
						ChildNum2 = 0;
						pChildBlob2 = pChildBlob1->pChildList;
						while(pChildBlob2)
						{
							ChildNum2++;
							if(ChildNum2 == pChildBlob1->lChildNum && pChildBlob2->pNext)
							{
								pChildBlob2->pNext = MNull;
								continue;
							}
							pChildBlob2 = pChildBlob2->pNext;
						}
					}
					pChildBlob1 = pChildBlob1->pNext;
				}
			}
			pChildBlob = pChildBlob->pNext;
		}
		pWordBlob = pWordBlob->pNext;
	}
	return res;
}

///在纵向上投影灰度图，取一阶导数，寻找最陡的斜坡
///间距在lEstCharWidth的正负两个斜坡之间是一个可能的candidate

MRESULT CharCrop(MHandle hMemMgr, MByte* pImgData, MLong lDataLine, MLong lWidth, MLong lHeight, 
	MLong lEstCharWidth,RECT_CONF* pRectList, MLong lMaxRectNum, MLong *plRectNum)
{
	///先求Gradient
	MRESULT res = LI_ERR_NONE;

	MByte *pTmp = MNull;
	MLong *plHist = MNull;
	MLong *plHist1D = MNull;
	MLong lExt;
	MLong lCurMaxIdx;
	MLong *pPeakMinValues = MNull;
	MLong *pPeakMaxValues = MNull;
	MLong lPeakMinIdxNum, lPeakMaxIdxNum;
	MDouble dMaxConf = 0;
	if (!pImgData || !pRectList ||!plRectNum)
	{	res = LI_ERR_INVALID_PARAM; goto EXT;}
	
	AllocVectMem(hMemMgr, plHist, lWidth*2, MLong);
	SetVectZero(plHist, lWidth*2*sizeof(MLong));
	plHist1D = plHist + lWidth;

	AllocVectMem(hMemMgr, pPeakMaxValues, lWidth*2, MLong);
	SetVectZero(pPeakMaxValues, lWidth*2*sizeof(MLong));
	pPeakMinValues = pPeakMaxValues + lWidth;

	///统计
	lExt = lDataLine - lWidth;
	pTmp = pImgData;
	for (int j=0; j<lHeight; j++, pTmp+=lExt)
		for (int i=0; i<lWidth; i++, pTmp++)
			plHist[i] += *pTmp;

	///分析统计结果
	GO(SmoothHist(hMemMgr, plHist, lWidth));
	///一阶导数,两边要各少一个数
	for (int i=1; i<lWidth-1; i++)
		plHist1D[i] = (plHist[i+1] - plHist[i-1])/2;
	GO(SmoothHist(hMemMgr, plHist1D+1, lWidth-2));////

	//PrintChannel(plHist1D, lWidth, DATA_I32, lWidth, 1, 1, 0);

	///从右边开始进行倒数，寻找合适的一阶导数局部最小，到一阶导数局部最大的区间
	///区间宽度要符合charWidth
	GO(GetPeaksValue(hMemMgr, plHist1D, lWidth, 
			pPeakMinValues, &lPeakMinIdxNum, pPeakMaxValues, &lPeakMaxIdxNum, lEstCharWidth/4));

	
	///然后分析minPeak, maxPeak匹配区间
	*plRectNum  =0;
	lCurMaxIdx = lPeakMaxIdxNum-1;
	for (int i=lPeakMinIdxNum-1; i>=0; i--)
	{
		for (int j=lCurMaxIdx; j>=0; j--)
		{
			if (pPeakMinValues[i] - pPeakMaxValues[j] < lEstCharWidth*2/5)continue;
			if (pPeakMinValues[i]-pPeakMaxValues[j]>lEstCharWidth*5/4)break;

			///write it to rect list	 
			if (*plRectNum>=lMaxRectNum)break;
			///如果有overlapping，那么就cut 掉落差小的

			pRectList[*plRectNum].rtRange.left = pPeakMaxValues[j];
			pRectList[*plRectNum].rtRange.right = pPeakMinValues[i];
			pRectList[*plRectNum].rtRange.top = 0;
			pRectList[*plRectNum].rtRange.bottom = lHeight;
			pRectList[*plRectNum].dConfi = plHist1D[pPeakMaxValues[j]] - plHist1D[pPeakMinValues[i]];
			if (dMaxConf<pRectList[*plRectNum].dConfi)
				dMaxConf = pRectList[*plRectNum].dConfi;
			if(*plRectNum>=1 && pRectList[*plRectNum].dConfi > pRectList[*plRectNum-1].dConfi &&
				(pRectList[*plRectNum-1].rtRange.right == pRectList[*plRectNum].rtRange.right|| pRectList[*plRectNum-1].rtRange.left == pRectList[*plRectNum].rtRange.left))
			{
				pRectList[*plRectNum-1] = pRectList[*plRectNum];
				lCurMaxIdx = j;
				continue;
			}
			else if(*plRectNum>=1 && pRectList[*plRectNum].dConfi <=pRectList[*plRectNum-1].dConfi &&
				(pRectList[*plRectNum-1].rtRange.right == pRectList[*plRectNum].rtRange.right|| pRectList[*plRectNum-1].rtRange.left == pRectList[*plRectNum].rtRange.left))
			{
				continue;
			}
			(*plRectNum)++;
			lCurMaxIdx = j;
			break;
		}
	}

	///对dConf 归一化
	if (dMaxConf>0)
	for (int i=0; i<*plRectNum; i++)
		pRectList[i].dConfi /= dMaxConf;
EXT:
	FreeVectMem(hMemMgr, plHist);
	FreeVectMem(hMemMgr, pPeakMaxValues);
	return res;
}

MVoid GrayLevelStatistics(MByte *pData, MLong lDataLine, 
						JLabelData *pLabelData, MLong lLabelLine,
						MLong lWidth, MLong lHeight, 
						JINFO_AREA* pInfoArea,MLong lLabelCur, 
						MDWord *pdwGraySum, MDWord *pdwNum)
{
	MDWord dwSum = 0, dwNum = 0;;
	MLong lDataExt = lDataLine - lWidth;
	MLong lMaskExt = lLabelLine - lWidth;
	MLong lLabel0 = GET_VALID_LABEL(pInfoArea, lLabelCur);
	if (lDataLine<lWidth) return;

	if (!pData || !pdwGraySum || !pdwNum || !pInfoArea)return;

	///debug 信息
	//PrintBmpEx(pData, lDataLine, DATA_U8, lWidth, lHeight, 1, "..\\BB.bmp");
	//DrawCurrentArea(pLabelData, lLabelLine, lWidth, lHeight, pInfoArea, lLabelCur, "..\\colorarea.bmp");

	///参考DrawCurrentArea获取Mask的方式
	if (pLabelData)
	{
		for (int i=0; i<lHeight; i++, pData+=lDataExt, pLabelData+=lMaskExt)
		{
			for (int j=0; j<lWidth; j++, pData++, pLabelData++)
			{
				MLong lTmpLabel;
				lTmpLabel = pLabelData[0];
				if(lTmpLabel<=0)
					continue;
				lTmpLabel = GET_VALID_LABEL(pInfoArea, lTmpLabel-1);
				if(lTmpLabel != lLabel0)
					continue;
				else
				{	
					dwSum+= *pData; dwNum++;
				}
			}
		}
	}	
	else
	{
		for (int i=0; i<lHeight; i++, pData+=lDataExt)
		{
			for (int j=0; j<lWidth; j++, pData++)
			{
				dwSum+= *pData;
				dwNum++;
			}
		}
	}

	*pdwGraySum = dwSum;
	*pdwNum = dwNum;
}


///对wordblob中由Mask覆盖的区域，进行灰度统计
///pImgData的起始地址是pWordBlob的(rtWord.left, rtWord.top)
///所以每一个childBlob，要按照该坐标偏移
MRESULT WordBlobGrayStatistics(MHandle hMemMgr, INFO_SKIN *pInfoSkin, 
		WORD_BLOB* pWordBlob, MByte *pImgData, MLong lImgLine, 
		MLong lWidth, MLong lHeight, MLong *pMeanGrayVal, MLong *pMeanMaskGrayVal)
{
	MRESULT res = LI_ERR_NONE;
	MLong lIndexNum = 0, lMemLength=0;
	MLong *pMemIndex = MNull;
	MDWord dwSum, dwNum;
	MDWord dwMaskSum, dwMaskNum;

	if (!pInfoSkin || !pWordBlob ||!pMeanGrayVal ||!pMeanMaskGrayVal)
		{res = LI_ERR_INVALID_PARAM; goto EXT;}
	
	IndexTraversal(pWordBlob, MNull, 0, &lMemLength);

	AllocVectMem(hMemMgr, pMemIndex, lMemLength, MLong);
	SetVectZero(pMemIndex, lMemLength*sizeof(MLong));
	
	IndexTraversal(pWordBlob, pMemIndex, lMemLength, &lIndexNum);

	///
	dwSum = 0;
	dwNum =0;
	dwMaskSum = 0;
	dwMaskNum = 0;
	for (int i=0; i<lIndexNum; i++)
	{
	///每一个blob都要计算进来
		MDWord dwSubSum = 0, dwSubNum = 0;
		MByte *pCurImgData;
		JLabelData *pCurLabelData;
		JINFO_AREA *pCurArea = pInfoSkin->infoGraph.pInfoArea + pMemIndex[i];
		if (pCurArea->rtArea.top<pWordBlob->rtWord.top || pCurArea->rtArea.left<pWordBlob->rtWord.left)
			continue;
		if (pCurArea->rtArea.bottom>pWordBlob->rtWord.bottom || pCurArea->rtArea.right>pWordBlob->rtWord.right)
			continue;
		pCurImgData = pImgData + lImgLine * (pCurArea->rtArea.top - pWordBlob->rtWord.top) + 
			 (pCurArea->rtArea.left - pWordBlob->rtWord.left) ;
		pCurLabelData = pInfoSkin->pLabelData + JMemLength(pInfoSkin->lWidth) * pCurArea->rtArea.top
			+ pCurArea->rtArea.left;
		GrayLevelStatistics(pCurImgData, lImgLine, 
			pCurLabelData, JMemLength(pInfoSkin->lWidth),
			pCurArea->rtArea.right - pCurArea->rtArea.left, 
			pCurArea->rtArea.bottom - pCurArea->rtArea.top, 
			pInfoSkin->infoGraph.pInfoArea, pMemIndex[i],
			&dwSubSum ,&dwSubNum);
		dwMaskSum += dwSubSum;
		dwMaskNum += dwSubNum;
		GrayLevelStatistics(pCurImgData, lImgLine, MNull, 0, 
			pCurArea->rtArea.right - pCurArea->rtArea.left, 
			pCurArea->rtArea.bottom - pCurArea->rtArea.top, 
			pInfoSkin->infoGraph.pInfoArea, pMemIndex[i],
			&dwSubSum, &dwSubNum);
		dwSum += dwSubSum;
		dwNum += dwSubNum;
	}

	if (dwNum>0)
		*pMeanGrayVal = dwSum/dwNum;
	else
		*pMeanGrayVal = 0;
	if (dwMaskNum>0)
		*pMeanMaskGrayVal = dwMaskSum/dwMaskNum;
	else 
		*pMeanMaskGrayVal = 0;
EXT:
	FreeVectMem(hMemMgr, pMemIndex);
	return res;
}
