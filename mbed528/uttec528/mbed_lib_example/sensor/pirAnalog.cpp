#include <stdio.h>
#include <string.h>

#include "pirAnalog.h"

AnalogIn pirAPin(p3);
Pyd1788 pirDPin(p4);
pirA_t pirAnalog::m_sPirA = {0,};

pirAnalog::pirAnalog(){
}

#define DePirAverageCount 1000
float pirAnalog::averageSensor(float fValue){
	static uint16_t uiCount = 0;
	static float fNow = 0.0;
	if(uiCount < DePirAverageCount){
		fNow += fValue;
		uiCount++;
	}
	else{
		fNow = fNow - m_sPirA.average;
		fNow = fNow + fValue;
		uiCount = DePirAverageCount;
	}
	m_sPirA.average = fNow/(float)uiCount; 
	return m_sPirA.average;
}

bool pirAnalog::procPirSensor(pirType_t sType){
	if(m_sPirA.flag) return m_sPirA.flag;
	
	if(sType == ePirAnalog){
		m_sPirA.current = pirAPin.read();
		if(m_sPirA.max<m_sPirA.current) m_sPirA.max = m_sPirA.current;
		else if(m_sPirA.min>m_sPirA.current) m_sPirA.min = m_sPirA.current;

		m_sPirA.realUpper= m_sPirA.max*m_sPirA.upper;
		averageSensor(m_sPirA.current);
		
		m_sPirA.average = (m_sPirA.max + m_sPirA.min)/(float)2.0;	
		if(m_sPirA.current>m_sPirA.realUpper){
			m_sPirA.flag = true;
		}
		else if(m_sPirA.current<(m_sPirA.lower+floatPirA_Zero)){
			m_sPirA.flag = true;
		}
	}
	else{
		pirDPin.processPyd1788();
		m_sPirA.flag = pirDPin.read();
	}
	return m_sPirA.flag;
}

void pirAnalog::setSensorRate(float fLimit){
//	fLimit = 0.01;
	m_sPirA.flag = false;
	m_sPirA.average = 0.5;
	m_sPirA.max = 0.5;
	m_sPirA.min = 0.5;
	m_sPirA.current = 0.5;
	
	m_sPirA.upper = 1.0 - fLimit;
	m_sPirA.lower = fLimit;
	m_sPirA.rate = fLimit;
	int iRate = fLimit*100.0;
//	printf("Rate = %f, %d\n\r", fLimit, iRate);
	pirDPin.setSenseRate(iRate+1);
}

void pirAnalog::clearSensorFlag(){
	m_sPirA.flag = false;
	pirDPin.clearFlag();
}

void pirAnalog::monitorSensorFactor(){
	Flash flash;
	Flash_t* pFlash = flash.getFlashFrame();
	printf("Pir Value = %0.3f\n\r", m_sPirA.current);
	printf("rate = %0.3f, rUpper = %0.3f, Upper = %0.3f, Lower = %0.3f\n\r",
	m_sPirA.rate, m_sPirA.realUpper, m_sPirA.upper, m_sPirA.lower);
	printf("max = %0.3f, min = %0.3f, avr = %0.3f, current = %0.3f \n\r",
	m_sPirA.max, m_sPirA.min, m_sPirA.average, m_sPirA.current);	
  printf("Flash Size = %d, float size = %d\n\r", 
		sizeof(Flash_t), sizeof(float));
	printf("Flash float init value = %f\n\r", pFlash->VolumeCheck);
	printf("Gid = %d\n\r", pFlash->rfFrame.MyAddr.GroupAddr);
}

