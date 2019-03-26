#ifdef PLATFORM_SOFTUNE
#pragma section CONST=RF_LIB_CONST, attr=CONST
#pragma section CODE=RF_LIB_CODE, attr=CODE
#endif
#include "HYAMR_meterReg.h"

static const HYAMR_Version g_ammeterReg_version = 
{
	2, 0, 0, 0, "meter Recognize - 2.0.0.0", "28/12/2015", 
	"Copyright (C) 2015, Whayer Inc."		
};

const HYAMR_Version * HYAMeterReg_GetVersion()
{
	return &g_ammeterReg_version;
}