#ifndef __VOLUME_H__
#define __VOLUME_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "Flash.h"

#define floatVol_Zero (float)0.01
#define MaxVolumeCount 10000

typedef struct{
	bool flag;
	uint32_t dTime;
	float rate;
	float realUpper;
	float upper;
	float lower;
	float max;
	float min;
	float average;
	float current;
	float target;
	uint32_t volumeCount;
} vol_t;

class volume
{
private:
	float averageSensor(float);
	void monitorSensorFactor();
	void setNextRange();

public:
	static vol_t m_sVol;

	volume();
	bool procVolumeSw();
	void setSensorRate(float);	
	void clearSensorFlag();
	float getTarget();
};

#endif
