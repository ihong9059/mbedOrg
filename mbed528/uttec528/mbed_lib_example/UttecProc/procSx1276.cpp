#include <stdio.h>
#include <string.h>

#include "procSx1276.h"
#include "UttecUtil.h"

static UttecUtil myUtil;

Flash* procSx1276::mpFlash = NULL;
Flash_t* procSx1276::mpFlashFrame = NULL;
rfFrame_t* procSx1276::mp_rfFrame = NULL;
DimmerRf* procSx1276::pMyRf = NULL;
rs485* procSx1276::pMy485 = NULL;

simSx* procSx1276::pMySx1276 = NULL;
//sx1276Exe* procSx1276::pMySx1276 = NULL;
UttecBle* procSx1276::pMyBle = NULL;
mSecExe* procSx1276::pMy_mSec = NULL;
procServer* procSx1276::pMyServer = NULL;

//sxFrame_t procSx1276::m_sxFrame = {0,};
//sxRxFrame_t procSx1276::m_sxRxFrame = {0,};

procSx1276::procSx1276(){
}

procSx1276::procSx1276(uttecLib_t pLib, procServer* pServer){
	mpFlash = pLib.pFlash;
	mpFlashFrame = mpFlash->getFlashFrame();
	mp_rfFrame = &mpFlashFrame->rfFrame;
	
	pMyRf = pLib.pDimmerRf;
	pMy485 = pLib.pRs485;
//	UttecSx1276 = pLib.pSx1276;
	pMyBle = pLib.pBle;
	pMy_mSec = pLib.pMsec;
	pMyServer = pServer;
}

void procSx1276::transferMstGwBy485(rfFrame_t* pFrame, UttecDirection_t dir){
	if(myUtil.isNotMyGwGroup(pFrame, mp_rfFrame)&&myUtil.isGw(mp_rfFrame)) return;
	
	char cCmd[20]; char cSub[20];
	myUtil.dispCmdandSub(cCmd, cSub, pFrame);
	if(dir == eDown){
		printf("server ---> gateway, %s, %s\n\r", cCmd, cSub);
		pMy485->send485(pFrame, eRsDown);
	}
	else{
		printf("gateway  ---> server, %s, %s\n\r", cCmd, cSub);
		pMy485->send485(pFrame, eRsUp);
	}
}

void procSx1276::fromServer2Tx(rfFrame_t* pFrame){
	if(myUtil.isNotMyGwGroup(pFrame, mp_rfFrame)&&myUtil.isGw(mp_rfFrame)) return;
	
	char cCmd[20]; char cSub[20];
	myUtil.dispCmdandSub(cCmd, cSub, pFrame);
	printf("server ---> gateway, %s, %s -> ", cCmd, cSub);
	
	printf("From Gw sendSxFrame -> end\n\r");
	sendSxFrame(pFrame);
}

void procSx1276::dispSx1276(){
	sxRxFrame_t msxRxFrame= *pMySx1276->readLoRa();
	sxFrame_t msxFrame;
	msxFrame = *msxRxFrame.ptrBuf;
	printf("cmd:%d, ",msxFrame.cmd);
	printf("sub:%d, ",msxFrame.sub);
	printf("size:%d, rssi:%d, snr:%d \n\r",
		msxRxFrame.size, msxRxFrame.rssi, msxRxFrame.snr);
}

void procSx1276::setSimulationData(){
	sxRxFrame_t msxRxFrame = *pMySx1276->readLoRa();
	sxFrame_t msxFrame;
	msxFrame.gid = 45;
	msxFrame.pid = 11;
	msxFrame.cmd = edServerReq;
	msxFrame.sub = edsControl;
	*(sxFrame_t*)msxRxFrame.ptrBuf = msxFrame;
	msxRxFrame.rssi = 101;
	msxRxFrame.snr = -10;
	msxRxFrame.size = sizeof(msxFrame);
	msxRxFrame.sxRxFlag = true;
	
	sxRxFrame_t* ptemp = pMySx1276->readLoRa();
	*ptemp = msxRxFrame;
}

void procSx1276::sendSxFrame(rfFrame_t* pFrame){
	sxTxFrame_t sTxFrame;
	sTxFrame.ptrBuf = (sxFrame_t*)pFrame;
	sTxFrame.size = sizeof(sxFrame_t);
	pMySx1276->sendLoRa(sTxFrame);
}

bool procSx1276::isMyGroup(rfFrame_t* pSrc, rfFrame_t* pMy){
	if(pSrc->MyAddr.GroupAddr == pMy->MyAddr.GroupAddr)
		return true;
	else return false;
}

rfFrame_t* procSx1276::readSxFrame(){
	sxRxFrame_t* psxRxFrame = pMySx1276->readLoRa();
	sxFrame_t* psxFrame = psxRxFrame->ptrBuf;
	return (rfFrame_t*)psxFrame;
}

void procSx1276::processCmdNewSet(rfFrame_t* sFrame){
	printf("NewSet\n\r");
	Flash myFlash;
	rfFrame_t* tempFrame=&myFlash.getFlashFrame()->rfFrame;

	tempFrame->MyAddr.PrivateAddr=sFrame->MyAddr.PrivateAddr;
	tempFrame->MyAddr.RxTx = sFrame->MyAddr.RxTx;

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

	tempFrame->Ctr.High=	sFrame->Ctr.High;
	tempFrame->Ctr.Low=	sFrame->Ctr.Low;
	tempFrame->Ctr.SensorRate=	sFrame->Ctr.SensorRate;
	tempFrame->Ctr.DTime=	sFrame->Ctr.DTime;

	tempFrame->MyAddr.GroupAddr=sFrame->Trans.DstGroupAddr;
	myFlash.getFlashFrame()->Channel.channel=
	sFrame->Trans.DstGroupAddr;

	myFlash.writeFlash();
	NVIC_SystemReset();
}

static setProcedure_t setSx1276Proc = eReadySet;

Timeout setSx1276Timeout;

static void atSetTimeout(){
	setSx1276Proc = eTimeoutSet;
	printf("\n\r----------Setting Timeout, please try again\n\r");
}

#include "UttecLed.h"

void procSx1276::setNewFactor(){
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
	
	while(setSx1276Proc == eWaitRfSet){		
		myUtil.setWdtReload();
		if(pMyRf->isRxDone()){		//For Rf Receive
			pMyRf->clearRxFlag();
			rfFrame_t* pFrame = pMyRf->returnRxBuf();
			processCmdNewSet(pFrame);
			printf("\n\r:::::I Received set Command from user\n\r");
		}
	}	
	
	myChannel = reservedCh;
	pMyRf->changeGroup(myChannel);	
	myLed.unforced(eRfLed);
	myLed.unforced(eSensLed);	
}

void procSx1276::resendByRepeater(rfFrame_t* pFrame){
	if(myUtil.isRpt(mp_rfFrame)){	//Repeat Function		
		if(!myUtil.isRpt(pFrame)){	//From Rpt?
			printf("resend by repeater --> ");
			sendSxFrame(pFrame);	
		}			
	}
}

void procSx1276::procVolumeCmd(rfFrame_t* pFrame){
	UttecUtil myUtil;
	printf("procVolumeCmd -->");
		
	pMy_mSec->m_sensorType = eVolume;
	mp_rfFrame->Ctr.Level = pFrame->Ctr.Level;
	pMy_mSec->sDim.target = pFrame->Ctr.Level/100.0;
	
	resendByRepeater(pFrame);
	printf("pwm = %0.3f\n\r", pMy_mSec->sDim.target);
}


void procSx1276::procSensorCmd(rfFrame_t* pFrame){
	printf("procSensorCmd --> ");
	if(myUtil.isMstOrGw(mp_rfFrame)) return;
	if(myUtil.isRx(mp_rfFrame)) return;
	
	pMy_mSec->sDim.dTime = mp_rfFrame->Ctr.DTime*1000;
	pMy_mSec->sDim.target = mp_rfFrame->Ctr.High/100.0;
	
	resendByRepeater(pFrame);
	printf(" DTime: %d, pwm: %0.3f\n\r",
		pMy_mSec->sDim.dTime, pMy_mSec->sDim.target);	
}

void procSx1276::searchSx1276(rfFrame_t* pFrame){
	UttecLed myLed;
	//Next you have to set pwm control for off the led lamp 
	myLed.on(eRfLed);
	myLed.on(eSensLed);
	pMy_mSec->setForcedDim(DeLampOff);
	printf("\n\r----------searchSx1276\n\r");
	printf("Wait Gateway Approval\n\r");
	setSx1276Timeout.attach(atSetTimeout,DeSx1276SetTimeout);
	setSx1276Proc = eWaitGatewayApprove;
}

void procSx1276::procAck(rfFrame_t* pFrame){
}

static void dispErrorRxTx(rfFrame_t* pThis, rfFrame_t* pFrame){
	printf(" ??? This %s: ", myUtil.dispRxTx(pThis));
	printf("from where %s, No Action\n\r",myUtil.dispRxTx(pFrame));
}

void procSx1276::sx1276Task(rfFrame_t* pFrame){
	bool bSetOk = false;	
	uint8_t ucCmd = pFrame->Cmd.Command;
	printf("sx1276Task --> ");
	switch(ucCmd){
		case edDummy:
				break;
		case edSensor:
			procSensorCmd(pFrame);
				break;
		case edRepeat:
			printf("End of edRepeat\n\r");
				break;
		case edLifeEnd:
				break;
		case edNewSet:
				break;
		case edNewSetAck:
				break;
		case edSearch:
			printf("End of edSearch???\n\r");
//			searchSx1276(pFrame);
				break;
		case edBack:
				break;
		case edAsk:
				break;
		case edVolume:
			if(myUtil.isMstOrGw(mp_rfFrame)){
				printf("transferMstGwBy485 -->");
				transferMstGwBy485(pFrame, eDown);
				return;
			}			
			procVolumeCmd(pFrame);
				break;
		case edDayLight:
				break;
		
		case edServerReq:
			if(myUtil.isMstOrGw(mp_rfFrame)){
				if(myUtil.isMst(pFrame)){
					pFrame->MyAddr.RxTx.iRxTx = eGW;
					fromServer2Tx(pFrame);
				}
				else dispErrorRxTx(mp_rfFrame, pFrame);
				return;
			}			
			else if(myUtil.isTx(mp_rfFrame)){
				if(myUtil.isGw(pFrame)){
					printf("Tx: from Mst, send SxFrame to Rx -> ");
					pFrame->MyAddr.RxTx.iRxTx = eTx;
					sendSxFrame(pFrame);
				}
				else dispErrorRxTx(mp_rfFrame, pFrame);
			}
			if(pMyServer->taskServer(pFrame)){
				wait(0.01);
				printf("Now ready for return\n\r");
				sendSxFrame(pFrame);
			}
				break;
			
		case edClientAck:
			printf("Now start edClientAck %d\n\r", 
				pFrame->MyAddr.RxTx.iRxTx);
			if(myUtil.isMstOrGw(mp_rfFrame)){
				printf("Now start edClientAck %d\n\r", 
					pFrame->MyAddr.RxTx.iRxTx);
				if(myUtil.isTx(pFrame)){
					pFrame->MyAddr.RxTx.iRxTx = eGW;
					sendSxFrame(pFrame);
					printf("Gw: from tx, send SxFrame to Mst -> \n\r");
				}
				else dispErrorRxTx(mp_rfFrame, pFrame);
				return;
			}			
			else if(myUtil.isTx(mp_rfFrame)){
				if(!myUtil.isMstOrGw(pFrame)){
					printf("Tx: from Rx, SRx, Rpt, send SxFrame to Mst -> \n\r");
					pFrame->MyAddr.RxTx.iRxTx = eTx;
					sendSxFrame(pFrame);
				}
				else dispErrorRxTx(mp_rfFrame, pFrame);
			}
				break;
			
		default:
			printf("Check Cmd %d\n\r", ucCmd);
			break;
	}
}
