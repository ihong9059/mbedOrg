#include "mbed.h"
#include "nrf.h"
DigitalOut alivenessLED(LED1, 0);
Ticker secTimer;

//Serial uart(p6, p8);
Serial uart(BUTTON1,p11);

bool tick_Sec = false;
void tickSec(){
	tick_Sec = true;
}
int main(void)
{
	secTimer.attach(&tickSec, 1);
	uint32_t ulCount =0;
	uart.baud(115200);
	
	while (true) {
		if(tick_Sec){
			tick_Sec = false;
			uart.printf("Count = %d\n\r", ulCount++);
			
			alivenessLED = !alivenessLED;
		}
	}
}
