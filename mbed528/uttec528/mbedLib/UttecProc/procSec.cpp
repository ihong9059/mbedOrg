#include <stdio.h>
#include <string.h>

#include "procSec.h"
#include "UttecUtil.h"

Flash* procSec::mpFlash = NULL;
Flash_t* procSec::mpFlashFrame = NULL;
rfFrame_t* procSec::mp_rfFrame = NULL;
DimmerRf* procSec::pMyRf = NULL;
rs485* procSec::pMy485 = NULL;
UttecBle* procSec::pMyBle = NULL;
mSecExe* procSec::pMy_mSec = NULL;
procServer* procSec::pMyServer = NULL;

productType_t procSec::m_product = {0,};

void procSec::setProductType(){
	m_product.rcu = true;
	m_product.rf = true;
	m_product.ble	= false;
	m_product.rs485 = true;
	m_product.sx1276 = false;
}

procSec::procSec(uttecLib_t pLib, procServer* pServer){
	mpFlash = pLib.pFlash;
	mpFlashFrame = mpFlash->getFlashFrame();
	mp_rfFrame = &mpFlashFrame->rfFrame;
	
	pMyRf = pLib.pDimmerRf;
	pMy485 = pLib.pRs485;
	pMyBle = pLib.pBle;
	pMy_mSec = pLib.pMsec;
	pMyServer = pServer;
	setProductType();
}
//eRpt, eSRx, eTx, eRx, eMst, eGw
#define testCount 7

void procSec::testFrame(rfFrame_t* pFrame){
	static bool bSender = true;
	static bool bToggle = false;
	static uint32_t tgCount = 0;
	tgCount++;
	if(!(tgCount%testCount)) bToggle = !bToggle;
	if(bSender){
		pFrame->MyAddr.RxTx.iRxTx = eMst;
		pFrame->MyAddr.SensorType.iSensor = ePir;	
		pFrame->MyAddr.PrivateAddr = 	11; //org 10
		pFrame->Cmd.Command = edVolume;
		if(bToggle)
			pFrame->Cmd.SubCmd = edsControl;
		else
			pFrame->Cmd.SubCmd = edsCmd_Alternative;
			
		pFrame->Ctr.High = 100;
		pFrame->Ctr.Low = 0;
		pFrame->Ctr.Level = 50;
	}
	else{
		pFrame->MyAddr.SensorType.iSensor = ePir;	
		pFrame->MyAddr.RxTx.iRxTx = eRpt;
	}	
}

void procSec::proc1Sec(){
	rfFrame_t* pRf = mp_rfFrame;
	if(pRf->MyAddr.RxTx.Bit.Tx){
//		printf("Send Repeat Cmd form Tx\n\r");
		pRf->Cmd.Command = edRepeat;
		pMyRf->sendRf(pRf);		
	}
}

#include "UttecLed.h"
static Serial myUart(p9, p11);
void procSec::secTask(rfFrame_t* pFrame){
	UttecUtil myUtil;	
//	testFrame(pFrame);
	if(myUtil.isFactoryOutMode()){
//		pMy_mSec->setForcedDim(DeLampTtest);
		pMy_mSec->setForcedDim(pFrame->Ctr.High/100.0);
	}
//	printf("Freq = %d\r\n", NRF_RADIO->FREQUENCY);
	proc1Sec();	
	if(!myUtil.isMstOrGw(mp_rfFrame))
		myUtil.dispSec(pFrame, true);
	else{
		UttecLed myLed;
		static uint32_t ulCount = 0;	
		if(!(ulCount++%40)) printf("\r\n");
		else myUart.putc('0'+ulCount%10);//("*");
//		myLed.blink(eRfLed, eRfBlink);
		myLed.blink(eSensLed, eRfBlink);
	}
}
