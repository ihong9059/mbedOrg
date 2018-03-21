#ifndef __MSECEXE_H__
#define __MSECEXE_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "nrf.h"
#include "DimmerRf.h"

#define floatZero (float)0.01
#define VolumeZeroPoint (float)0.01
	
#define TimeoutForPhotoRepeat 500 
#define TimeoutForVolumeRepeat 200 
#define TimeoutForPirRepeat 500 

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
	float pwm;
} UttecPir_t;

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
	float pwm;
	uint32_t volumeCount;
} UttecVol_t;

typedef struct{
	bool forced;
	uint32_t dTime;
	float current;
	float target;	
	float pwm;	
	float upStep;
	float downStep;	
} UttecDim_t;

class mSecExe
{
private:
	static DimmerRf* m_pRf;
	static bool m_sensorFlag;

	void switchSensorType(rfFrame_t*);
	void procDim(UttecDim_t);
	void setSensorFlag();

public:
	static eSensorType_t m_sensorType;
	static UttecDim_t sDim;

	mSecExe(DimmerRf*);
	void msecTask(rfFrame_t*);
	bool returnSensorFlag();
	void clearSensorFlag();

	void switchDimType(rfFrame_t*);
	void setForcedDim(float level);
	void setUnforcedDim();
};

#endif
