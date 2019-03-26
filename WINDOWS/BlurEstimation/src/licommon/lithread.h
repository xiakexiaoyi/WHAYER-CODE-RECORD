#if !defined(_LI_THREAD_H_)
#define _LI_THREAD_H_

#include "licomdef.h"

typedef MVoid (*fnThreadWork)(MVoid * pData);

#ifdef __cplusplus
extern "C" {
#endif	

/***********************************************************/
#define DoWork_Parallel		PX(DoWork_Parallel)
/***********************************************************/

MVoid DoWork_Parallel(fnThreadWork fnWork, MVoid **ppData, MLong lWorkNum);

#ifdef __cplusplus
}
#endif

#endif // !defined(_LI_THREAD_H_)
