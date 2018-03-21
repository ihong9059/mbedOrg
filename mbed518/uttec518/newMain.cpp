#include "mbed.h"
#include "nrf.h"
#include <stdlib.h>

#include "test.h"
#include "rs485.h"

Ticker timer;
static bool my1Sec = true;
void attime(){
	my1Sec = true;
}

test myTest;

int main(void)
{
Serial Uart(p9,p11);//tx, rx
	Uart.baud(115200);	
	rs485 my485(&Uart);
	
	timer.attach(&attime, 1);
	
	printf("\n\rNow New nrf51822 2018.01.15 11:00\n\r");
	printf("bsl Manual Control\n\r");
	uint32_t ulCount = 0;
	while(1){
//		printf("my51822:%d\r\n", ulCount++);
		if(my1Sec){
			my1Sec = false;
			Uart.printf("ulCount = %d\r\n", ulCount++);
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
					printf("\r\n");
					printf("\r\n Value = %d \r\n", uiTemp);
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
			printf("isTestDone\r\n");
		}	
	}
}
