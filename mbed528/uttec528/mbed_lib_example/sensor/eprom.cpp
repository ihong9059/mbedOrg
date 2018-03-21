#include <stdio.h>
#include <string.h>

#include "mbed.h"
#include "UttecUtil.h"

#include "eprom.h"

I2C at24(p7, p30);		//sda, scl

eprom::eprom(){
	m_addr = DeEpromAddr;
//	printf("bh1750()\n\r");
}

eprom::eprom(int iAddr) {
	m_addr = iAddr;
	printf("bh1750(int m_addr=%02x)\n\r",iAddr);
}

uint16_t eprom::swapByte(uint16_t uiData){
	ChangeByte_t uu16;
	uint8_t ucTemp;
	uu16.u16 = uiData;
	ucTemp = uu16.u8[0];
	uu16.u8[0] = uu16.u8[1];
	uu16.u8[1] = ucTemp;
	return uu16.u16;
}

char eprom::readByte02(int iAddr)
{
	ep02ByteWrite_t epAddrData;
	epAddrData.addr = iAddr;
	if(at24.write(DeEpromAddr, (char*)&epAddrData, 1, false)){
		printf("Error i2c write: \n\r");
	}
	wait(DeWriteDelay);
	if(at24.read(DeEpromAddr,&epAddrData.data,1,false)){
		printf("Error i2c read: \n\r");
	}
	return epAddrData.data;
}

int eprom::readPage02(int iAddr, char* pData){
	ep02PageWrite_t epAddrData;
	epAddrData.addr = iAddr;
	int iResult = 0;
	if(at24.write(DeEpromAddr, (char*)&epAddrData, 1, false)){
		printf("Error i2c write: \n\r");
	}
	wait(DeWriteDelay);
	if(at24.read(DeEpromAddr,pData,16,false)){
		printf("Error i2c writeByte: \n\r");
		iResult = 1;
	}
	else{
		wait(DeReadDelay);
	}
	return iResult;
}

int eprom::writeByte02(int iAddr, char cData)
{
	int iResult = 0;
	ep02ByteWrite_t epAddrData;
	epAddrData.addr = iAddr;
	epAddrData.data = cData;
	if(at24.write(DeEpromAddr,(char*)&epAddrData,2,false)){
		printf("Error i2c writeByte: \n\r");
		iResult = 1;
	}
	else{
		wait(DeWriteDelay);
	}
	return iResult;
}

int eprom::writePage02(int iAddr, char* pData){
	int iResult = 0;
	ep02PageWrite_t epAddrData;
	epAddrData.addr = iAddr;
	memcpy(epAddrData.data, pData, 16);
	if(at24.write(DeEpromAddr,(char*)&epAddrData,
		sizeof(epAddrData),false)){
		printf("Error i2c writeByte: \n\r");
			iResult = 1;
	}
	else{
		wait(DePageWriteDelay);
	}
	return iResult;
}
//------------------------------------------- 256
char eprom::readByte256(uint16_t iAddr)
{
	epByteWrite_t epAddrData;
	epAddrData.addr = swapByte(iAddr);
	if(at24.write(DeEpromAddr, (char*)&epAddrData, 2, false)){
		printf("Error i2c readByte256 write: \n\r");
	}
	wait(De256WriteDelay);
	if(at24.read(DeEpromAddr,&epAddrData.data,1,false)){
		printf("Error i2c read: \n\r");
	}
	return epAddrData.data;
}

int eprom::readPage256(uint16_t iAddr, char* pData){
	epPageWrite_t epAddrData;
	epAddrData.addr = swapByte(iAddr);
	int iResult = 0;
	if(at24.write(DeEpromAddr, (char*)&epAddrData, 2, false)){
		printf("Error i2c readPage256 write: \n\r");
		iResult = 1;
	}
	wait(De256WriteDelay);
	
	if(at24.read(DeEpromAddr,pData,16,false))
		printf("Error i2c readPage256: \n\r");
	else{
		wait(DeReadDelay);
	}
	return iResult;
}

int eprom::writeByte256(uint16_t iAddr, char cData)
{
	epByteWrite_t epAddrData;
	epAddrData.addr = swapByte(iAddr);
	epAddrData.data = cData;
	int iResult = 0;
	if(at24.write(DeEpromAddr,(char*)&epAddrData,3,false)){
		printf("Error i2c writeByte256: %d\n\r",iAddr);
		iResult = 1;
	}
	wait(De256WriteDelay);
	return iResult;
}

int eprom::writePage256(uint16_t iAddr, char* pData){
	epPageWrite_t epAddrData;
	epAddrData.addr = swapByte(iAddr);
	int iResult = 0;
	memcpy(epAddrData.data, pData, 16);
	
	if(at24.write(DeEpromAddr,(char*)&epAddrData,
			sizeof(epAddrData),false)){
		printf("Error i2c writePage256: \n\r");
		iResult = 1;
	}
	else{
		wait(De256PageWriteDelay);
	}
	return iResult;
}

void eprom::testByte(uint16_t uiStart, uint16_t uiEnd){
	UttecUtil myUtil;
	char cResult, cStart;
	bool bResult = true;
	for(int i = uiStart; i<uiEnd; i++){
		myUtil.setWdtReload();
		cStart = i + 10;
		writeByte02(i, cStart);
		cResult = readByte02(i);
		if(cResult != cStart){
			printf("Error %d, s:%d, r:%d\n\r", i, cStart, cResult);
			bResult = false;
		}
//		printf("Step:%d\n\r",i);
//		wait(0.2);
	}
	if(bResult) printf("Ok Success\n\r");
	else printf("There are some errors\n\r");
}

void eprom::testPage(uint16_t uiStart, uint16_t uiEnd){
	UttecUtil myUtil;
	char cdResult[16], cdStart[16];
	bool bResult = true;
	for(int i = 0; i<16; i++) cdStart[i] = 'a' + i;
	for(int i = uiStart; i<uiEnd; i++){
		myUtil.setWdtReload();
		writePage02(i*16, cdStart);
		readPage02(i*16,cdResult);
		for(int j = 0; j<16; j++)
			if(cdStart[j] != cdResult[j]){
				printf("Error %d, s:%d, r:%d\n\r", i, 
					cdStart[j], cdResult[j]);
				bResult = false;
			}
		printf("Step:%d\n\r",i);
//		wait(0.2);
	}
	if(bResult) printf("Ok Success\n\r");
	else printf("There are some errors\n\r");
}

void eprom::test256Byte(uint16_t uiStart, uint16_t uiEnd){
	UttecUtil myUtil;
	char cResult, cStart;
	bool bResult = true;
	for(int i = uiStart; i<uiEnd; i++){
		myUtil.setWdtReload();
		cStart = i + 10;
		writeByte256(i, cStart);
		cResult = readByte256(i);
		if(cResult != cStart){
			printf("Error %d, s:%d, r:%d\n\r", i, cStart, cResult);
			bResult = false;
		}
//		printf("Step:%d\n\r",i);
//		wait(0.2);
	}
	if(bResult) printf("Ok Success\n\r");
	else printf("There are some errors\n\r");
}


void eprom::test256Page(uint16_t uiStart, uint16_t uiEnd){
	UttecUtil myUtil;
	char cdResult[16], cdStart[16];
	bool bResult = true;
	for(int i = 0; i<16; i++) cdStart[i] = 'a' + i;
	for(int i = uiStart; i<uiEnd; i++){
		myUtil.setWdtReload();
		writePage256(i*16, cdStart);
		readPage256(i*16,cdResult);
		for(int j = 0; j<16; j++)
			if(cdStart[j] != cdResult[j]){
				printf("Error %d, s:%d, r:%d\n\r", i, 
					cdStart[j], cdResult[j]);
				bResult = false;
			}
		printf("Step:%d\n\r",i);
//		wait(0.2);
	}
	if(bResult) printf("Ok Success\n\r");
	else printf("There are some errors\n\r");
}

