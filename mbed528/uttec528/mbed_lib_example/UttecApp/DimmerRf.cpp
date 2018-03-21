#include <stdio.h>
#include <string.h>

#include "DimmerRf.h"

Flash* DimmerRf::mpFlash=NULL;
Flash_t* DimmerRf::mpFlashFrame=NULL;
rfFrame_t* DimmerRf::mp_rfFrame=NULL;

DimmerRf::DimmerRf(){
}

DimmerRf::DimmerRf(Flash* pFlash){
	mpFlash=pFlash;
	mpFlashFrame=mpFlash->getFlashFrame();
	mp_rfFrame=&mpFlashFrame->rfFrame;
	dispRfFactor(mp_rfFrame);
}
	
void DimmerRf::test(){
	initRfFrame();
	printf("Gid=%d\n\r",mp_rfFrame->MyAddr.GroupAddr);
	printf("Pid=%d\n\r",mp_rfFrame->MyAddr.PrivateAddr);
	
	printf("sizeof(rfFrame)=%d\n\r",sizeof(rfFrame_t));
	printf("sizeof(UttecFlash_t)=%d\n\r",sizeof(Flash_t));
	printf("sizeof(DaliFalsh_t)=%d\n\r",sizeof(DaliFlash_t));
	printf("sizeof(channel_t)=%d\n\r",sizeof(channel_t));
}

void DimmerRf::initRfFrame(){
//	mpFlash->initOrgFlash();		
	channel_t* pCh=&mpFlashFrame->Channel;
	initRf(pCh);
}

void DimmerRf::dispRfFactor(rfFrame_t* frame){
	
	printf("\n\r------------- New Frame Factor --------------\n\r");
	printf("Gid=%d\n\r",frame->MyAddr.GroupAddr);
	printf("Pid=%d\n\r",frame->MyAddr.PrivateAddr);
	printf("RxTx=%d\n\r",frame->MyAddr.RxTx.iRxTx);
	printf("Sensor=%d\n\r",frame->MyAddr.SensorType.iSensor);
	
	printf("High=%d\n\r",frame->Ctr.High);
	printf("Low=%d\n\r",frame->Ctr.Low);
	printf("Level=%d\n\r",frame->Ctr.Level);
	printf("Type=%d\n\r",frame->Ctr.Type);
	printf("Rate=%d\n\r",frame->Ctr.SensorRate);
	printf("Status=%d\n\r",frame->Ctr.Status.iData);
	printf("DTime=%d\n\r",frame->Ctr.DTime);
}


