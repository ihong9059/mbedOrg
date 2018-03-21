
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "Rcu.h"
#include "test.h"

extern test myTest;
static Timer rcuTimer;

InterruptIn rcuEvent(p2);

Rcu_t m_Rcu={true,false,false,false,0,0,0};
rcuFrame_t Rcu::m_utRcu = {0,};
bool Rcu::m_utFlag = false;

PwmOut rcuPwm(p18);

#define DeRcuZero 0.00035
#define DeRcuOne 0.00135
#define DeRcuHeader_0 0.008
#define DeRcuHeader_1 0.005
#define DeUtRcuHeader_0 0.004
#define DeUtRcuHeader_1 0.0025

static Timeout rcuClock;
static bool bRcuTimeout = false;
static void rcuTimeout(){
	bRcuTimeout = true;
}

void Rcu::setOne(){
	rcuPwm = 0.3;
	wait(DeRcuZero);
	rcuPwm = 0.0;
	wait(DeRcuOne);
}

void Rcu::setZero(){
	rcuPwm = 0.3;
	wait(DeRcuZero);
	rcuPwm = 0.0;
	wait(DeRcuZero);
}

void Rcu::setUtHeader(){
	rcuPwm = 0.3;
	wait(DeUtRcuHeader_0);
	rcuPwm = 0.0;
	wait(DeUtRcuHeader_1);
}

void Rcu::setHeader(){
	rcuPwm = 0.3;
	wait(DeRcuHeader_0);
	rcuPwm = 0.0;
	wait(DeRcuHeader_1);
}


void Rcu::generateRcuSignal(){
	uint32_t ulRcu = 0xa10cd7d7;
	setHeader();
	for(int i = 0; i<32; i++){
		if(ulRcu&0x80000000) setOne();
		else setZero();
		ulRcu <<= 1;
	}
	setZero();
}

static void rcuUtCallback()
{
	bool bBitOk=false;
	uint32_t ulDelta;
	
	static int begin, end;
	
	end=rcuTimer.read_us();
	ulDelta = end - begin; 
	begin=rcuTimer.read_us();
	
	if((ulDelta>=(DeUtRcuHeader-DeRcuRange))&&(ulDelta<=(DeUtRcuHeader+DeRcuRange))){
		m_Rcu.bStart=true;
		m_Rcu.m_ulRcuTimeTimeOut=750;
		m_Rcu.m_bitCount=64;
		m_Rcu.m_RcuData=0;
//		putchar('H');
	}
	else if(m_Rcu.bStart&&(m_Rcu.m_bitCount--)){
		if((ulDelta>=(DeRcu_1-DeRcuRange))&&(ulDelta<=(DeRcu_1+DeRcuRange))){
			m_Rcu.m_UtRcuData|=0x0000000000000001; bBitOk=true;
//			putchar('1');
		}
		else if((ulDelta>=(DeRcu_0-DeRcuRange))&&(ulDelta<=(DeRcu_0+DeRcuRange))){
			m_Rcu.m_UtRcuData&=~0x0000000000000001; bBitOk=true;
//			putchar('0');
		}
		if(bBitOk){
			if(!m_Rcu.m_bitCount){
//				printf("End=%x",m_Rcu.m_RcuData);
				m_Rcu.bRcuFlag=true;
				m_Rcu.bStart=m_Rcu.bStart=false;
				m_Rcu.m_ulRcuTimeTimeOut=m_Rcu.m_bitCount=0;
			}
			else m_Rcu.m_UtRcuData<<=1;
		}
		else{
			m_Rcu.bEnd=m_Rcu.bStart=false;
			m_Rcu.m_ulRcuTimeTimeOut=m_Rcu.m_bitCount=0;
		}
	}
	else{
			m_Rcu.bEnd=m_Rcu.bStart=false;
			m_Rcu.m_ulRcuTimeTimeOut=m_Rcu.m_bitCount=0;
	}
/*	
	*/
}
static void rcuCallback()
{
	bool bBitOk=false;
	uint32_t ulDelta;
	
	static int begin, end;
	
	end=rcuTimer.read_us();
	ulDelta = end - begin; 
	begin=rcuTimer.read_us();
	
	if((ulDelta>=(DeRcuHeader-DeRcuRange))&&(ulDelta<=(DeRcuHeader+DeRcuRange))){
		m_Rcu.bStart=true;
		m_Rcu.m_ulRcuTimeTimeOut=750;
		m_Rcu.m_bitCount=32;
		m_Rcu.m_RcuData=0;
//		putchar('H');
	}
	else if(m_Rcu.bStart&&(m_Rcu.m_bitCount--)){
		if((ulDelta>=(DeRcu_1-DeRcuRange))&&(ulDelta<=(DeRcu_1+DeRcuRange))){
			m_Rcu.m_RcuData|=0x00000001; bBitOk=true;
//			putchar('1');
		}
		else if((ulDelta>=(DeRcu_0-DeRcuRange))&&(ulDelta<=(DeRcu_0+DeRcuRange))){
			m_Rcu.m_RcuData&=~0x00000001; bBitOk=true;
//			putchar('0');
		}
		if(bBitOk){
			if(!m_Rcu.m_bitCount){
//				printf("End=%x",m_Rcu.m_RcuData);
				m_Rcu.bRcuFlag=true;
				m_Rcu.bStart=m_Rcu.bStart=false;
				m_Rcu.m_ulRcuTimeTimeOut=m_Rcu.m_bitCount=0;
			}
			else m_Rcu.m_RcuData<<=1;
		}
		else{
			m_Rcu.bEnd=m_Rcu.bStart=false;
			m_Rcu.m_ulRcuTimeTimeOut=m_Rcu.m_bitCount=0;
		}
	}
	else{
			m_Rcu.bEnd=m_Rcu.bStart=false;
			m_Rcu.m_ulRcuTimeTimeOut=m_Rcu.m_bitCount=0;
	}
/*	
	*/
}
void rcuTest(){
}

Rcu::Rcu(){
	m_Rcu.bRcuFlag=true;
	printf("Rcu\n\r");
	rcuTimer.start();
//	rcuEvent.fall(&rcuCallback);
	rcuEvent.fall(&rcuUtCallback);
}
void Rcu::setRcuPwm(){
	rcuPwm.period_us(25);		//set Pwm Freq
	rcuPwm = 0.3;
}

bool Rcu::isRcuReady(){
	return m_Rcu.bRcuFlag;
}

void Rcu::clearRcuFlag(){
	m_Rcu.bRcuFlag=false;
}
uint16_t Rcu::returnVendorCode(){
	uint32_t ulRcu;
	ulRcu=m_Rcu.m_RcuData;
	ulRcu>>=16;
	return ulRcu;
}

void Rcu::generateUtRcuSignal(uint64_t ulRcu){
	setUtHeader();
	for(int i = 0; i<64; i++){
		if(ulRcu&0x8000000000000000) setOne();
		else setZero();
		ulRcu <<= 1;
	}
	setZero();
}

bool Rcu::isCrcOk(rcuFrame_t* pFrame){
	UttecUtil myUtil;
	uint16_t uiResult;
	uiResult = myUtil.gen_crc16((uint8_t*)pFrame, 14);
	if(uiResult == pFrame->rcu1.Data_t.crc) return true;
	else return false;
}

rcuFrame_t* Rcu::returnUtRcuCode(){	
//	static rcuFrame_t sRcu  = {0,};
	rcuFrame_t sTemp;
	static bool bStart = false;
	sTemp.rcu0.ulData	= m_Rcu.m_UtRcuData;	
	if(sTemp.rcu0.Data_t.utCode == DeUtCode){
		m_utRcu = sTemp;
		bStart = true;
	}
	else if(bStart){
		m_utRcu.rcu1.ulData = sTemp.rcu0.ulData;
		bStart = false;
		if(isCrcOk(&m_utRcu)){
			m_utFlag = true;
			printf("Ok Crc\n\r");
		}
		else printf("Error Crc\n\r");
	}
	return &m_utRcu;
}

void Rcu::testRcuGenerate(){
	UttecUtil myUtil;
	
	static bool bOrder = false;
	bOrder = !bOrder;
	rcuFrame_t sRcu;
	uint8_t* ucpData = (uint8_t*)&sRcu;
	
	for(int i = 0; i<sizeof(rcuFrame_t); i++) *ucpData++ = 0;

	sRcu.rcu0.Data_t.utCode = DeUtCode;
	sRcu.rcu0.Data_t.gid = 1;
	ucpData = (uint8_t*)&sRcu;
	sRcu.rcu1.Data_t.crc = myUtil.gen_crc16(ucpData, 14);
	
	if(bOrder)
		generateUtRcuSignal(sRcu.rcu0.ulData);
	else{
		generateUtRcuSignal(sRcu.rcu1.ulData);
//		printf("Send Rcu0: %llx, Rcu1: %llx\n\r", 
//			sRcu.rcu0.ulData,  sRcu.rcu1.ulData);
	}
}

uint8_t Rcu::returnRcuCode(){
	uint32_t ulRcu;
	uint8_t ucRcu0,ucRcu1;
	ulRcu=m_Rcu.m_RcuData;
	ucRcu1=ulRcu>>8;
	ucRcu0=ulRcu;
	ucRcu1=~ucRcu1;
	if(ucRcu1==ucRcu0) printf("Ok=%x\n\r",ucRcu0);
	else printf("Error Rcu Data=%x\n\r",m_Rcu.m_RcuData);
	return ucRcu0;
}
bool Rcu::isUttecCode(){
	if(returnVendorCode()==DeUttecRcuCode) return true;
	else return false;
}
void Rcu::procRcu(rcuValue_t value)
{
	static uint16_t uiNum = 0;
	printf("From procRcu -> ");
	switch(value){
		case rPower: 
			myTest.setTestRfData(); 
		break;
		case rTv: printf("rTv\n\r"); 
			myTest.setTest485Data(); 				
		break;
		case rUp: printf("rUp\n\r"); 
			myTest.setTestBleData(); 
		break;
		case rDown: printf("rDown\n\r"); 
		break;
		case rRight: printf("rRight\n\r"); 
		break;
		case rLeft: printf("rLeft\n\r"); 
		break;
		case rMenu: printf("rMenu\n\r"); 
			uiNum = 0;
		break;
		case rScan: printf("rScan\n\r"); 
			myTest.setTestSxRxData(); 
		break;
		case rMute: printf("rMute\n\r"); 
		break;
		case rOne: printf("rOne\n\r"); 
			uiNum *= 10; uiNum += 1;
		break;
		case rTwo: printf("rTwo\n\r"); 
			uiNum *= 10; uiNum += 2;
		break;
		case rThree: printf("rThree\n\r");
			uiNum *= 10; uiNum += 3;
		break;
		case rFour: printf("rFour\n\r"); 
			uiNum *= 10; uiNum += 4;
		break;
		case rFive: printf("rFive\n\r"); 
			uiNum *= 10; uiNum += 5;
		break;
		case rSix: printf("rSix\n\r"); 
			uiNum *= 10; uiNum += 6;
		break;
		case rSeven: printf("rSeven\n\r"); 
			uiNum *= 10; uiNum += 7;
		break;
		case rEight: printf("rEight\n\r"); 
			uiNum *= 10; uiNum += 8;
		break;
		case rNine: printf("rNine\n\r"); 
			uiNum *= 10; uiNum += 9;
		break;
		case rZero: printf("rZero\n\r"); 
			uiNum *= 10; uiNum += 0;
		break;
		case rPlus: printf("rPlus\n\r"); 
			myTest.setTestReceiveFrameByNum(uiNum);
//			uiNum = 0;
		break;
		case rMinus: printf("rMinus\n\r"); 
		break;
		default: printf("Rcu Error value: %x\n\r", value);			
	}
	uiNum %= 10000;
	printf("Num = %d\n\r", uiNum);
}

bool Rcu::isUtRcuReady(){
	return m_utFlag;
}

void Rcu::clearUtRcuFlag(){
	m_utFlag = false;
}

void Rcu::procUtRcu(rcuFrame_t* sRcu){
	printf("gid = %d\n\r", sRcu->rcu0.Data_t.gid);
}




