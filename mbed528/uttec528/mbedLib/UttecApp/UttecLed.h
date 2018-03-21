#ifndef __UTTECLED_H__
#define __UTTECLED_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
typedef enum{
	eRfLed = 0,
	eSensLed = 1,
	eOn = 0,
	eOff = 1,
	eForced = true,
	eNonForced = false,
	eRfBlink = 20,
	eSensBlink = 20	
} enumLed_t;

typedef struct{
	bool status;
	bool forced;
	uint16_t liveTime;
} UttecLed_t;

class UttecLed 
{
private:
	static uint32_t sAlarmTime;
public:
	static DigitalOut rfLed;
	static DigitalOut sensLed;
	static UttecLed_t sRfLed;
	static UttecLed_t sSensLed;

	UttecLed();
	void taskLed();
	void on(enumLed_t);
	void off(enumLed_t);
	void unforced(enumLed_t);
	void blink(enumLed_t,uint16_t);
	void alarm();
	void setAlarmTime(uint32_t);
};

#endif
