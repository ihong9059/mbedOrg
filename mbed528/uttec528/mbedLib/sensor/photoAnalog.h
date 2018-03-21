#ifndef __PHOTOA_H__
#define __PHOTOA_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "Flash.h"
#include "UttecUtil.h"

#include "BH1750.h"

#define floatVol_Zero (float)0.01
#define MaxVolumeCount 10000

typedef enum{
	ePhotoAnalog = 0,
	ePhotoDigital = 1
} photoType_t;

typedef struct{
	bool flag;
	UttecDirection_t direction;
	uint32_t dTime;
	float reference;
	float rate;
	float next;
	float max;
	float min;
	float average;
	float current;
	float target;
	uint32_t volumeCount;
} photoA_t;

class photoAnalog
{
private:
	float averageSensor(float);
	void monitorSensorFactor();
	void setNextRange();

public:
	static photoA_t m_sPhotoA;

	photoAnalog();
	bool procPhotoA(photoType_t);
	void setSensorRate(float);	
	void clearSensorFlag();
	UttecDirection_t getDir();
	float getTarget();
	void setBH1750Mode(BH1750_Mode_t);
	float getPhotoAnalog();
};

#endif
