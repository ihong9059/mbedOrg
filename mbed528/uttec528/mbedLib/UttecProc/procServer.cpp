#include "procServer.h"

#include "CmdDefine.h"
#include "procRf.h"

#include "UttecUtil.h"
#include "UttecLed.h"

static UttecUtil myUtil;

Flash* procServer::mpFlash = NULL;
Flash_t* procServer::mpFlashFrame = NULL;
rfFrame_t* procServer::mp_rfFrame = NULL;
DimmerRf* procServer::pMyRf = NULL;
rs485* procServer::pMy485 = NULL;
UttecBle* procServer::pMyBle = NULL;
mSecExe* procServer::pMy_mSec = NULL;

procServer::procServer(uttecLib_t pLib){
	mpFlash = pLib.pFlash;
	mpFlashFrame = mpFlash->getFlashFrame();
	mp_rfFrame = &mpFlashFrame->rfFrame;
	
	pMyRf = pLib.pDimmerRf;
	pMy485 = pLib.pRs485;
	pMyBle = pLib.pBle;
	pMy_mSec = pLib.pMsec;
}
void procServer::procControlSub(rfFrame_t* pFrame){
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		if(myUtil.isGw(mp_rfFrame))	
			pMy_mSec->setDirectDim(pFrame->Ctr.Level/(float)100.0);
		else
			pMy_mSec->setForcedDim(pFrame->Ctr.Level/(float)100.0);
		printf("procControlSub: Level = %d\n\r",pFrame->Ctr.Level);
	}
}
void procServer::procNewSetSub(rfFrame_t* pFrame){
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		pMy_mSec->sDim.forced = false;
		printf("procNewSetSub: Level = %d\n\r",pFrame->Ctr.Level);
	}
}

void procServer::procNewFactSetSub(rfFrame_t* pFrame){
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		Flash myFlash;
		mp_rfFrame->Ctr = pFrame->Ctr;
		myFlash.writeFlash();
		printf("procNewFactSetSub: Level = %d\n\r",pFrame->Ctr.Level);
	}
}

void procServer::procAltSub(rfFrame_t* pFrame){
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		Flash myFlash;
		if(mp_rfFrame->MyAddr.PrivateAddr%2 ==
			pFrame->MyAddr.PrivateAddr%2){
			pMy_mSec->setForcedDim(pFrame->Ctr.Level/(float)100.0);				
			printf("Matching, procAltSub: Level = %d\n\r",
				pFrame->Ctr.Level);
		}
		else	
			printf("Not Matching, procAltSub: Level = %d\n\r",
				mp_rfFrame->Ctr.Level);
	}
}

void procServer::setAckFrame(rfFrame_t* pFrame){
	*pFrame = *mp_rfFrame;
	ping_t* pDst = (ping_t*)&pFrame->Trans;
	pDst->rxtx = eMst;
	pDst->pid = 0;
	pDst->gid = 0;
	pFrame->Cmd.Command = edClientAck;
}

bool procServer::procStatus(rfFrame_t* pFrame){
	bool bResult = false;
	static uint16_t uiTime = 0; 
	uiTime++;
	
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		printf("Return Status tbd\n\r");
		printf("procStatus: Level = %d\n\r",mp_rfFrame->Ctr.Level);
		setAckFrame(pFrame);
		pFrame->Cmd.Time = uiTime;
		pFrame->Ctr.DTime = uiTime*7;
		pFrame->Cmd.SubCmd = edsCmd_Status;
		printf("RxTx = %d\n\r", pFrame->MyAddr.RxTx.iRxTx);
		bResult = true;
	}
	return bResult;
}

bool procServer::taskServer(rfFrame_t* pFrame){
	UttecLed myLed;
	bool bResult = false;
	uint8_t ucCmd = pFrame->Cmd.SubCmd;
	switch(ucCmd){
		case edsPowerReset:	//100
				break;
		case edsPowerRead:
				break;
		case edsMonitor:
				break;
		case edsControl:
			procControlSub(pFrame);
				break;
		case edsNewSet:
			procNewSetSub(pFrame);
				break;
		case edsColor:
				break;
		case edsCmd_485NewSet:
			procNewFactSetSub(pFrame);
				break;
		case edsCmd_Alternative:
			procAltSub(pFrame);
				break;
		case edsCmd_Status:
			bResult = procStatus(pFrame);
				break;
		default:
			printf("Sx Check Cmd %d\n\r", ucCmd);
			break;
	}
	return bResult;
}

void procServer::taskClient(rfFrame_t* pFrame){
	UttecLed myLed;
	
	uint8_t ucCmd = pFrame->Cmd.Command;
	switch(ucCmd){
		case edsPowerReset:	//100
				break;
		case edsPowerRead:
				break;
		case edsMonitor:
				break;
		case edsControl:
				break;
		case edsNewSet:
				break;
		case edsColor:
				break;
		case edsCmd_485NewSet:
				break;
		case edsCmd_Alternative:
				break;
		default:
			printf("Sx Check Cmd %d\n\r", ucCmd);
			break;
	}
}

