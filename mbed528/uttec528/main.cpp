#include "mbed.h"
#include "nrf.h"

<<<<<<< HEAD
Serial uart(USBTX, USBRX);
		
=======
#include "Flash.h"
#include "DimmerRf.h"
#include "procRf.h"
#include "rs485.h"
#include "procSec.h"
#include "proc_mSec.h"
#include "UttecUtil.h"

#include "serial_api.h"

#ifdef my52832
Serial uart(p6, p8);
#else
Serial uart(p9,p11);
#endif

Flash flash;
Ticker secTimer;
Ticker msecTimer;
Timer timer;

bool tick_Sec = false;
bool tick_mSec = false;
bool testTick = false;
#define DeTestTime 3

void tickSec(){
	static uint32_t ulSecCount = 0;
	ulSecCount++;
	if(!(ulSecCount%DeTestTime)) testTick = true;
	tick_Sec = true;
}
void tickmSec(){
	tick_mSec = true;
}

#include "CmdDefine.h"
#include "serial_api.h"

>>>>>>> 2086788bbbda04e20f499aabf2cb3f59da19b12f
int main(void)
{
	uart.baud(115200);
	printf("now start 2017.12.23\n\r");
	
}
