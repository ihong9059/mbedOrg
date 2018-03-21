#include <stdio.h>
#include <string.h>

#include "photoAnalog.h"

bh1750 myBH;

AnalogIn photoPin(p2);
photoA_t photoAnalog::m_sPhotoA = {0,};

photoAnalog::photoAnalog(){
}

#define DeVolAverageCount 1000
float photoAnalog::averageSensor(float fValue){
	static uint16_t uiCount = 0;
	static float fNow = 0.0;
	if(uiCount < DeVolAverageCount){
		fNow += fValue;
		uiCount++;
	}
	else{
		fNow = fNow - m_sPhotoA.average;
		fNow = fNow + fValue;
		uiCount = DeVolAverageCount;
	}
	m_sPhotoA.average = fNow/(float)uiCount; 
	return m_sPhotoA.average;
}

void photoAnalog::setNextRange(){
}

void photoAnalog::setBH1750Mode(BH1750_Mode_t mode){
	myBH.setMode(mode);	
}

bool photoAnalog::procPhotoA(photoType_t sType){
	static uint32_t ulcount =0;
	static uint32_t ulTest = 0;
	
	Flash myFlash;
	Flash_t* pFlash = myFlash.getFlashFrame();	
	if(sType == ePhotoDigital){
		if(!(ulcount++%0x400)){
			m_sPhotoA.current = myBH.readBH1750()/65536.0;	//0x10000 (max lux)
//			if(!(ulTest++%200))
			printf("Digital Photo = %f\r\n", m_sPhotoA.current);
		}	
	}
	else{
		m_sPhotoA.current = photoPin.read();	//check range of max, min
			if(!(ulTest++%500)){
				printf("Analog Photo = %f\r\n", m_sPhotoA.current);
//				if(m_sPhotoA.direction == eUp) printf("Up\r\n");
//				else printf("Down\r\n");
			}
	}	
	averageSensor(m_sPhotoA.current);
	
	if(m_sPhotoA.average > m_sPhotoA.next){
		m_sPhotoA.direction = eDown;
		m_sPhotoA.next = m_sPhotoA.reference + m_sPhotoA.rate;
//		printf("Down\r\n");
	}
	else if(m_sPhotoA.average <= (m_sPhotoA.next)){
		m_sPhotoA.direction = eUp;
		m_sPhotoA.next = m_sPhotoA.reference - m_sPhotoA.rate;
//		printf("up\r\n");
	}
	m_sPhotoA.flag = true;
	return m_sPhotoA.flag;
}

void photoAnalog::setSensorRate(float fLimit){
	fLimit = 0.2;
	m_sPhotoA.flag = false;
	m_sPhotoA.average = 0.5;
	m_sPhotoA.max = 0.5;
	m_sPhotoA.min = 0.5;
	m_sPhotoA.current = 0.5;
	m_sPhotoA.rate = 0.001;
	
	m_sPhotoA.next = fLimit;
	m_sPhotoA.reference = fLimit;
}

void photoAnalog::clearSensorFlag(){
	m_sPhotoA.flag = false;
}
float photoAnalog::getTarget(){
	return m_sPhotoA.target;
}
UttecDirection_t photoAnalog::getDir(){
	return m_sPhotoA.direction;
}

void photoAnalog::monitorSensorFactor(){
	Flash flash;
	Flash_t* pFlash = flash.getFlashFrame();
	printf("Pir Value = %0.3f\n\r", m_sPhotoA.current);
	printf("rate = %0.3f, next = %0.3f\n\r",
	m_sPhotoA.rate, m_sPhotoA.next);
	printf("max = %0.3f, min = %0.3f, avr = %0.3f, current = %0.3f \n\r",
	m_sPhotoA.max, m_sPhotoA.min, m_sPhotoA.average, m_sPhotoA.current);	
  printf("Flash Size = %d, float size = %d\n\r", 
		sizeof(Flash_t), sizeof(float));
	printf("Flash float init value = %f\n\r", pFlash->VolumeCheck);
	printf("Gid = %d\n\r", pFlash->rfFrame.MyAddr.GroupAddr);
}

float photoAnalog::getPhotoAnalog(){
	return photoPin;
}