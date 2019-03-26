#ifndef SNAKE_H
#define SNAKE_H
#include"amcomdef.h"
#include "liblock.h"
#ifdef __cplusplus
extern "C" {
#endif
#define TERMCRIT_ITER    1
#define TERMCRIT_EPS     2
#define MAX_WIDTH        20
#define MAX_HEIGHT       20

	typedef struct SnakeTermCriteria_s
	{
		MLong    type;  /* may be combination of
					  CV_TERMCRIT_ITER
					  CV_TERMCRIT_EPS */
		MLong    max_iter;
		MLong epsilon;
	}SnakeTermCriteria;
	typedef struct SnakeParam_s
	{
		MLong alpha;
		MLong beta;
		MLong gamma;
		MLong lHeight;
		MLong lWidth;
	}SnakeParam;
MVoid SnakeOpenRegion(BLOCK*pImage, MPOINT *pPoint,MVoid *pMem,MLong lNum,SnakeParam param,SnakeTermCriteria criteria);

#ifdef __cplusplus
}
#endif
#endif