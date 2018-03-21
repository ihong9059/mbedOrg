#include <stdio.h>
#include <string.h>

#include "mbed.h"

#include "BH1750.h"

I2C i2c(p7, p30);		//sda, scl

bh1750::bh1750(){
	m_addr = DeBH1750Addr;
//	printf("bh1750()\n\r");
}

bh1750::bh1750(int iAddr) {
	m_addr = iAddr;
	printf("bh1750(int m_addr=%02x)\n\r",iAddr);
}

void bh1750::setMode(BH1750_Mode_t eMode){	
	char reg =eMode;
	if(i2c.write(m_addr, &reg, 1, false)){
		printf("\n\r+++++++++++++++++Error i2c write: \n\r");
	}
}

uint16_t bh1750::readBH1750()
{
	char m_sample[2] = {0,};
	uint16_t uiResult = 0;
	
	setMode(BH1750_ONE_TIME_HIGH_RES_MODE);
	
	if(i2c.read(m_addr, m_sample, 2, false)){
		printf("\n\r+++++++++++++Error i2c read: \n\r");
	}
	printf("d0: %d, d1: %d\r\n", m_sample[0], m_sample[1]);	
	uiResult = ((m_sample[0]<<8)|m_sample[1])/1.2;
	printf("Photo=%d\n\r",uiResult);	
	return uiResult;
}
