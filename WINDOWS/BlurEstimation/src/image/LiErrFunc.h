/*!
* \file LiErrFunction.h
* \brief  the function related to error function and gamma function 
* \author hmy@whayer
* \version vision 1.0 
* \date 23 June 2014
*/



#if !defined(_LI_ERRFUNC_H_)
#define _LI_ERRFUNC_H

#include "liblock.h"
#include <stdio.h>

/************************************************************************/
#define  LiGampdf			    PX(LiGampdf)
#define  LiGamcdf				PX(LiGamcdf)
#define  LiGamInv			    PX(LiGamInv)
#define  LiGamma			    PX( LiGamma)
#define  LiGaminc				PX(LiGaminc)
#define  LiLogGam			    PX(LiLogGam)
#define  ErfCinv			        PX(ErfCinv)
#define  Erfinv				    PX(Erfinv)
#define  Erfc		                PX(Erfc)
#define  Erf			                PX(Erf)
#define  LiIntrgral				PX( LiIntrgral)


/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif	
	MRESULT LiGampdf(MFloat *Res,MFloat data1,MFloat shape,MFloat scale);
	MRESULT LiGamcdf(MFloat *Res,MFloat data1,MFloat shape,MFloat scale);
	MRESULT LiGamInv(MFloat *res,MFloat data1,MFloat shape,MFloat scale);
	MRESULT LiGamma(MDouble *result,MDouble data1);
	MRESULT LiGaminc(MFloat *Result,MFloat data1,MFloat data2);
	MRESULT LiLogGam(MFloat *Res,MFloat data1);
	MRESULT ErfCinv(MFloat* datares,MFloat datasrc);
	MRESULT Erfinv(MFloat* datares,MFloat datasrc);
	MRESULT Erfc(MFloat* datares,MFloat datasrc);
	MRESULT Erf(MFloat *datares,MFloat datasrc);
	MRESULT LiIntrgral(MFloat* datares, MLong a,MLong b, MFloat eps,MFloat IntrgraF(MFloat));
#ifdef __cplusplus
}
#endif
#endif // _LIIERRFUNC_H