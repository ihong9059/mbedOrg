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
bool procServer::procControlSub(rfFrame_t* pFrame){
	bool bResult = false;
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		if(myUtil.isGw(mp_rfFrame))	
			pMy_mSec->setDirectDim(pFrame->Ctr.Level/(float)100.0);
		else
			pMy_mSec->setForcedDim(pFrame->Ctr.Level/(float)100.0);
#ifdef testModePrint		
		printf("procControlSub: Level = %d\n\r",pFrame->Ctr.Level);
#endif
	}
	if(myUtil.isMstOrGw(mp_rfFrame)){
#ifdef testModePrint		
		printf("Return Master or Gateway to Server procControlSub Ack\n\r");
#endif
		setAckFrame(pFrame);
		pFrame->Cmd.SubCmd = edsControl;
		bResult = true;
	}	
	return bResult;
}
bool procServer::procNewSetSub(rfFrame_t* pFrame){
	bool bResult = false;
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		pMy_mSec->sDim.forced = false;
#ifdef testModePrint		
		printf("procNewSetSub: Level = %d\n\r",pFrame->Ctr.Level);
#endif
	}
	if(myUtil.isMstOrGw(mp_rfFrame)){
#ifdef testModePrint		
		printf("Return Master or Gateway to Server procNewSetSub Ack\n\r");
#endif
		setAckFrame(pFrame);
		pFrame->Cmd.SubCmd = edsNewSet;
		bResult = true;
	}
	return bResult;
}

bool procServer::procNewFactSetSub(rfFrame_t* pFrame){
	bool bResult = false;
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		Role_t* pNewRole = (Role_t*)&pFrame->Ctr;		
		Flash myFlash;
		mp_rfFrame->MyAddr.PrivateAddr = pNewRole->pid;		
		switch(pNewRole->rxtx){
			case eRx:
			case eTx:
			case eSRx:
			case eGW:
				mp_rfFrame->MyAddr.RxTx.iRxTx = pNewRole->rxtx;
			break;
			
			default:
#ifdef testModePrint		
				printf("Not change the role\r\n");
#endif
			break;	
		}
		mp_rfFrame->MyAddr.GroupAddr = pNewRole->gid;
		
		mp_rfFrame->Ctr.High = pFrame->Ctr.High;
		mp_rfFrame->Ctr.Low = pFrame->Ctr.Low;
		myFlash.writeFlash();
		wait(0.1);
#ifdef testModePrint		
		printf("Change Group: procNewFactSetSub: Level = %d\n\r",pFrame->Ctr.Level);
#endif
	}
	if(myUtil.isMstOrGw(mp_rfFrame)){
#ifdef testModePrint		
		printf("Return Master or Gateway to Server procNewFactSetSub Ack\n\r");
#endif
		setAckFrame(pFrame);
		pFrame->Cmd.SubCmd = edsCmd_485NewSet;
		bResult = true;
	}
	return bResult;
}

bool procServer::procAltSub(rfFrame_t* pFrame){
	bool bResult = false;
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		Flash myFlash;
		if(mp_rfFrame->MyAddr.PrivateAddr%2 ==
			pFrame->MyAddr.PrivateAddr%2){
			pMy_mSec->setForcedDim(pFrame->Ctr.Level/(float)100.0);				
#ifdef testModePrint		
			printf("On, Alternative: Level = %d\n\r",
				pFrame->Ctr.Level);
#endif
		}
		else{	
#ifdef testModePrint		
			printf("Off, Alternative: Level = %d\n\r",
				mp_rfFrame->Ctr.Level);
#endif
		}
	}
	if(myUtil.isMstOrGw(mp_rfFrame)){
#ifdef testModePrint		
		printf("Return Master or Gateway to Server procAltSub Ack\n\r");
#endif
		setAckFrame(pFrame);
		pFrame->Cmd.SubCmd = edsCmd_Alternative;
		bResult = true;
	}
	return bResult;
}

void procServer::setAckFrame(rfFrame_t* pFrame){
	*pFrame = *mp_rfFrame;
	ping_t* pDst = (ping_t*)&pFrame->Trans;
	pDst->rxtx = eMst;
	pDst->pid = 0;
	pDst->gid = 0;
	pFrame->Cmd.Command = edClientAck;
}

#include "monitor.h"

bool procServer::procStatus(rfFrame_t* pFrame){
	monitor myMon;
	bool bResult = false;
	
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		setAckFrame(pFrame);
		pFrame->Cmd.SubCmd = edsCmd_Status;
		
		Monitor_t* pMonitor = (Monitor_t*)&pFrame->Ctr;
		pMonitor->traffic = myMon.getTraffic();
		pMonitor->monitor = myMon.getMonitorResult();
		pMonitor->photo = myMon.getCurrentPhoto();
		
#ifdef testModePrint		
		printf("traffic:%d, monitor:%d, photo:%d\r\n", 
			pMonitor->traffic, pMonitor->monitor, pMonitor->photo);
#endif

		bResult = true;
	}
	return bResult;
}

bool procServer::procPowerRead(rfFrame_t* pFrame){
	monitor myMon;
	bool bResult = false;
	
	if(myUtil.isMyAddr(pFrame,mp_rfFrame)){
		setAckFrame(pFrame);
		pFrame->Cmd.SubCmd = edsPowerRead;
		
		Ctr_t pCtr = pFrame->Ctr;
		Power_t* pPower = (Power_t*)&pCtr;
		
		pPower->Power = myMon.getPower();
#ifdef testModePrint		
		printf("Return Power Value:%d\n\r", pPower->Power);
#endif
		pFrame->Ctr = *(Ctr_t*)pPower;
		
		bResult = true;
	}
	return bResult;
}
bool checkErrorPid(rfFrame_t* pFrame){
	uint8_t ucDstPid = pFrame->Trans.SrcPrivateAddr;
	if(!ucDstPid){
		printf("**********Error pid and command\r\n");
		return true;
	}
	else return false;
}

bool procServer::taskServer(rfFrame_t* pFrame){
	UttecLed myLed;
	bool bResult = false;
	uint8_t ucCmd = pFrame->Cmd.SubCmd;
	monitor myMon;
	switch(ucCmd){
		case edsPowerRead:
			if(checkErrorPid(pFrame)) return false;
			bResult = procPowerRead(pFrame);
				break;
		case edsControl:
			bResult = procControlSub(pFrame);
				break;
		case edsNewSet:
			bResult = procNewSetSub(pFrame);
				break;
		case edsCmd_485NewSet:
			if(checkErrorPid(pFrame)) return false;
			bResult = procNewFactSetSub(pFrame);
				break;
		case edsCmd_Alternative:
			bResult = procAltSub(pFrame);
				break;
		
		case edsMonitor:
			if(checkErrorPid(pFrame)) return false;
			printf("-------- Monitoring\r\n");
			myMon.isLampOk();
			bResult = false;
				break;
		
		case edsCmd_Status:
			if(checkErrorPid(pFrame)) return false;
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
		default:
			printf("Sx Check Cmd %d\n\r", ucCmd);
			break;
	}
}

