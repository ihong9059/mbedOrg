#ifndef __UttecUtil_H__
#define __UttecUtil_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "Flash.h"

#define UartTx   p9
#define UartRx   p11

#define DeLampOn 1.0
#define DeLampTtest 0.5
#define DeLampOff 0.0

#define DeFactoryModeTimeout 10 //60Sec
#define DeFactoryChannel 1
typedef enum{
	eFactoryTestMode = 0,
	eFactoryOutMode = 1,
	eFieldMode = 2
} eFactoryMode_t;

typedef struct{
	eFactoryMode_t mode;
	uint16_t timeout;
} UttecFactory_t;

typedef enum{
	eReadySet = 0,
	eWaitGatewayApprove = 1,
	eWaitRfSet = 2,
	eFinishRfSet = 3,
	eTimeoutSet = 4
} setProcedure_t;

typedef enum{
	eUp = 0,
	eDown = 1
} UttecDirection_t;

typedef struct{
	bool forced;
	uint8_t sensorType;	
	float targetValue;
	float nowValue;
} dimFactors_t;

typedef union
{
	uint8_t u8[2];
  uint16_t u16;                        
} ChangeByte_t;

class UttecUtil
{	
private: 
	static dimFactors_t myDimFact;

public:
	static UttecFactory_t m_Factory;

	UttecUtil();
	uint16_t gen_crc16(const uint8_t *data, uint16_t size);
	uint8_t Hex2Dec(uint8_t cHex);

	void setWdt(uint8_t ucTime);
	void setWdtReload();
	void dispSec(rfFrame_t*, bool);
	void testProc(uint8_t , uint32_t );
	void testProc(uint8_t , uint32_t , uint32_t);
	void testProc(uint8_t , uint32_t , float);
	void getDimFactor(dimFactors_t);
	dimFactors_t getDimFactor();
	uint16_t changeBytesInWord(uint16_t);

	bool isMstOrGw(rfFrame_t*);
	bool isMst(rfFrame_t*);
	bool isGw(rfFrame_t*);
	bool isTx(rfFrame_t*);
	bool isSRx(rfFrame_t*);
	bool isRx(rfFrame_t*);
	bool isRpt(rfFrame_t*);
	bool isMyRxTx(rfFrame_t*,rfFrame_t*);
	bool isMyGroup(rfFrame_t*,rfFrame_t*);
	bool isMyAddr(rfFrame_t*,rfFrame_t*);
	void dispRfFactor(rfFrame_t*);
	void dispCmdandSub(char*, char*, rfFrame_t*);
	bool isNotMyGwGroup(rfFrame_t* pSrc, rfFrame_t* pMy);
	bool isFactoryOutMode();
	void alertFaultSet(uint8_t);
	char* dispRxTx(rfFrame_t*);
	char* dispRxTx(uint8_t);
};

#endif

