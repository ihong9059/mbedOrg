#ifndef UTTEC_PYD1788_H
#define UTTEC_PYD1788_H

#include <stdint.h>

#define DeReadCount	28-1
#define DeNextCount	10
#define DeDataLength	10
#define DeMaxPyd 9200
#define DeMinPyd 7000
#define DeSensRate 5
extern bool bRtcFlag;

typedef enum{
	eBitTrigStart=0,
	eBitTrigEnd,
	eBitRead,
	eBitReadEnd,
	eNextStandby
} PydState_t;

typedef struct{
	bool bSensing;
	uint16_t uiLevel;
	uint16_t uiTemperature;
} pydReturn_t;

class Pyd1788
{
private: 
	uint32_t m_cycle;
	uint16_t m_data[DeDataLength];
	uint32_t m_total;
	uint32_t m_port;
	uint8_t m_countRead;
	uint8_t m_countNext;
	uint8_t m_test;
	uint8_t m_test1;
	uint32_t m_dataTotal;
	uint16_t m_dataTemp;
	uint16_t m_channel;
	uint16_t m_prechannel;
	int16_t m_iChannel;
	int16_t m_iTemp;
	uint16_t m_result;
	uint8_t m_senseRate;

	PydState_t m_state;
	pydReturn_t m_return;

	void setPort();
	bool getPort();
	void delayNext();
	void myTrigDelay();
	void myReadDelay();
	void myStrobeDelay();
public:
    Pyd1788(uint32_t io_num);
	pydReturn_t* processPyd1788();
	void testPyd1788();
	void setSenseRate(uint8_t);
	bool read();
	pydReturn_t* getPydValue();
	void clearFlag();
};


#endif

