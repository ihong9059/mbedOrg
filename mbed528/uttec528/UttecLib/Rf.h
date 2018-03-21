#ifndef __RF_H__
#define __RF_H__

#include <stdint.h>

#include <stdbool.h>

#include "Flash.h"
//#include "Led.h"
#include "Random.h"
#include "UttecUtil.h"

#include "radio_config.h"

class Rf
{	
private: 
	static uint8_t m_ucNewGeneration;
	static bool TxEndFlag;
	static bool RxEndFlag;
	static bool SxRxEndFlag;

	static RfRxMode_t m_RfRxTx;
	static uint8_t m_HoppingCount;
	static channel_t m_Ch;

	static rfFrame_t m_SxRxFrame;	//for Sx1276Test

	static rfFrame_t m_RxFrame;
	static rfFrame_t m_TxFrame;

	void setRadio(channel_t);
	uint32_t receiveAction();
	void clearRfMode();
	void sendInternal();
	void setRfMode(RfRxMode_t);

public:
	static uint32_t m_blockingTime;
	uint32_t m_test;

	Rf();
	void initRf(channel_t* pCh);
	void sendRf(const rfFrame_t* ucpTx);
	void sendSxRf(const rfFrame_t* ucpTx);
	bool isRxDone();
	bool isSxRxDone();
	void clearRxFlag();
	void clearSxRxFlag();
	rfFrame_t* returnRxBuf();
	rfFrame_t* returnSxRxBuf();

	Rf* returnRfClass();
	const channel_t* returnRadio();
	void changeGroup(uint32_t uiGroup);
	void changeGroup(uint32_t uiGroup,TxDbm_t);
	void changeGroup(channel_t sChannel);
	void isOkChannel();
	void setHoppingCount();
	bool isOkCheckSum(uint16_t myAddr);
	void setRxFlag();
	
	void setSxRxFlag();
	channel_t getRfFactor();
};


#endif

