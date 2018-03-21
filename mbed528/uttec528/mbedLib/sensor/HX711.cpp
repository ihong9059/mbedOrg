
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "mbed.h"

#include "HX711.h"
	
#define DeZeroSumCount	30

DigitalInOut Dout(p18);
DigitalOut Sclk(p19);

void HX711::set_gain(uint8_t gain) {
	switch (gain) {
		case 128:		// channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:		// channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:		// channel B, gain factor 32
			GAIN = 2;
			break;
	}
	HX711PowerUp();
	read();
}

HX711::HX711() 
{	
	OFFSET=0;
//	m_10g=10;		//100g
	m_10g=100;		//1Kg
	m_KgResult=0;
	AverageNum=8;
	m_AverageValue=0;
	m_TareFinish=false;
	m_TareCount=0;
	
	Dout.input();
	set_gain(128);
}

HX711::~HX711() {
}
void HX711::resetHX711() {
	OFFSET=0;
//	m_10g=10;		//100g
	m_10g=100;		//1Kg
	m_KgResult=0;
	AverageNum=8;
	m_AverageValue=0;
	m_TareCount=0;
	m_TareFinish=false;	
}

#define DeHX711Count	24

#define minus 0xffffff
#define minusLimit 0xff0000
#define readTimeOut 400

long HX711::read() {
	Timer readTime;	
	readTime.start();
	
	static long lBefore = 0;
	long value = 0;	
	
	Dout=1;
	Dout.read();	
	Sclk=0;	
	
	while(Dout.read());	
	int begin = readTime.read_us();
		
	for(int i=0; i<DeHX711Count; i++){	//read Bits
		Sclk=1;
		value=value<<1;
		Sclk=0;
		if(Dout.read()) value++;
	}
	int end = readTime.read_us();
	if((end - begin) > readTimeOut) value = lBefore;
	
	if(minusLimit< value) value = -(minus - value);	
	lBefore = value;
	
	HX711LoadCellData();	// generate Clock for Loadcell Data
	return value;	
}

long HX711::averageing(long lValue) {
	static long total=0;
	static uint8_t ucAvrCount=0;
	
	if(ucAvrCount<AverageNum){
		total+=lValue;
		ucAvrCount++;
	}
	else{
		total-=m_AverageValue;
		total+=lValue;
	}
	m_AverageValue=total/ucAvrCount;
	if((m_TareCount++>DeTareCount)&&(!m_TareFinish)){
		m_TareCount=0;
		m_TareFinish=true;
		OFFSET=m_AverageValue;
	}
	return m_AverageValue;
}

long HX711::read_average() {
	return m_AverageValue;
}

float HX711::get_value() {
	m_resolution=100;
	return m_KgResult*m_resolution;
//	return m_Value;
}


void HX711::HX711LoadCellData(){
		Sclk=1;Sclk=0;
}
void HX711::HX711BatteryData(){
		Sclk=1; Sclk=0;
		Sclk=1; Sclk=0;
}
/*
*/
void HX711::HX711PowerDown(){
	Sclk=1;
}
void HX711::HX711PowerUp(){
	Sclk=0;
}
bool HX711::returnTareFinish(){
	return m_TareFinish;
}

#include "UttecUtil.h"

Ticker Hx711_secTimer;
Ticker Hx711_msecTimer;

static bool tick_Sec = false;
static bool tick_mSec = false;
static bool testTick = false;
#define DeTestTime 3

static void Hx711_tickSec(){
	static uint32_t ulSecCount = 0;
	ulSecCount++;
	if(!(ulSecCount%DeTestTime)) testTick = true;
	tick_Sec = true;
}

static void Hx711_tickmSec(){
	tick_mSec = true;
}

void HX711::testHX711(){
	UttecUtil myUtil;
	Hx711_secTimer.attach(&Hx711_tickSec,1);
	Hx711_msecTimer.attach(&Hx711_tickmSec,0.01);
	
	uint32_t ulData = 0;
	uint32_t ulBefore =0;
	uint32_t ulAfter = 0;
	
	while(1){
		myUtil.setWdtReload();
		if(tick_mSec){
			tick_mSec = false;
			ulData=read();			
			printf("Data = %d\n\r",ulData);
			ulAfter++;
		}
		if(tick_Sec){
			tick_Sec = false;
			printf("------Delta = %d\n\r", ulAfter - ulBefore);
			ulBefore = ulAfter;
		}
	}

}

