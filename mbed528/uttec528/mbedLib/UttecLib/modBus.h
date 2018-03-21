#ifndef __UTTECMODBUS_H__
#define __UTTECMODBUS_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "Flash.h"

typedef enum{
	eWriteDim = 0x0600,
	eEmergeDim = 0x0610,
	eEmergeTime = 0x0620,
	eReadDim = 0x0300
} rtuCmd_t;

typedef enum{
	eReadReg = 0x06,
	eWriteMultiReg = 0x16,
} asciiCmd_t;

typedef enum{
	sofModA=':',
	crModA = 13,
	eofModA= 10,
	modA_FLENGTH=70,
	DeModA_FrameLength=68
} modAscii_t;

typedef struct{
	bool flag;
	bool start;
	uint8_t count;
} modAsciiStatus_t;

typedef struct{
	bool flag;
	bool start;
	uint8_t count;
} modRtuStatus_t;

typedef struct{
	uint8_t* pLrc;
	uint8_t len;
	uint8_t lrc;
} Lrc_t;

typedef struct{
	uint8_t* pCrc;
	uint8_t len;
	uint16_t crc;
} Crc_t;

typedef struct{
	uint8_t slave;
	uint8_t function;
	uint8_t data[modA_FLENGTH];
	uint16_t crc;
	uint16_t lrc;
	uint16_t lengthForData;	//For Ascii
	uint16_t length;				//Data Length
	uint16_t lengthForLrc;
	uint16_t lengthForCrc;
	uint16_t lengthForSend;
	uint16_t crc_sum;
	uint16_t lrc_sum;
} modRtu_Frame_t;

class modBus
{
private:
	modRtu_Frame_t m_modRFrame;

	static modAsciiStatus_t m_status;
	static modRtuStatus_t m_Rstatus;
	static Serial* pMySer;

	static bool m_modAsciiDone;
	static uint8_t mModAsciiRaw[modA_FLENGTH];

	static bool m_modRtuDone;
	static uint8_t mModRtuRaw[modA_FLENGTH];
	
	uint8_t mModAsciiTx[modA_FLENGTH];
	uint8_t mModRtuTx[modA_FLENGTH];

	void setModAData(uint8_t);
	bool isLrcOk(Lrc_t);
	bool isCrcOk(Crc_t);

public:	
	uint8_t m_modData[modA_FLENGTH];

	modBus(Serial*);
	modBus();

	uint16_t ModRTU_CRC(uint8_t*, int);
	bool parseModAscii(uint8_t);
	void parseModRtu(uint8_t);

	uint8_t* generateModAscii(modRtu_Frame_t*);
	uint8_t* generateModRtu(modRtu_Frame_t*);

	bool isMySlave();
	bool isRtuDone();
	bool isAsciiDone();

	void setModRData();
	void clearRtuDone();
	void clearAsciiDone();

	rfFrame_t* returnModRtu();
	rfFrame_t* returnModAscii();
};

#endif
