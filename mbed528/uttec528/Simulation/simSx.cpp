#include "mbed.h"
#include "main.h"

#include "simSx.h"

DimmerRf* simSx::pMySim = NULL;

sxRxFrame_t simSx::m_simSxRx = {0,};
sxTxFrame_t simSx::m_simSxTx = {0,};

simSx::simSx(){
}

simSx::simSx(DimmerRf* pRf){
	pMySim = pRf;
}


bool simSx::isSxRxDone(){
	return pMySim->isSxRxDone();
}

void simSx::clearSxRxFlag(){
	pMySim->clearSxRxFlag();
}

sxRxFrame_t* simSx::readLoRa(){
	rfFrame_t* pFrame = pMySim->returnSxRxBuf();
	m_simSxRx.sxRxFlag = pMySim->isSxRxDone();
	m_simSxRx.ptrBuf = (sxFrame_t*)pFrame;
	m_simSxRx.size = sizeof(rfFrame_t);
	m_simSxRx.rssi = 66;
	m_simSxRx.snr = 77;
	return &m_simSxRx;
}

void simSx::sendLoRa(sxTxFrame_t sFrame){
	rfFrame_t* pFrame = (rfFrame_t*)sFrame.ptrBuf;
	pMySim->sendSxRf(pFrame);
}

void simSx::setSxRxFlag(){
	pMySim->setSxRxFlag();
}

void simSx::initSx1276(uint16_t uiGid){
	printf("change gid %d\r\n", uiGid);
}



