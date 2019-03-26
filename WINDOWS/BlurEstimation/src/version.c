#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include"HY_IMAGEQUALITY.h"

static const IQ_Version g_ImageQuality_version = 
{
    1, 1, 0, 3, "Image Quality  - 1.0.0.0", "29/07/2014", 
    "Copyright (C) 2014, Whayer Inc."		
};


const IQ_Version * IQ_GetVersion()
{
    return &g_ImageQuality_version;
}