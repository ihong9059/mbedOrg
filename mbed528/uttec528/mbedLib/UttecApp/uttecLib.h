#ifndef _UTTELIB_H_
#define _UTTELIB_H_

#include "Flash.h"
#include "DimmerRf.h"
#include "rs485.h"
#include "UttecBle.h"
#include "mSecExe.h"

//for mbed 
//#include "sx1276Exe.h"

#include "CmdDefine.h"

//C:\sx1276\downloadTest\UttecLoRa2Ble_FinalOk\uttecLib.h
//C:\sx1276\downloadTest\Rf\UttecUtil.h
typedef struct{
	Flash* pFlash;
	DimmerRf* pDimmerRf;
	rs485* pRs485;
//	sx1276Exe* pSx1276;
	UttecBle* pBle;
	mSecExe* pMsec;
} uttecLib_t;

#endif

