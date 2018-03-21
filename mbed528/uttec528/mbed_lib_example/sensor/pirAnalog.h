#ifndef __UTTECPIRA_H__
#define __UTTECPIRA_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "Flash.h"
#include "Pyd1788.h"

#define floatPirA_Zero (float)0.01
typedef enum{
	ePirAnalog = 0,
	ePirDigital = 1
} pirType_t;
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
} pirA_t;

class pirAnalog
{
private:
	static bool m_sensorFlag;

	float averageSensor(float);
	void monitorSensorFactor();

public:
	static pirA_t m_sPirA;

	pirAnalog();
	bool procPirSensor(pirType_t);
	void setSensorRate(float);	
	void clearSensorFlag();
};


#endif
