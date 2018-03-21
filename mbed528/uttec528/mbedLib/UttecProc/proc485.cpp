#include <stdio.h>
#include <string.h>

#include "proc485.h"
#include "UttecUtil.h"

static UttecUtil myUtil;

Flash* proc485::mpFlash = NULL;
Flash_t* proc485::mpFlashFrame = NULL;
rfFrame_t* proc485::mp_rfFrame = NULL;
DimmerRf* proc485::pMyRf = NULL;
rs485* proc485::pMy485 = NULL;
UttecBle* proc485::pMyBle = NULL;
mSecExe* proc485::pMy_mSec = NULL;
procServer* proc485::pMyServer = NULL;

proc485::proc485(uttecLib_t pLib, procServer* pServer){
	mpFlash = pLib.pFlash;
	mpFlashFrame = mpFlash->getFlashFrame();
	mp_rfFrame = &mpFlashFrame->rfFrame;
	
	pMyRf = pLib.pDimmerRf;
	pMy485 = pLib.pRs485;
	pMyBle = pLib.pBle;
	pMy_mSec = pLib.pMsec;
	pMyServer = pServer;
}



void proc485::procVolumeCmd(rfFrame_t* pFrame){
	pMy_mSec->m_sensorType = eVolume;
	mp_rfFrame->Ctr.Level = pFrame->Ctr.Level;
	pMy_mSec->sDim.target = pFrame->Ctr.Level/100.0;
	printf("End of procVolumeCmdL %0.3f\n\r",
		pMy_mSec->sDim.target);
}

static void dispErrorRxTx(rfFrame_t* pThis, rfFrame_t* pFrame){
	printf("dispErrorRxTx !!! This %s: ", myUtil.dispRxTx(pThis));
	printf("from where %s, No Action\n\r",myUtil.dispRxTx(pFrame));
}

void proc485::rs485Task(rfFrame_t* pFrame){
	static uint32_t ulCount = 0;
	UttecUtil myUtil;
	ulCount++;	
	uint8_t ucCmd = pFrame->Cmd.Command;
	printf("From 485:%d -> ", ucCmd);
	switch(ucCmd){
		case edDummy:
		case edSensor:
		case edRepeat:
		case edLifeEnd:
		case edNewSet:
		case edNewSetAck:
		case edSearch:
		case edBack:
		case edAsk:
			printf("TBD\r\n");
			break;
		case edVolume:
			if(myUtil.isMst(mp_rfFrame)){
				printf("and by485: isMst -> ");
				pMy485->send485(pFrame, eRsDown);
				return;
			}			
			if(myUtil.isGw(mp_rfFrame)&&
				(!myUtil.isNotMyGwGroup(mp_rfFrame,pFrame))){
				printf("and by485: isGw -> ");
				pMy485->send485(pFrame, eRsDown);
				return;
			}			
			if(myUtil.isTx(mp_rfFrame)){
				printf("byRf: isTx ");
				pMyRf->sendRf(pFrame);	
				printf("and by485: isTx -> ");
				pMy485->send485(pFrame, eRsDown);
			}
			procVolumeCmd(pFrame);
				break;
		case edDayLight:
				break;
		case edServerReq:
			if(myUtil.isMyAddr(pFrame, mp_rfFrame)){	// if all address are same
				if(pMyServer->taskServer(pFrame)){			// execute taskServer
					wait(0.01);														// if command is status
					printf("Now ready for return\n\r");		// return process
					pMy485->send485(pFrame, eRsUp);
				}
				return;
			}
			if(myUtil.isMstOrGw(mp_rfFrame)){
				ping_t* pPing = (ping_t*)&pFrame->Trans;
				pMyRf->changeGroup(pPing->gid);
				printf("changeGroup %d\r\n", pPing->gid);
				pMy485->send485(pFrame, eRsDown);
				pMyRf->sendRf(pFrame);		// check whether to change channel
				printf("From mst or gw send 485Frame to Tx -> end \n\r");
//				pMyRf->changeGroup(mp_rfFrame->MyAddr.GroupAddr); //return to origianl
				return;
			}			
			else if(myUtil.isTx(mp_rfFrame)){
				printf("Tx: from Mst, send SxFrame to Rx -> ");
				pMy485->send485(pFrame, eRsDown);
				
				wait(0.3);	//For delay, prevent to confict with Gateway Frame
				pMyRf->sendRf(pFrame);		// check whether to change channel
			}
				break;
		case edClientAck:
			if(myUtil.isMstOrGw(mp_rfFrame)){
				if(myUtil.isTx(pFrame)){
					pFrame->MyAddr.RxTx.iRxTx = eGW;
					pMy485->send485(pFrame, eRsUp);
					printf("Gw: from tx, send 485Frame to Mst -> end\n\r");
				}
				else dispErrorRxTx(mp_rfFrame, pFrame);
				return;
			}			
			else if(myUtil.isTx(mp_rfFrame)){
				if(!myUtil.isMstOrGw(pFrame)){
					printf("Tx: from Rx, SRx, Rpt, send 485Frame to Mst -> end\n\r");
					pFrame->MyAddr.RxTx.iRxTx = eTx;
					pMy485->send485(pFrame, eRsUp);
				}
				else dispErrorRxTx(mp_rfFrame, pFrame);
			}
				break;
		default:
			printf("rs485 Check Cmd %d\n\r", ucCmd);
			break;
	}
}
