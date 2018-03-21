#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "uttecLib.h"

typedef enum{
	eMNoProblem = 0,
	eMLedOut = 1,
	eMNoAck = 2
} eMonitor_t;

class monitor
{
private:
public:	
	static eMonitor_t m_monitorResult;
	static bool m_lampOk;

	monitor();
	bool isLampOk();
	uint32_t getTraffic();
	void setTrafficFlag();
	bool getTrafficCountFlag();
	void clearTrafficCountFlag();
	eMonitor_t getMonitorResult();
	uint16_t getCurrentPhoto();
	uint32_t getPower();
};	

#endif
