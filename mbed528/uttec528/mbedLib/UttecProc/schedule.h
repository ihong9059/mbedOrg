#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"

/* Schedule Scenario 2018.01.21
*	1. Tx에  Database 구성
*	2. Server에서 각 Tx에 대한 Data 전송
*	각 Tx에 대해 2회 전송(Sub Command에서 edsSche1, edsSche2로 구분)
*	3. Gw에서는 전송만 담당 
* 4. Tx에 하루 Cycle로 Data저장
*	5. Max Event는 12개로 구성(2회 구성)
* 6. Event는 10분 단위로 구성(24*6 = 144)
* 7. level이 0xFF일때는 Sensor mode로 운영
*/
#define DeScheduleEventMax 12
#define DeScheduleInterval 6 //10 minutes

typedef struct{
	uint8_t id;
	uint8_t time;
	uint8_t level;
} sEvent_t;

typedef struct{
	bool flag;
	uint8_t total;
	uint8_t now;
	sEvent_t event[12];
} schedule_t;

class schedule
{
private:
//	static schedule_t m_schedule;

public:	
	schedule();
	void initSchedule();
	void setSchedule(uint8_t, uint8_t);
	void procSchedule();
};	

#endif
