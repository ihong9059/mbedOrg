#include <stdio.h>
#include <string.h>

#include "procBle.h"
#include "UttecUtil.h"

DimmerRf* procBle::m_pRf = NULL;

procBle::procBle(uttecLib_t pLib, procServer* pServer){
}

void procBle::bleTask(uint8_t* pFrame){
	printf("Convert from uint8_t* to rfFrame_t*\n\r");
}
