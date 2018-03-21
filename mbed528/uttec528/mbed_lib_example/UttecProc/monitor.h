#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "uttecLib.h"

/* Monitoring Scenario
* 1. Tx가 Data Base를 갖고 있는다.
*	2. Tx가 Schedule에 따라, Monitoring
*	3. Tx Data Base에 Monitoring 정보 보관
*	4. Server를 통한 Monitor결과 요구시 전송
* at Tx, initMonitor: set ackTimeout, nextTimeout 
*/
typedef enum{
	eMNoProblem = 0,
	eMLedOut = 1,
	eMNoAck = 2
} eMonitor_t;

typedef struct{
	uint8_t ackTime;
	uint8_t nextTime;
	uint8_t nextNum;
	uint8_t totalNum;
	uint8_t memBit[24];
	uint8_t member[96];
} monitor_t;

typedef struct{
	uint8_t ackTime;
	uint8_t nextTime;
	uint8_t nextNum;
	uint8_t totalNum;
	eMonitor_t memBit[12];
	uint8_t member[96];
} monitor1_t;

class monitor
{
private:
	static monitor_t m_monitor;

	void clearAckFlag();
	void clearNextFlag();
	void askAlive();
	void reformMonitor();

public:	
	monitor();
	void initMonitor(uint32_t, uint32_t);
	void requestForAlive(uint8_t);
	bool isMonitor();
	void procMonitor();
	
	void receivedAliveAck(uint8_t,eMonitor_t);
	void setTestData();
};	

#endif
