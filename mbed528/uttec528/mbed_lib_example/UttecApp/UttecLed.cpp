#include <stdio.h>
#include <string.h>

#include "UttecLed.h"

DigitalOut UttecLed::rfLed = DigitalOut(LED1);
DigitalOut UttecLed::sensLed = DigitalOut(LED2);
UttecLed_t UttecLed::sRfLed = {0,};
UttecLed_t UttecLed::sSensLed = {0,};

UttecLed::UttecLed(){
	/*
	sRfLed.status = sSensLed.status = false;
	sRfLed.forced = sSensLed.forced = false;
	sRfLed.liveTime = sSensLed.liveTime = 0;
	*/
}

void UttecLed::taskLed(){
	if(!sRfLed.forced){
		if(sRfLed.liveTime){
			rfLed = eOn;
			sRfLed.liveTime--;
		}
		else rfLed = eOff;
	}
	if(!sSensLed.forced){
		if(sSensLed.liveTime){
			sensLed = eOn;
			sSensLed.liveTime--;
		}
		else sensLed = eOff;
	}
}

void UttecLed::on(enumLed_t eLed){
	switch(eLed){
		case eRfLed:
			sRfLed.forced = eForced;
			rfLed = eOn;
			break;
		case eSensLed:
			sSensLed.forced = eForced;
			sensLed = eOn;
			break;
		default:
			break;
	}
}
void UttecLed::off(enumLed_t eLed){
	switch(eLed){
		case eRfLed:
			sRfLed.forced = eForced;
			rfLed = eOff;
			break;
		case eSensLed:
			sSensLed.forced = eForced;
			sensLed = eOff;
			break;
		default:
			break;
	}
}
void UttecLed::unforced(enumLed_t eLed){
	switch(eLed){
		case eRfLed:
			sRfLed.forced = eNonForced;
			break;
		case eSensLed:
			sSensLed.forced = eNonForced;
			break;
		default:
			break;
	}
}
void UttecLed::blink(enumLed_t eLed, uint16_t uiTime){
	switch(eLed){
		case eRfLed:
			sRfLed.liveTime = uiTime;
			break;
		case eSensLed:
			sSensLed.liveTime = uiTime;
			break;
		default:
			break;
	}
}
