
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "test.h"
#include "modBus.h"

Flash* test::mpFlash = NULL;
Flash_t* test::mpFlashFrame = NULL;
rfFrame_t* test::mp_rfFrame = NULL;
DimmerRf* test::pMyRf = NULL;
rs485* test::pMy485 = NULL;
UttecBle* test::pMyBle = NULL;
mSecExe* test::pMy_mSec = NULL;
procServer* test::pMyServer = NULL;
//simSx* test::pMySim = NULL;

test::test(){
}
void test::setTest(uttecLib_t pLib, procServer* pServer){
	mpFlash = pLib.pFlash;
	mpFlashFrame = mpFlash->getFlashFrame();
	mp_rfFrame = &mpFlashFrame->rfFrame;
/*	
	pMyRf = pLib.pDimmerRf;
	pMy485 = pLib.pRs485;
	pMyBle = pLib.pBle;
	pMy_mSec = pLib.pMsec;
	pMyServer = pServer;
	*/
}

void test::setTestRfData(){		
	pMyRf->setRxFlag();
	rfFrame_t* pRf = pMyRf->returnRxBuf();
	*pRf = *mp_rfFrame;
	pRf->MyAddr.RxTx.iRxTx = eTx;
	pRf->Cmd.Command = edServerReq;
	pRf->Cmd.SubCmd = edsControl;
	pRf->Ctr.Level = 22;
	printf("End of setTestRfData\n\r");
}

void test::setTestSxRxData(){
	printf("End of setTestSxRxData\n\r");
}

void test::setTest485Data(){
	printf("End of setTest485Data\n\r");
}

void test::setTestBleData(){
	printf("End of setTestBleData\n\r");
}

static Ticker secTimer;
static Ticker msecTimer;

static bool tick_Sec = false;
static bool tick_mSec = false;

static void tickSec(){
	static uint32_t ulSecCount = 0;
	ulSecCount++;
	tick_Sec = true;
}
static void tickmSec(){
	tick_mSec = true;
}

void test::testTicker(){
	UttecUtil myUtil;
	secTimer.attach(&tickSec, 1);
//	secTimer.attach(&tickmSec, 0.001);
	msecTimer.attach(&tickmSec, 0.001);
	uint32_t ulSecTest = 0;
	uint32_t ul_mSecTest = 0;
	
	while(1){
		myUtil.setWdtReload();
		if(tick_Sec){
			tick_Sec = false;
			printf("ulSecTest: %d\n\r", ulSecTest++);
		}
		if(tick_mSec){
			tick_mSec = false;
			ul_mSecTest++;
			if(!(ul_mSecTest%1000))
				printf("ul_mSecTest: %d\n\r",ul_mSecTest);
		}			
	}
}
static eSensorType_t setSensorType(uint8_t ucSensor){
	eSensorType_t eType;
	switch(ucSensor){
		case 1: eType = eNoSensor; 
		printf("eNoSensor,");
		break;
		case 2: eType = ePir; 
		printf("ePir,");
		break;
		case 3: eType = eDayLight;
		printf("eDayLight,");
		break;
		case 4: eType = eVolume; 
		printf("eVolume,");
		break;
		default: printf("Error Sensor Type:%d\n\r", ucSensor);
		break;
	}
	return eType;
}
static eRole_t setRoleType(uint8_t ucRxTx){
	eRole_t eType;
	switch(ucRxTx){
		case 1: eType = eRx; 
		printf("eRx,");
		break;
		case 2: eType = eTx;
		printf("eTx,");
		break;
		case 3: eType = eSRx;
		printf("eSRx,");
		break;
		case 4: eType = eRpt;
		printf("eRpt,");
		break;
		case 5: eType = eGW;
		printf("eGW,");
		break;
		case 6: eType = eMst;
		printf("eMst,");
		break;
		default: printf("Error RxTx Type:%d\n\r", ucRxTx);
		break;
	}
	return eType;	
}

static uint8_t setCmd(uint8_t ucCmd){
	uint8_t ucResult;
	switch(ucCmd){
		case 1: ucResult = edSensor; 
		printf("edSensor,");
		break;
		case 2: ucResult = edRepeat; 
		printf("edRepeat,");
		break;
		case 3: ucResult = edVolume;
		printf("edVolume,");
		break;
		case 4: ucResult = edDayLight;
		printf("edDayLight,");
		break;
		case 5: ucResult = edServerReq;
		printf("edServerReq,");
		break;
		case 6: ucResult = edClientAck;
		printf("edClientAck,");
		break;
		default: printf("Error Command:%d\n\r", ucCmd);
		break;
	}
	return ucResult;	
}

static uint8_t setSub(uint8_t ucSub){
	uint8_t ucResult;
	switch(ucSub){
		case 0: ucResult = 0; 
		printf("NoSub,");
		break;
		case 1: ucResult = edsControl; 
		printf("edsControl,");
		break;
		case 2: ucResult = edsNewSet;
		printf("edsNewSet,");
		break;
		case 3: ucResult = edsPing; 
		printf("edsPing,");
		break;
		case 4: ucResult = edsPhoto;
		printf("edsPhoto,");
		break;
		case 5: ucResult = edsCmd_485NewSet;
		printf("edsCmd_485NewSet,");
		break;
		case 6: ucResult = edsCmd_Alternative; 
		printf("edsCmd_Alternative,");
		break;
		case 7: ucResult = edsCmd_Status; 
		printf("edsCmd_Status,");
		break;
		default: printf("Error Sub:%d\n\r", ucSub);
		break;
	}
	return ucResult;	
}


void test::setTestReceiveFrameByNum(uint16_t uiNum){
	UttecUtil myUtil;
	uint8_t ucRxTxDst = uiNum%10; uiNum /= 10;
	uint8_t ucSensorDst = uiNum%10; uiNum /= 10;
	uint8_t ucSub = uiNum%10; uiNum /= 10;
	uint8_t ucRxTxSrc = uiNum%10; uiNum /= 10;
	uint8_t ucSensorSrc = uiNum;
/*	
	pMyRf->setRxFlag();
	rfFrame_t* pRf = pMyRf->returnRxBuf();
	*pRf = *mp_rfFrame;
	
	pMy485->set485Done();
	rfFrame_t* pRf = pMy485->return485Buf();
	*pRf = *mp_rfFrame;

	pMySim->setSxRxFlag();
	sxRxFrame_t* pSxRf = pMySim->readLoRa();
	rfFrame_t* pRf = mp_rfFrame;
	
	*(rfFrame_t*)pSxRf->ptrBuf = *pRf;
	
*/
	pMyRf->setRxFlag();
	rfFrame_t* pRf = pMyRf->returnRxBuf();
	rfFrame_t MyRf = *mp_rfFrame;

	*pRf = MyRf;
	
	dst_t* sDst = (dst_t*)&pRf->Trans;

	pRf->MyAddr.SensorType.iSensor = setSensorType(ucSensorSrc);
	pRf->MyAddr.RxTx.iRxTx = setRoleType(ucRxTxSrc);
	pRf->Cmd.Command = edServerReq;
//	pRf->Cmd.Command = edClientAck;
	pRf->Cmd.SubCmd = setSub(ucSub);
	pRf->Ctr.Level = 22;
		
	printf("\n\rEnd of setReceiveFrameByNum\n\r");
	myUtil.dispSec(mp_rfFrame,false);
}

void test::setTestMyAddr(uint32_t uiNum){
	UttecUtil myUtil;
	
	uint8_t ucPid = uiNum%10; uiNum /= 10;
	uint8_t ucGid = uiNum%10; uiNum /= 10;
	uint8_t ucRxTx = uiNum%10; uiNum /= 10;
	uint8_t ucSensor = uiNum%10;
	
	rfFrame_t* pRf = mp_rfFrame;
	
	pRf->MyAddr.GroupAddr	= ucGid;
	pRf->MyAddr.PrivateAddr = ucPid;
	pRf->MyAddr.SensorType.iSensor = setSensorType(ucSensor);
	pRf->MyAddr.RxTx.iRxTx = setRoleType(ucRxTx);
	
	printf("\n\r MyAddr = %d, %d, %d, %d \n\r", 
		ucSensor, ucRxTx, ucGid, ucPid);
	printf("End of setTestMyAddr\n\r");
	myUtil.dispSec(mp_rfFrame,false);
}

// rxtx, cmd, sub, sensor
void test::setTestYourAddr(uint32_t uiNum){
	UttecUtil myUtil;
	
	uint8_t ucPid = uiNum%10; uiNum /= 10;
	uint8_t ucGid = uiNum%10; uiNum /= 10;
	uint8_t ucRxTx = uiNum%10; uiNum /= 10;
	uint8_t ucSensor = uiNum%10;
	
	rfFrame_t* pRf = mp_rfFrame;
	dst_t* sDst = (dst_t*)&pRf->Trans;

	sDst->rxtx = setRoleType(ucRxTx);
	sDst->pid = pRf->MyAddr.PrivateAddr;
	sDst->gid = pRf->MyAddr.GroupAddr;
	
	printf("\n\r YourAddr = %d, %d, %d, %d \n\r", 
		ucSensor, ucRxTx, ucGid, ucPid);
	printf("End of setTestYourAddr\n\r");
	myUtil.dispSec(mp_rfFrame,false);
}

void test::testModAscii(){
	Serial Uart(p9,p11);
	printf("testModAscii\n\r");
	modBus myMod(&Uart);
	
	modRtu_Frame_t rFrame;
	
	rFrame.slave = 2;
	rFrame.function = 0x06;
	rFrame.length = 8;
	
	for(int i = 0; i<rFrame.length; i++)
		rFrame.data[i] = i+2;

	uint8_t* ucpTest = 
		myMod.generateModAscii(&rFrame);
	
	printf("Send Length = %d\n\r",rFrame.lengthForSend);
	for(int i = 0; i<rFrame.lengthForSend; i++)
		putchar(ucpTest[i]);
	for(int i = 0; i<rFrame.lengthForSend; i++){
		myMod.parseModAscii(ucpTest[i]);
	}
	
//	myMod.setModRData();
	myMod.clearAsciiDone();
	myMod.returnModAscii();
	
	printf("Rtu Time out\n\r");	
}

void test::testModRtu(){
	UttecUtil myUtil;
	Serial Uart(p9,p11);
	
	printf("testModRtu\n\r");
	modBus myMod(&Uart);
	modRtu_Frame_t rFrame;
	
	rFrame.slave = 2;
	rFrame.function = 6;
	rFrame.length = 4;
	rFrame.data[0] = 0x10;
	rFrame.data[1] = 0x46;
	rFrame.data[2] = 0;
	rFrame.data[3] = 0x32;
	
	uint8_t* ucpTest = 
		myMod.generateModRtu(&rFrame);
	
	printf("Send Length = %d\n\r",rFrame.lengthForSend);
	for(int i = 0; i<rFrame.lengthForSend; i++){
		myMod.parseModRtu(ucpTest[i]);
		wait(0.1);
	}
	
	while(!myMod.isRtuDone()){
		myUtil.setWdtReload();
	}	
	myMod.clearRtuDone();
	myMod.returnModRtu();
	
	printf("Rtu Time out\n\r");
/*	
	*/
}

void test::testRealRtu(){
	UttecUtil myUtil;
	Serial Uart(p9,p11);
	
	printf("testModRtu\n\r");
	modBus myMod(&Uart);

	printf("....Waiting serial input\n\r");
	
	while(!myMod.isRtuDone()){
		myUtil.setWdtReload();
	}
	
	myMod.clearRtuDone();
	myMod.returnModRtu();
	
	printf("Rtu Time out\n\r");
	
}

void test::testRealAscii(){
	UttecUtil myUtil;
	Serial Uart(p9,p11);
	
	printf("testRealAscii\n\r");
	modBus myMod(&Uart);

	printf("....Waiting serial input\n\r");
	
	while(!myMod.isAsciiDone()){
		myUtil.setWdtReload();
	}
	
	myMod.setModRData();
	myMod.clearAsciiDone();
	myMod.returnModAscii();
	
	printf("Rtu Time out\n\r");
	
}

#include "monitor.h"

void test::testMonitor(){
	monitor myMon;
	myMon.setTestData();
}

#include "schedule.h"

void test::testSchedule(){
	UttecUtil myUtil;
	schedule mySch;
	mySch.initSchedule();
	while(1){
		mySch.procSchedule();
		myUtil.setWdtReload();
	}
}



