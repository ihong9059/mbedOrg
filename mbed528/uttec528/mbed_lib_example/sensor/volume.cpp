#include <stdio.h>
#include <string.h>

#include "volume.h"

AnalogIn volPin(p4);
vol_t volume::m_sVol = {0,};

volume::volume(){
}

#define DeVolAverageCount 1000
float volume::averageSensor(float fValue){
	static uint16_t uiCount = 0;
	static float fNow = 0.0;
	if(uiCount < DeVolAverageCount){
		fNow += fValue;
		uiCount++;
	}
	else{
		fNow = fNow - m_sVol.average;
		fNow = fNow + fValue;
		uiCount = DeVolAverageCount;
	}
	m_sVol.average = fNow/(float)uiCount; 
	return m_sVol.average;
}
void volume::setNextRange(){
	m_sVol.upper = m_sVol.current + m_sVol.rate;
	m_sVol.lower = m_sVol.current - m_sVol.rate;
	if(m_sVol.upper > m_sVol.max) m_sVol.upper = m_sVol.max;
	if(m_sVol.lower < m_sVol.min) m_sVol.lower = m_sVol.min;
}

bool volume::procVolumeSw(){
	Flash myFlash;
	Flash_t* pFlash = myFlash.getFlashFrame();
	
	if(m_sVol.flag) return m_sVol.flag;
	
	m_sVol.current = volPin.read();	//check range of max, min
	if(m_sVol.max<m_sVol.current) m_sVol.max = m_sVol.current;
	else if(m_sVol.min>m_sVol.current) m_sVol.min = m_sVol.current;
	averageSensor(m_sVol.current);
	
	if(!(pFlash->VolumeCheck>0.001)){
		m_sVol.volumeCount++;
		if(m_sVol.volumeCount == MaxVolumeCount){
				m_sVol.rate = (m_sVol.max - m_sVol.min)/(float)100.0;
				pFlash->VolumeCheck = m_sVol.rate;
				myFlash.writeFlash();
				printf("------------------Save range:unit = %0.3f, max = %0.3f, min = %0.3f\n\r",
					m_sVol.rate, m_sVol.max, m_sVol.min);
		}
	}
	else m_sVol.rate = pFlash->VolumeCheck;
	
	if(m_sVol.current > m_sVol.upper){
		m_sVol.flag = true;
		m_sVol.target = m_sVol.current/(m_sVol.rate*100);
		setNextRange();
	}
	else if(m_sVol.current <= (m_sVol.lower)){
		m_sVol.flag = true;
		m_sVol.target = m_sVol.current/(m_sVol.rate*100);
		setNextRange();
	}
	return m_sVol.flag;
}

void volume::setSensorRate(float fLimit){
//	fLimit = 0.01;
	m_sVol.flag = false;
	m_sVol.average = 0.5;
	m_sVol.max = 0.5;
	m_sVol.min = 0.5;
	m_sVol.current = 0.5;
	
	m_sVol.upper = 1.0 - fLimit;
	m_sVol.lower = fLimit;
	m_sVol.rate = fLimit;
}

void volume::clearSensorFlag(){
	m_sVol.flag = false;
}
float volume::getTarget(){
	return m_sVol.target;
}

void volume::monitorSensorFactor(){
	Flash flash;
	Flash_t* pFlash = flash.getFlashFrame();
	printf("Pir Value = %0.3f\n\r", m_sVol.current);
	printf("rate = %0.3f, rUpper = %0.3f, Upper = %0.3f, Lower = %0.3f\n\r",
	m_sVol.rate, m_sVol.realUpper, m_sVol.upper, m_sVol.lower);
	printf("max = %0.3f, min = %0.3f, avr = %0.3f, current = %0.3f \n\r",
	m_sVol.max, m_sVol.min, m_sVol.average, m_sVol.current);	
  printf("Flash Size = %d, float size = %d\n\r", 
		sizeof(Flash_t), sizeof(float));
	printf("Flash float init value = %f\n\r", pFlash->VolumeCheck);
	printf("Gid = %d\n\r", pFlash->rfFrame.MyAddr.GroupAddr);
}

