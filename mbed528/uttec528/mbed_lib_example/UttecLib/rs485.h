#ifndef __RS485_H__
#define __RS485_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "DimmerRf.h"
typedef enum{
	eUart = 0,
	eRsDown = 1,
	eRsUp = 2
} rs485Channel_t;

typedef union
{
	uint16_t u16;
	uint8_t u8[2];
} UU16;

typedef enum{
	sof='{',
	eof='}',
	FLENGTH=70,
	De485FrameLength=68
} rs485_Frame_t;

typedef enum{
	tsof='t',
	teof='T',
	tFLENGTH=7,
	tDe485FrameLength=4
} testFrame_t;

typedef struct{
	bool flag;
	bool start;
	uint8_t count;
} rs485Status_t;

typedef struct{
	rfFrame_t sRf;
	uint16_t sum;
} rs485withRf_t;

typedef enum{
	eUttecFrame = '{',
	eTestFrame = 't',
	eAsciiFrame = ':',
	eRtuFrame = 'r'
} frame485_t;

typedef enum{
	e485NoType = 0,
	e485Uttec = 1,
	e485Ascii = 2,
	e485Rtu = 3,
	e485Test = 4
} type485_t; 

typedef enum{
	e485procReady = 0,
	e485procStart = 1,
	e485procDo = 2,
	e485procEnd = 3
} type485Proc_t;

class rs485
{
private:
	static rs485Status_t m_status;
//	static rs485Status_t m_tstatus;

	static rfFrame_t m_485Rx;
	static uint8_t m485Data[FLENGTH];
	static uint8_t mtData[tFLENGTH];
	static bool m_485Done;
	static bool m_testDone;
	static Serial* pMySer;

	void reformRsFrame(uint8_t);

	bool parse485Data(uint8_t);
	bool parseTestData(uint8_t);

	bool reform485toRx(uint8_t*);
	void changeChannel(rs485Channel_t);
	void sendByUart();
	void sendByDown();
	void sendByUp();
	
public:
	rs485(Serial*);
	bool isAnyDone();
	bool isTestDone();
	bool is485Done();
	void clear485Done();
	void clearTestDone();
	void task485(rfFrame_t*);
//	void send485(rfFrame_t*);
	void send485(rfFrame_t*,rs485Channel_t);
	void set485Done();	//for test only

	rfFrame_t* return485Buf();
	uint32_t returnTestData();

};
#endif
