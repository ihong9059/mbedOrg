#include <stdio.h>
#include <string.h>

#include "mbed.h"

#include "dac.h"

int dac::m_addr = 0;
bool dac::m_enableFlag = true;

I2C dacI2c(p7, p30);		//sda, scl

typedef union
{
	uint16_t u16;
	uint8_t u8[2];
} UU16;

dac::dac(){
	m_addr = DeDacAddr;
//	printf("bh1750()\n\r");
}

void dac::setMode(MCP_t eMode){	
	printf("setMode tbd\r\n");
}

void dac::writeDac(float fDac)
{
	UU16 uDac;
	if(fDac>=1.0) fDac = 1.0-0.0001;
	
	uDac.u16= fDac*4096;
	uint8_t ucdTemp[2];
	ucdTemp[0] = uDac.u8[1];
	ucdTemp[1] = uDac.u8[0];
	
	if(dacI2c.write(m_addr,(char*)ucdTemp,2,false)){
		printf("Error i2c writeByte: \n\r");		
		m_enableFlag = false;
	}
	else m_enableFlag = true;
}

void dac::testDac(){
	static uint32_t ulCount = 0;
	float fTest = 0.0;	
	for(int i = 0; i< 1000; i++){
		fTest += 0.001;
		writeDac(fTest);
		wait(0.002);
	}
	printf("End of testDac:%d \r\n", ulCount++);
}
