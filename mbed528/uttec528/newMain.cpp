#include "mbed.h"
#include "nrf.h"
#include <stdlib.h>

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

Ticker timer;
static bool my1Sec = true;
void attime(){
	my1Sec = true;
}

test myTest;
Flash myFlash;


int main(void)
{
#ifdef 	my52832
Serial Uart(p6, p8);
#else
Serial Uart(p11, p8);
#endif
	
	uttecLib_t myLib;	
	
	Uart.baud(115200);	
	rs485 my485(&Uart);
	
	timer.attach(&attime, 1);
	
	Uart.printf("\n\rNow New nrf51822 2018.01.15 11:00\n\r");
	printf("\r\n bsl Manual Control\n\r");
	
	uint32_t ulCount = 0;

	myFlash.ReadAllFlash();	
	Flash_t* pFlash = myFlash.getFlashFrame();
	rfFrame_t* pMyFrame=&pFlash->rfFrame;
	
	UttecUtil myUtil;
	DimmerRf myRf(&myFlash);
	myRf.initRfFrame(); 

	UttecBle myBle;
	
	mSecExe my_mSec(&myRf);
	
	myLib.pFlash = &myFlash;
	myLib.pDimmerRf = &myRf;
	myLib.pRs485 = &my485;
//	myLib.pSx1276 = &mySx1276;
	myLib.pBle = &myBle;
	myLib.pMsec = &my_mSec;

	procServer mProcServer(myLib);
	procRf mProcRf(myLib, &mProcServer);
	uint16_t uidData[7];
	
	printf("Now Start Bluetooth\r\n");
	
	while(1){
		if(my1Sec){
			static bool bToggle = false;
			my1Sec = false;
			bToggle = !bToggle;
			if(bToggle)
				Uart.putc('-');
			else
				Uart.putc('|');
		}
		if(my485.isTestDone()){
			my485.clearTestDone();
			
			rs485Status_t* pStatus = my485.return485Status();
			uint8_t* pData = my485.returnTestFrame();
			char cTemp;
			uint16_t uiTemp;
			uint8_t ucCount = 0;
			
			for(int i = 0; i<pStatus->count; i++){
				if(*pData == 44){
//					printf("\r\n");
//					printf("\r\n Value = %d \r\n", uiTemp);
					uidData[ucCount] = uiTemp;
					ucCount++;
					uiTemp=0;
				}
				else{
					cTemp = *pData;
					if(cTemp<'0' || cTemp > '9'){
						if(!ucCount) uiTemp = cTemp;
					}
					else
						uiTemp = cTemp -'0'+ uiTemp*10;
				}
				pData++;
			}
			rfSet_find_t sFind;
			rfSet_set_t sSet;
			switch(uidData[0]){
				case 'F':
					sFind.cmd = edSearch;
					sFind.start = uidData[1];
					sFind.end = uidData[2];
					mProcRf.sendFind(sFind);
					break;
				case 'S':
					sSet.cmd = edNewSet;
					sSet.gid = uidData[1];
					sSet.pid = uidData[2];
					sSet.rxtx = uidData[3];
					sSet.high = uidData[4];
					sSet.low = uidData[5];
					sSet.dtime = uidData[6];
					mProcRf.sendSet(sSet);
					break;
				default:
					break;
			}
//			Uart.printf(":is Test Done Ok \n");
		}	
		/*
		*/
	}
}
