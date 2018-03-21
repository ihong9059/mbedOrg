#include <stdio.h>
#include <string.h>

#include "comErr.h"

comErr_t comErr::m_comErr = {0,};

Timeout comErrT;

static bool bComErrTimeoutFlag = false;

static void setComErrTimeout(){
	printf("setComErrTimeout\n\r");
	bComErrTimeoutFlag = true;
}

comErr::comErr(){
}

void comErr::initComErr(uint32_t uiTime){
	memset(&m_comErr, 0, sizeof(comErr_t));
	m_comErr.timeout = uiTime;
	comErrT.attach(&setComErrTimeout, m_comErr.timeout);
}

void comErr::setComErr(uint16_t uiGid, uint8_t ucLevel){
	uint8_t ucId = uiGid%10;
	m_comErr.element[ucId].id = ucId;
	m_comErr.element[ucId].level = ucLevel;
	m_comErr.element[ucId].gid = uiGid;
}

void comErr::clearFlag(){
	comErrT.attach(&setComErrTimeout, m_comErr.timeout);
	bComErrTimeoutFlag = false;
}

bool comErr::isComErr(){
	return bComErrTimeoutFlag;
}

void comErr::procComErrTimeout(DimmerRf* pRf){
	UttecUtil myUtil;
	Flash flash;
	rfFrame_t* myFrame = &flash.getFlashFrame()->rfFrame;
	printf("Send Monitor Timeout Level by Rf\n\r");
	for(int i = 0; i<10; i++){
		if(m_comErr.element[i].id){
			printf("Send recover = %d\n\r",
				m_comErr.element[i].gid);
			pRf->sendRf(myFrame);
		}
		wait(0.1);
	}
}

void comErr::recoverComErrTimeout(DimmerRf* pRf){
	UttecUtil myUtil;
	Flash flash;
	rfFrame_t* myFrame = &flash.getFlashFrame()->rfFrame;
	printf("Send Monitor Timeout Level by Rf\n\r");
	for(int i = 0; i<10; i++){
		if(m_comErr.element[i].id){
			printf("Send recover = %d\n\r",
				m_comErr.element[i].gid);
			pRf->sendRf(myFrame);
		}
		wait(0.1);
	}	
}


