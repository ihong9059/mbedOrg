#include <stdio.h>
#include <string.h>

#include "CmdDefine.h"
#include "procRf.h"

#include "UttecUtil.h"
#include "UttecLed.h"

Flash* procRf::mpFlash = NULL;
Flash_t* procRf::mpFlashFrame = NULL;
rfFrame_t* procRf::mp_rfFrame = NULL;
DimmerRf* procRf::pMyRf = NULL;
rs485* procRf::pMy485 = NULL;
UttecBle* procRf::pMyBle = NULL;
mSecExe* procRf::pMy_mSec = NULL;
procServer* procRf::pMyServer = NULL;

static UttecUtil myUtil;

procRf::procRf(uttecLib_t pLib, procServer* pServer){
	mpFlash = pLib.pFlash;
	mpFlashFrame = mpFlash->getFlashFrame();
	mp_rfFrame = &mpFlashFrame->rfFrame;
	
	pMyRf = pLib.pDimmerRf;
	pMy485 = pLib.pRs485;
	pMyBle = pLib.pBle;
	pMy_mSec = pLib.pMsec;
	pMyServer = pServer;
}

procRf::procRf(Flash* pFlash, DimmerRf* pRf, mSecExe* pMsec){
}

void procRf::procRfRepeatCmd(rfFrame_t* pFrame){
	if(myUtil.isMstOrGw(mp_rfFrame)) return;
	if(myUtil.isTx(mp_rfFrame)) conflictTx();
	
	mp_rfFrame->Ctr.High = pFrame->Ctr.High;
	mp_rfFrame->Ctr.Low = pFrame->Ctr.Low;
	mp_rfFrame->Ctr.Level = pFrame->Ctr.Level;
	mp_rfFrame->Ctr.DTime = pFrame->Ctr.DTime;
	mp_rfFrame->Ctr.Type = pFrame->Ctr.Type;
	
	if(myUtil.isRx(mp_rfFrame)){
		if(mp_rfFrame->MyAddr.SensorType.iSensor == eNoSensor){
			pMy_mSec->m_sensorType = 
				(eSensorType_t)pFrame->MyAddr.SensorType.iSensor;
			pMy_mSec->sDim.target =  pFrame->Ctr.Level/100.0;		
			printf("Rx pwm = %f ->",	pMy_mSec->sDim.target);
		}
	}
//	printf("procRepeatCmd\n\r");
}

void procRf::resendByRfRepeater(rfFrame_t* pFrame){
	if(myUtil.isRpt(mp_rfFrame)){	//Repeat Function
		if(myUtil.isTx(pFrame)||myUtil.isMstOrGw(pFrame)){	//From Rpt?
			mp_rfFrame->MyAddr.RxTx.iRxTx = eRpt;
			printf("Rf Rpt ->");
			pMyRf->sendRf(pFrame);	
		}			
	}
}

void procRf::resendSensorByRfRepeater(rfFrame_t* pFrame){
	if(myUtil.isRpt(mp_rfFrame)){	//Repeat Function
		if(!myUtil.isRpt(pFrame)){	//From Rpt?
			mp_rfFrame->MyAddr.RxTx.iRxTx = eRpt;
			printf("Rf Rpt ->");
			pMyRf->sendRf(pFrame);	
		}			
	}
}

void procRf::procRfSensorCmd(rfFrame_t* pFrame){
	if(myUtil.isMstOrGw(mp_rfFrame)){
		printf("No Action: isMstOrGw\n\r");
		return;
	}
	if(myUtil.isRx(mp_rfFrame)){
		if(mp_rfFrame->MyAddr.SensorType.iSensor==eNoSensor){
			pMy_mSec->m_sensorType = 
				(eSensorType_t)pFrame->MyAddr.SensorType.iSensor;
			pMy_mSec->sDim.target =  mp_rfFrame->Ctr.High/100.0;		
			printf("Rx pwm = %f ->",	pMy_mSec->sDim.target);
		}
		else{
//		printf("No Action: isRx\n\r");
			return;
		}
	}
	
	pMy_mSec->sDim.dTime = mp_rfFrame->Ctr.DTime*1000;
	pMy_mSec->sDim.target = mp_rfFrame->Ctr.High/100.0;
	
	resendSensorByRfRepeater(pFrame);
	printf("End of procRfSensorCmd\n\r");
}

void procRf::procRfVolumeCmd(rfFrame_t* pFrame){
	if(myUtil.isMstOrGw(mp_rfFrame)){
		printf("No Action: isMstOrGw\n\r");
		return;
	}
	if(myUtil.isTx(mp_rfFrame)) conflictTx();
	
	pMy_mSec->m_sensorType = eVolume;
	mp_rfFrame->Ctr.Level = pFrame->Ctr.Level;
	pMy_mSec->sDim.target = pFrame->Ctr.Level/100.0;
	
	resendByRfRepeater(pFrame);
	printf("procRfVolumeCmd: %0.3f\n\r", pMy_mSec->sDim.target);
}

void procRf::procRfDaylightCmd(rfFrame_t* pFrame){
	if(myUtil.isMstOrGw(mp_rfFrame)){
		printf("No Action: isMstOrGw\n\r");
		return;
	}
	if(myUtil.isTx(mp_rfFrame)) conflictTx();
	
	pMy_mSec->m_sensorType = eVolume;
	mp_rfFrame->Ctr.Level = pFrame->Ctr.Level;
	pMy_mSec->sDim.target = pFrame->Ctr.Level/100.0;
	
	resendByRfRepeater(pFrame);
	printf("procRfDaylightCmd: %0.3f\n\r", pMy_mSec->sDim.target);
}

void procRf::processCmdNewSet(rfFrame_t* sFrame){
	printf("NewSet\n\r");
	Flash myFlash;
	rfFrame_t* tempFrame=&myFlash.getFlashFrame()->rfFrame;

	tempFrame->MyAddr.PrivateAddr=sFrame->MyAddr.PrivateAddr;
	tempFrame->MyAddr.RxTx=sFrame->MyAddr.RxTx;
	
	switch(tempFrame->MyAddr.SensorType.iSensor){
		case eNoSensor:
			if(sFrame->MyAddr.RxTx.iRxTx == eSRx)
				tempFrame->MyAddr.RxTx.iRxTx = eRx;
			break;
		case ePir:
			switch(sFrame->MyAddr.RxTx.iRxTx){
				case eGW: case eMst:
					myUtil.alertFaultSet(1);
					break;
			}
			break;	
		case eDayLight:
			switch(sFrame->MyAddr.RxTx.iRxTx){
				case eMx: case eGW: case eMst:
					myUtil.alertFaultSet(2);
					break;
			}
			break;
		case eVolume:
			switch(sFrame->MyAddr.RxTx.iRxTx){
				case eSRx: case eMx: case eGW: case eMst:
					myUtil.alertFaultSet(3);
					break;
			}
			break;	
		default:
			break;	
	}
	
	tempFrame->Ctr.High =	sFrame->Ctr.High;
	tempFrame->Ctr.Low =	sFrame->Ctr.Low;
	tempFrame->Ctr.SensorRate =	sFrame->Ctr.SensorRate;
	tempFrame->Ctr.DTime=	sFrame->Ctr.DTime;
	printf("high, low = %d, %d\r\n", tempFrame->Ctr.High, tempFrame->Ctr.Low);
	myUtil.setWdt(5);
	tempFrame->MyAddr.GroupAddr=sFrame->Trans.DstGroupAddr;
	myFlash.getFlashFrame()->Channel.channel=
	sFrame->Trans.DstGroupAddr;

	myFlash.writeFlash();
	
//	while(1);
	NVIC_SystemReset();
}

static setProcedure_t setRfProc = eReadySet;

Timeout setRfTimeout;

static void atSetTimeout(){
	setRfProc = eTimeoutSet;
	printf("\n\r----------Setting Timeout, please try again\n\r");
}

#include "UttecLed.h"

void procRf::setNewFactor(){
	UttecLed myLed;
	channel_t myChannel, reservedCh;
	Flash myFlash;

	myChannel=myFlash.getFlashFrame()->Channel;
	rfFrame_t frame = myFlash.getFlashFrame()->rfFrame;
	
	reservedCh = myChannel;
	myChannel.channel=DeSetChannelOld;
	myChannel.SetupMode=true;
	pMyRf->changeGroup(myChannel);
	
	frame.Cmd.Command = edDummy;
	frame.Cmd.SubCmd = edsStart;
	
	pMyRf->sendRf(&frame); //dummy tx for rx enable, very important
	
	printf("\n\r:::::Wait Final Rf Setting from user, about 10 Seconds\n\r");
	setRfProc = eWaitRfSet;
	uint16_t uiCount=0;
	while(setRfProc == eWaitRfSet){		
		myUtil.setWdtReload();
		if(pMyRf->isRxDone()){		//For Rf Receive
			pMyRf->clearRxFlag();
			rfFrame_t* pFrame = pMyRf->returnRxBuf();
			if(pFrame->Cmd.Command == edNewSet){
				printf("\n\r:::::I Received set Command from user\n\r");
				processCmdNewSet(pFrame);
			}
			if(pFrame->Cmd.Command == edBack){
				printf("\n\r:::::I Received back Command from user\n\r");
				setRfProc = eReadySet;
			}
			else printf("Wrong Command, Please check\n\r");
		}
		wait(0.001);
		if(!(uiCount%1000))
			printf("Wait set command: %d\n\r", uiCount/1000);
		uiCount++;
	}	
	
	myChannel = reservedCh;
	pMyRf->changeGroup(myChannel);	
	pMy_mSec->setUnforcedDim();
	myLed.unforced(eRfLed);
	myLed.unforced(eSensLed);	
}

void procRf::searchRf(rfFrame_t* pFrame){
	UttecLed myLed;
	//Next you have to set pwm control for off the led lamp 
	myLed.on(eRfLed);
	myLed.on(eSensLed);
	pMy_mSec->setDirectDim(1);
	printf("\n\r----------searchSx1276\n\r");
	setRfTimeout.attach(atSetTimeout,DeRfSetTimeout);
	setRfProc = eWaitGatewayApprove;
	
	setNewFactor();
}

void procRf::transferMstGwByRf(rfFrame_t* pFrame){
	if(myUtil.isNotMyGwGroup(pFrame, mp_rfFrame)&&myUtil.isGw(mp_rfFrame)) return;
	printf("From Mst or Gw -> ");
	pMyRf->sendRf(pFrame);
	printf("End of transferMstGwByRf\n\r");
}

static void dispErrorRxTx(rfFrame_t* pThis, rfFrame_t* pFrame){
	printf(" ??? This %s: ", myUtil.dispRxTx(pThis));
	printf("from where %s, No Action\n\r",myUtil.dispRxTx(pFrame));
}

void procRf::returnToServer(rfFrame_t* pFrame){
	if(myUtil.isMstOrGw(mp_rfFrame)){
		printf("Gw: from tx, send 485Frame to Mst -> end\n\r");
		pMy485->send485(pFrame, eRsUp);
	}			
	else if(myUtil.isTx(mp_rfFrame)){
//		pMyRf->sendRf(pFrame);		// check whether to change channel
		printf("Tx: from Tx, SRx Rx, send 485Frame to Mst -> end\n\r");
		pMy485->send485(pFrame, eRsUp);
	}
	else{
		pMyRf->sendRf(pFrame);		// check whether to change channel
//		pMy485->send485(pFrame, eRsUp);
		printf("Tx: from SRx Rx, send rfFrame to Mst -> end\n\r");
	}
}


void procRf::taskRf(rfFrame_t* pFrame){
	UttecLed myLed;
	uint8_t ucCmd = pFrame->Cmd.Command;
//	printf("From taskRf:%d -> ", ucCmd);
	switch(ucCmd){
		case edDummy:
			printf("End of edDummy\n\r");
				break;
		case edSensor:
			procRfSensorCmd(pFrame);
				break;
		case edRepeat:
			procRfRepeatCmd(pFrame);
				break;
		case edLifeEnd:
				break;
		case edNewSet:
				break;
		case edNewSetAck:
				break;
		case edSearch:
			searchRf(pFrame);
				break;
		case edBack:
				break;
		case edAsk:
				break;
		case edVolume:
			procRfVolumeCmd(pFrame);
				break;
		case edDayLight:
			procRfDaylightCmd(pFrame);
				break;
		case edServerReq:
			resendByRfRepeater(pFrame);
			if(myUtil.isMyAddr(pFrame, mp_rfFrame)){	// if all address are same
				if(pMyServer->taskServer(pFrame)){			// execute taskServer
					wait(0.01);														// if command is status
					printf("Now ready for return\n\r");		// return process
					returnToServer(pFrame);
//					pMy485->send485(pFrame, eRsUp);
				}
				return;
			}
			if(myUtil.isMstOrGw(mp_rfFrame)){
				pMyRf->changeGroup(pFrame->MyAddr.GroupAddr);
				pMyRf->sendRf(pFrame);		// check whether to change channel
				pMy485->send485(pFrame, eRsDown);
				printf("From mst or gw send 485Frame to Tx -> end \n\r");
//				pMyRf->changeGroup(mp_rfFrame->MyAddr.GroupAddr);
				return;
			}			
			else if(myUtil.isTx(mp_rfFrame)){
				printf("Tx: from Mst, send SxFrame to Rx -> ");
//				pMyRf->sendRf(pFrame);		// check whether to change channel
				pMy485->send485(pFrame, eRsDown);
			}
				break;
		case edClientAck:
			returnToServer(pFrame);
		/*
			if(myUtil.isMstOrGw(mp_rfFrame)){
				pMy485->send485(pFrame, eRsUp);
				printf("Gw: from tx, send 485Frame to Mst -> end\n\r");
			}			
			else if(myUtil.isTx(mp_rfFrame)){
				pMyRf->sendRf(pFrame);		// check whether to change channel
				pMy485->send485(pFrame, eRsUp);
				printf("Tx: from SRx Rx, send 485Frame to Mst -> end\n\r");
			}
		*/
				break;
		case edClientReq:
				break;
		default:
			printf("Rf Check Cmd %d\n\r", ucCmd);
			break;
	}
}

void procRf::conflictTx(){
	UttecLed myLed;
	while(1){
		myUtil.setWdtReload();
		printf("Duplicate Tx in this Group\n\r");
		wait(0.5);
		myLed.on(eRfLed);
		myLed.on(eSensLed);
		wait(0.5);
		myLed.off(eRfLed);
		myLed.off(eSensLed);
	}
}



