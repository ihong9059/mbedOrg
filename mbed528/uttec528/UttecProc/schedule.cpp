#include <stdio.h>
#include <string.h>

#include "schedule.h"

static schedule_t m_schedule = {0,};

schedule::schedule(){
}

static Ticker scheduleTicker;

static void executeSchedule(){
	m_schedule.now++;
	m_schedule.flag = true;
}
	
void schedule::initSchedule(){
	memset(&m_schedule, 0, sizeof(schedule_t));
	m_schedule.total = 1;
	scheduleTicker.attach(&executeSchedule, DeScheduleInterval);
}

void schedule::setSchedule(uint8_t ucTime, uint8_t ucLevel){
	for(int i = 0; i<m_schedule.total; i++){
		if((ucTime != m_schedule.event[i].time)||
			(ucLevel != m_schedule.event[i].level))
		m_schedule.total++;
		if(m_schedule.total>DeScheduleEventMax) break;
		else{
			m_schedule.event[m_schedule.total].time = ucTime;
			m_schedule.event[m_schedule.total].level = ucLevel;
		}
	}
}

void schedule::procSchedule(){
	if(m_schedule.flag){
		m_schedule.flag = false;
		printf("m_schedule.now = %d\n\r", m_schedule.now);
	}
}




