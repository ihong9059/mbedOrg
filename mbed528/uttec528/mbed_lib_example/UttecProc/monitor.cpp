#include <stdio.h>
#include <string.h>

#include "monitor.h"

monitor_t monitor::m_monitor = {0,};

Timeout monitorT;
Timeout monitorAckT; 

static bool bAckTimeoutFlag = false;

static bool bNextTimeoutFlag = false;

static void timeoutAck(){
	printf("timeoutAck\n\r");
	bAckTimeoutFlag = true;
}

static void timeoutNext(){
	printf("timeoutNext\n\r");
	bNextTimeoutFlag = true;
}

monitor::monitor(){
}

void monitor::initMonitor(uint32_t uiAck, uint32_t uiNext){
	memset(&m_monitor, 0, sizeof(monitor_t));
	m_monitor.ackTime = uiAck;
	m_monitor.nextTime = uiNext;
	monitorT.attach(&timeoutAck, m_monitor.ackTime);
	monitorT.attach(&timeoutNext, m_monitor.nextTime);	
}

void monitor::requestForAlive(uint8_t ucMember){
	clearAckFlag();
	printf("Send request for alive ack by Rf\n\r");
}

void monitor::clearAckFlag(){
	monitorT.attach(&timeoutAck, m_monitor.ackTime);
	bAckTimeoutFlag = false;
}

void monitor::clearNextFlag(){
	monitorT.attach(&timeoutNext, m_monitor.nextTime);
	bNextTimeoutFlag = false;
	if(bAckTimeoutFlag){
		m_monitor.member[m_monitor.nextNum] = eMNoAck;	
	}
	clearAckFlag();
}

bool monitor::isMonitor(){
	return bAckTimeoutFlag;
}

void monitor::askAlive(){
	printf("Monitor ask to %dth member, wheter alive\n\r",
		m_monitor.nextNum);
	printf("and process previous Ack result\n\\r");
}

void monitor::procMonitor(){
	UttecUtil myUtil;
	if(bNextTimeoutFlag){
		clearNextFlag();
		askAlive();
		m_monitor.nextNum++;
		m_monitor.nextNum %= m_monitor.totalNum;
	}
}

void monitor::receivedAliveAck(uint8_t ucNum,
		eMonitor_t eResult){
	UttecUtil myUtil;
	Flash flash;
	clearAckFlag();
	if(eResult)
		m_monitor.member[ucNum] = eResult;	
}

void monitor::reformMonitor(){
	uint8_t ucTemp = 0;
	for(int i = 0; i<24; i++) m_monitor.memBit[i] = 0;
	
	for(int j = 0; j<24; j++){
		for(int i = 0; i<4; i++){
			ucTemp = 0;
			ucTemp = m_monitor.member[j*4+i];
			ucTemp <<= i*2;
			m_monitor.memBit[j] |= ucTemp;
		}
	}	
	
	for(int i = 0; i<24; i++)
		printf("%02x,",m_monitor.memBit[i]);
	
	printf(" end of reformMonitor\n\r");
}

void monitor::setTestData(){
	for(int i = 0; i<96; i++) m_monitor.member[i] = 0;
	m_monitor.member[0] = eMLedOut;
	m_monitor.member[95] = eMLedOut;
//	printf("myRng = %d\n\r", rand()%96);
	reformMonitor();
	for(int i = 0; i<3; i++){
		printf("\n\r");
		for(int j = 0; j<8; j++)
			printf("%02x,",m_monitor.memBit[j+i*8]);
	}
	printf("End of bit \n\r");
	
	for(int i = 0; i<12; i++){
		printf("\n\r");
		for(int j = 0; j<8; j++)
			printf("%02x,", m_monitor.member[j+i*8]);
	}
	printf("End of setTestData\n\r");
}
