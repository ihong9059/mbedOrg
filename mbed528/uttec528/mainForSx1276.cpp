#include "mbed.h"
#include "nrf.h"

#include "UttecUtil.h"
#include "CmdDefine.h"
#include "uttecLib.h"
#include "UttecLed.h"
#include "Rcu.h"
#include "test.h"
#include "eprom.h"

#include "procRf.h"
#include "procBle.h"
#include "procSx1276.h"
#include "proc485.h"
#include "procSec.h"

#include "procServer.h"

#include "simSx.h"
#include "nrf_ble_gap.h"

DigitalIn CTS(p10, PullDown);

UttecBle myBle;

Flash myFlash;
test myTest;

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
#include "dac.h"
dac myDac;
/*
#include "Rand.h"
#include "nrf_ble_gap.h"
*/
int main(void)
{
Serial Uart(p9,p11);
	uttecLib_t myLib;	
	Uart.baud(115200);	
	printf("\n\rNow New nrf51822 2018.01.15 11:00\n\r");
	printf("bsl Manual Control\n\r");

	myFlash.ReadAllFlash();	
	Flash_t* pFlash = myFlash.getFlashFrame();
	rfFrame_t* pMyFrame=&pFlash->rfFrame;
	
	myFlash.isFactoryMode();
	
	printf("From Flash Mac Address:\n\r");
	for(int i = 0; i<6; i++){
		printf("Mac[%d]:%x, ",i ,pFlash->MacAddr.mac[i]);
	}
	printf("\r\n");
	
	printf("My Gid =%d, %f\n\r", pMyFrame->MyAddr.GroupAddr,
		pFlash->VolumeCheck);
	printf("high =%d, low =%d\n\r", pMyFrame->Ctr.High,
		pMyFrame->Ctr.Low);

	UttecUtil myUtil;
	DimmerRf myRf(&myFlash);
	myRf.initRfFrame(); 
	secTimer.attach(&tickSec, 1);
	msecTimer.attach(&tickmSec, 0.001);
		
	mSecExe my_mSec(&myRf);
	rs485 my485(&Uart);	
//	mySx1276.initSx1276(0);
	UttecBle myBle;
	
	myLib.pFlash = &myFlash;
	myLib.pDimmerRf = &myRf;
	myLib.pRs485 = &my485;
//	myLib.pSx1276 = &mySx1276;
	myLib.pBle = &myBle;
	myLib.pMsec = &my_mSec;
	
simSx mySim(&myRf);	

	procServer mProcServer(myLib);
	procRf mProcRf(myLib, &mProcServer);
	procBle mProcBle(myLib, &mProcServer);
	proc485 mProc485(myLib, &mProcServer);
	procSec mProcSec(myLib, &mProcServer);
	procSx1276 mProcSx1276(myLib, &mProcServer);

	mProcSx1276.changeGroup(pMyFrame->MyAddr.GroupAddr);

//#define DeFlashInit 1
#ifdef DeFlashInit 	
	myFlash.resetFlash();	
#endif

	Rcu myRcu;	
	myTest.setTest(myLib, &mProcServer);
	
UttecLed myLed;
	myUtil.setProductType();
	
	myFlash.isHardwareOk();	
//	myFlash.setHardwareError(100);
	
	myUtil.setWdt(6);	
	while(true){
		myUtil.setWdtReload();
		
		if(myUtil.m_product.rcu)
		if(myRcu.isRcuReady()){
			rcuValue_t myCode;
			myRcu.clearRcuFlag();
			myCode = (rcuValue_t)myRcu.returnRcuCode(); 
			myRcu.procRcu(myCode);
		}

		if(myUtil.m_product.rf)
		if(myRf.isRxDone()){		//For Rf Receive
			myLed.blink(eRfLed, eRfBlink);
			myRf.clearRxFlag();
			rfFrame_t* pFrame = myRf.returnRxBuf();
			mProcRf.taskRf(pFrame);		
		}

		if(0)
		if(myUtil.m_product.sx1276)
		if(mySim.isSxRxDone()){		//For sx1276 Receive
			printf("-------------isSxRxDone\n\r");
			mySim.clearSxRxFlag();
			
			rfFrame_t* psRf = mProcSx1276.readSxFrame();
			sxFrame_t* psx = (sxFrame_t*)psRf;
			uint8_t* pMyCrc = (uint8_t*)psx;
			uint16_t myCrc = myUtil.gen_crc16(pMyCrc, sizeof(sxFrame_t)-2);
			if( psx->crc  != myCrc) printf("--------- Crc Error: %d, %d\r\n",
				psx->crc, myCrc);
			
			printf("gid = %d, %d\n\r", psRf->MyAddr.GroupAddr, 
				psRf->Ctr.Level);
			printf("\n\r");
			if(mProcSx1276.isMyGroup(pMyFrame, psRf))
				mProcSx1276.sx1276Task(psRf);
		}
		
		if(myUtil.m_product.rs485)
		if(my485.is485Done()){		//For rs485 Receive			
			my485.clear485Done();
			rfFrame_t* p485Frame = my485.return485Buf();
			mProc485.rs485Task(p485Frame);
		}		
/*		
*/		
				
		if(my_mSec.returnSensorFlag()){		//For sensor Receive
			my_mSec.clearSensorFlag();
			if(!myUtil.isRx(pMyFrame))
				myRf.sendRf(pMyFrame);
		}
		
		if(tick_mSec){
			myLed.alarm();
			tick_mSec = false;
			my_mSec.msecTask(pMyFrame);
		}
		
		if(tick_Sec){		
			tick_Sec = false;			
			mProcSec.secTask(pMyFrame);	
			
		}		
	}
}


//#include "bh1750.h"
//#include "Pyd1788.h"
//#include "eprom.h"
//#include "HX711.h"
//#include "procSx1276.h"

//bh1750 my1750;		//100KHz
//Pyd1788 myPyd(p4);
//eprom myEprom;
//HX711 scale;

/*		

		if(my485.isAnyDone()){
			if(my485.is485Done()){		//For rs485 Receive
				my485.clear485Done();
				printf("\n\ris485Done\n\r");
				mProc485.rs485Task(my485.return485Buf());
			}		
		
		if(myRcu.isRcuReady()&&myRcu.isUttecCode()){
			myRcu.returnUtRcuCode(); 
			if(myRcu.isUtRcuReady()){
				myRcu.clearUtRcuFlag();
				rcuFrame_t* pRcu;
				pRcu = myRcu.returnUtRcuCode();
				printf("rcu0: %llx, rcu1: %llx\n\r",
					pRcu->rcu0.ulData, pRcu->rcu1.ulData);
				myRcu.procUtRcu(pRcu);				
			}
*/
