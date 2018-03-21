#include "mbed.h"
//#include "Serial.h"

DigitalOut led1(LED1);
Serial pc(p9,p11);
Ticker myTimer;
// main() runs in its own thread in the OS
bool flag_Sec = false;
bool flag_mSec = false;


void call_mSec(){
	flag_mSec = true;
}

void disTime(uint32_t ulTime){
	tm myTime;
	myTime.tm_sec = ulTime%60;
	myTime.tm_min = (ulTime/60)%60;
	myTime.tm_hour = (ulTime/3600)%24;
	myTime.tm_mday = ulTime/(3600*24);
	printf("%d:day %d:H %d:M %d:S\n\r", myTime.tm_mday,
	myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
}

	AnalogIn aIn(p1);
	AnalogIn aIn1(p2);

int main() {
	pc.baud(115200);
	pc.printf("Empty Test program from 2017.12.17 17:33\n\r");
	
	uint32_t ulCount = 0;
	uint32_t ul_mSecCount = 0;
	
//	myTimer.attach(&call_Sec, 1);
	myTimer.attach_us(&call_mSec, 1000);
	PwmOut myPwm(LED2);
	Timer timer;
	float duty = 0.0;
	timer.start();
	
	while (true) {
		if(flag_Sec){
			printf("timer.reset\n\r");
			
			flag_Sec = false;
			led1 = !led1;
			if(led1) printf("High = %d\n\r", ulCount++);
			else printf("Low = %d\n\r", ulCount++);
			timer.reset();
			disTime(ulCount);
			printf("uSec1 = %d\n\r", timer.read_us());
			duty = duty + 0.1;
			if(duty >= 1.0) duty = 0.0;
			myPwm = duty;
			printf("Pwm = %f \n\r", duty);
			printf("uSec2 = %d\n\r", timer.read_us());
			printf("aIn = %d \n\r", aIn.read_u16());
			printf("aIn1 = %d \n\r", aIn1.read_u16());
		}
		
		if(flag_mSec){
			flag_mSec = false;
			ul_mSecCount++;
			if(!(ul_mSecCount%1000)){
				flag_Sec = true;
				printf("mSec = %d\n\r",ul_mSecCount);
			}
		}
	}
}

