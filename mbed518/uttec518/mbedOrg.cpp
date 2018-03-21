#include "mbed.h"
#include "ble/BLE.h"
#include "ble_gap.h"
#include "ble/services/UARTService.h"
#include "nrf_nvmc.h"
#include "nrf_soc.h"

#include "main.h"
#include "sx1276-hal.h"
#include "debug.h"
#include "UttecUtil.h"
#include "CmdDefine.h"
#include "uttecLib.h"
#include "Rcu.h"
#include "test.h"

#include "procRf.h"
#include "procBle.h"
#include "procSx1276.h"
#include "proc485.h"
#include "procSec.h"

#include "procServer.h"

Serial Uart(UartTx,UartRx);	//p9, p11
UttecBle myBle;
BLE temBle;

Flash myFlash;
test myTest;
sx1276Exe mySx1276;

//                    01234567890123456789012345    
uint8_t ucdTest[50] = "123{04d2010101640287}9873";

bool tick_Sec = false;
bool tick_mSec = false;

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

int main(void)
{
	
	uttecLib_t myLib;
//https://os.mbed.com/handbook/Serial	
	Uart.baud(115200);
	printf("\n\rNow New nrf51822 2017.12.18 12:50\n\r");

	mySx1276.initSx1276();
			
	myBle.InitBle4Rf();
//    myBle.InitBle(1);
    
//    UARTService uartService(*myBle.getBleDevice());
//    myBle.setUartServicePtr(&uartService);
	
	ble_gap_addr_t addr;
	sd_ble_gap_address_get(&addr);

	myFlash.ReadAllFlash();
	Flash_t* pFlash = myFlash.getFlashFrame();
	rfFrame_t* pFrame=&pFlash->rfFrame;	
	sxFrame_t* psxFrame = (sxFrame_t*)pFrame;	
	myFlash.isFactoryMode();
	
	printf("My Gid =%d, %f\n\r", pFrame->MyAddr.GroupAddr,
		pFlash->VolumeCheck);
 	
	psxFrame = (sxFrame_t*)pFrame;
	printf("\n\r");	
	for(int i=0; i<sizeof(addr.addr); i++)
		printf("mac[%d]:%x ", i,psxFrame->mac[i]);
	printf("\n\r");	
	
	UttecUtil myUtil;	
	DimmerRf myRf(&myFlash);
	myRf.initRfFrame(); 
	secTimer.attach(&tickSec, 1);
	msecTimer.attach(&tickmSec, 0.001);
	
	mSecExe my_mSec(&myRf);
	rs485 my485(&Uart);
	mySx1276.initSx1276(0);
	UttecBle myBle;
		
	myLib.pFlash = &myFlash;
	myLib.pDimmerRf = &myRf;
	myLib.pRs485 = &my485;
	myLib.pSx1276 = &mySx1276;
	myLib.pBle = &myBle;
	myLib.pMsec = &my_mSec;
	
	procServer mProcServer(myLib);
	procRf mProcRf(myLib, &mProcServer);
	procBle mProcBle(myLib, &mProcServer);
	procSx1276 mProcSx1276(myLib, &mProcServer);
	proc485 mProc485(myLib, &mProcServer);
	procSec mProcSec(myLib, &mProcServer);
/*
*/
	//flash.resetFlash();
	uint32_t uiTest = 0;
	uint8_t ucChannel = 0;

	myUtil.setWdt(5);

	Rcu myRcu;
	myTest.setTest(myLib, &mProcServer);
	
	while(true){
		myUtil.setWdtReload();
		
		if(mProcSec.m_product.rcu)
		if(myRcu.isRcuReady()){
//		if(myRcu.isRcuReady()&&myRcu.isUttecCode()){
			rcuValue_t myCode;
			myRcu.clearRcuFlag();
			myRcu.returnUtRcuCode(); 
//			myCode = (rcuValue_t)myRcu.returnRcuCode(); 
//			myRcu.procRcu(myCode);
			if(myRcu.isUtRcuReady()){
				myRcu.clearUtRcuFlag();
				rcuFrame_t* pRcu;
				pRcu = myRcu.returnUtRcuCode();
				printf("rcu0: %llx, rcu1: %llx\n\r",
					pRcu->rcu0.ulData, pRcu->rcu1.ulData);
				myRcu.procUtRcu(pRcu);				
			}				
		}

		if(mProcSec.m_product.rf)
		if(myRf.isRxDone()){		//For Rf Receive
			myRf.clearRxFlag();
			rfFrame_t* pFrame = myRf.returnRxBuf();
			mProcRf.taskRf(pFrame);			
		}
		
		if(mProcSec.m_product.ble)
		if(myBle.isBleRxReady()){		//For Ble Receive
			myBle.clearBleRxReady();
			mProcBle.bleTask(myBle.getBleRxData());
		}
		
		if(mProcSec.m_product.rs485)
		if(my485.is485Done()){		//For rs485 Receive
			my485.clear485Done();
			mProc485.rs485Task(my485.return485Buf());
		}		
		
/*		
		if(mySx1276.isSxRxReady()){		//For sx1276 Receive
			rfFrame_t sxRfFrame = *(rfFrame_t*)mySx1276.readLoRa()->ptrBuf;
			printf("-----------i received readLoRa: \n\r");
			mProcSx1276.dispSx1276();
			
			mySx1276.clearSxRxFlag();
				if(!myUtil.isTx(pFrame)){
					printf("Resend rfFrame\n\r");
//					wait(0.01);
					mProcSx1276.sendSxFrame(&sxRfFrame);
				}
		}
		*/
		if(mySx1276.isSxRxReady()){		//For sx1276 Receive
			printf("-------------isSxRxReady\n\r");
			mySx1276.clearSxRxFlag();
			rfFrame_t* psRf = mProcSx1276.readSxFrame();
			sxFrame_t* psx = (sxFrame_t*)psRf;
			printf("gid = %d, %d\n\r", psRf->MyAddr.GroupAddr, 
				psRf->Ctr.Level);
			printf("\n\r");	
			for(int i = 0; i<sizeof(addr.addr); i++){
				printf("mac[%d]:%x ", i, psx->mac[i]);
			}
			printf("\n\r");	
			if(mProcSx1276.isMyGroup(pFrame, psRf))
				mProcSx1276.sx1276Task(psRf);
		}
		
		if(my_mSec.returnSensorFlag()){		//For sensor Receive
			my_mSec.clearSensorFlag();
//			myRf.sendRf(pFrame);
//			my485.send485(pFrame);
//			mySx1276.sendSx1276(pFrame);
//			myBle.sendBle(pFrame);
		}
		
		if(tick_mSec){
			tick_mSec = false;
			my_mSec.msecTask(pFrame);
		}
/*		
		if(tick_Sec){
//	pFrame->MyAddr.RxTx.iRxTx = eTx;
			if(!(uiTest++%2)){
				if(myUtil.isTx(pFrame)){
					if(myUtil.isTx(pFrame)){
						pFrame->Cmd.Command = edServerReq;
						pFrame->Cmd.SubCmd = edsControl;
						mProcSx1276.sendSxFrame(pFrame);
					}
				}
			}
			if(!(uiTest%20)){
				ucChannel++;
			}
			printf("Sec proc\n\r");
			tick_Sec = false;			
//			mProcSec.secTask(pFrame);			
		}		
		*/
	}
}
