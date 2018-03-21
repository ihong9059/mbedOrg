#ifndef __Rcu_H__
#define __Rcu_H__

#include <stdint.h>
#include "mbed.h"
 
#define DeUtRcuHeader	6700
#define DeRcuHeader	13430
#define DeRcu_1	2200
#define DeRcu_0	1100
#define DeRcuRange 	400

#define DeUttecRcuCode 0xa10c
#define DeUtCode 0xaa

typedef enum{
	rPower = 0xd7, rUp = 0x9b, rDown = 0xbb, rRight = 0x87, rTv = 0x77,
	rLeft = 0xb7, rMenu = 0x07, rScan = 0x3b, rMute = 0xdb,
	rOne = 0xeb, rTwo = 0x6b, rThree = 0xab, rFour = 0x2b,
	rFive = 0xcb, rSix = 0x4b, rSeven = 0x8b, rEight = 0x0b,
	rNine = 0xf3, rZero = 0x73, rPlus = 0x67, rMinus = 0xa7
} rcuValue_t;

typedef union{
	struct{
		uint8_t utCode;
		uint8_t pid;
		uint16_t gid;
		uint8_t cmd;
		uint8_t sub;
		uint16_t time;
	} Data_t;
	uint64_t ulData;
} rcuFrame0_t;

typedef union{
	struct{
		uint8_t high;
		uint8_t low;
		uint16_t dTime;
		uint8_t tbd[2];
		uint16_t crc;
	} Data_t;
	uint64_t ulData;
} rcuFrame1_t;

typedef struct{
	rcuFrame0_t rcu0;
	rcuFrame1_t rcu1;
} rcuFrame_t;

typedef struct{
	bool bRcuEnable;
	bool bRcuFlag;
	bool bEnd;
	bool bStart;
	uint32_t m_ulRcuTimeTimeOut;
	uint32_t m_bitCount;
	uint32_t m_RcuData;
	uint64_t m_UtRcuData;
} Rcu_t;

class Rcu
{
private: 
	void setClock();
	void setOne();
	void setZero();
	void setHeader();
	void setUtHeader();
	bool isCrcOk(rcuFrame_t*);
	static rcuFrame_t m_utRcu;
	static bool m_utFlag;

public:
	Rcu();
	bool isRcuReady();
	bool isUtRcuReady();
	bool isUttecCode();
	void  clearRcuFlag();
	void  clearUtRcuFlag();
	uint16_t returnVendorCode();
	uint8_t returnRcuCode();
	rcuFrame_t* returnUtRcuCode();
	void procRcu(rcuValue_t);
	void procUtRcu(rcuFrame_t*);
	void setRcuPwm();
	void generateRcuSignal();
	void generateUtRcuSignal(uint64_t);
	void testRcuGenerate();
	uint8_t forTest(uint8_t);
};


#endif

