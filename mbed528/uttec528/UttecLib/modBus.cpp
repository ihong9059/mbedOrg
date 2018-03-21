#include <stdio.h>
#include <string.h>

#include "modBus.h"
#include "UttecUtil.h"

static DigitalOut downCtr(p13);	//FOR DOWNLOAD TXCTR
static DigitalOut upCtr(p19);	//For upload TXCtr

Serial* modBus::pMySer = NULL;
static UttecUtil myUtil;

bool modBus::m_modAsciiDone = false;
uint8_t modBus::mModAsciiRaw[] = {0,};

bool modBus::m_modRtuDone = false;
uint8_t modBus::mModRtuRaw[] = {0,};

modAsciiStatus_t modBus::m_status = {0,};
modRtuStatus_t modBus::m_Rstatus = {0,};

modBus::modBus(Serial* pSer){
	pMySer = pSer;
	downCtr = 1;
	upCtr = 1;	
}

modBus::modBus(){
}

uint8_t changeAscii2Hex(uint16_t uiData){
	UttecUtil myUtil;
	ChangeByte_t uChange;
	uint8_t ucTemp;
	uChange.u16 = uiData;
	ucTemp = myUtil.Hex2Dec(uChange.u8[0]);
	ucTemp = ucTemp*16 + myUtil.Hex2Dec(uChange.u8[1]);
	return ucTemp;
}


bool modBus::isLrcOk(Lrc_t sLrc){
	uint8_t ucTotal = 0;
	for(int i = 0; i<sLrc.len; i++){
		ucTotal += sLrc.pLrc[i];
	}
	ucTotal = ~ucTotal; ucTotal++;
	if(ucTotal == sLrc.lrc) return true;
	else return false;
}

void modBus::setModAData(uint8_t ucLen){
	printf("Ok setModAData\n\r");
	
	m_modRFrame.lengthForLrc = ucLen;
	m_modRFrame.length = ucLen/2;	
	m_modRFrame.lengthForData = m_modRFrame.length -2;
	uint16_t* uipTemp = (uint16_t*)mModAsciiRaw;
	
	m_modRFrame.slave = changeAscii2Hex(*uipTemp++);
	m_modRFrame.function = changeAscii2Hex(*uipTemp++);
	for(int i = 0; i<m_modRFrame.lengthForData; i++)
		m_modRFrame.data[i] = changeAscii2Hex(*uipTemp++);
	m_modRFrame.lrc = changeAscii2Hex(*uipTemp++);
	
	Lrc_t sLrc = 
		{mModAsciiRaw, m_modRFrame.lengthForLrc, m_modRFrame.lrc};
	
	if(isLrcOk(sLrc)&&isMySlave()){
		m_modAsciiDone = true;
		printf("Lrc Ok\n\r");
	}
	else printf("Lrc Error\n\r");		
}

bool modBus::isCrcOk(Crc_t sCrc){
	uint16_t uiResult = ModRTU_CRC(sCrc.pCrc, sCrc.len);
	printf("uiResult = %02x\n\r", uiResult);
	printf("lrc = %02x\n\r", sCrc.crc);
	
	if(uiResult == sCrc.crc) return true;
	else return false;
}

bool modBus::isMySlave(){
	if(m_modRFrame.slave) return true;
	else return false;
}

void modBus::setModRData(){
	printf("Ok setModAData\n\r");
	printf("len = %d\n\r", m_status.count);
	
	m_status.start = false;
	uint8_t ucLen = m_status.count;
	
	m_modRFrame.length = ucLen - 4; //slave, function, crc(2)
	
	uint8_t* ucpDst = (uint8_t*)&m_modRFrame;
	uint16_t* uipCrc = (uint16_t*)(mModRtuRaw + ucLen -2);
	
	for(int i = 0; i< ucLen; i++){
		*ucpDst++ = mModRtuRaw[i];
	}
	printf("slave = %02x\n\r", m_modRFrame.slave);
	printf("func = %02x\n\r", m_modRFrame.function);
	
	for(int i = 0; i<ucLen - 2; i++)
		printf("Data[%d]: %02x\n\r",i, m_modRFrame.data[i]);
	
	printf("Crc = %02x\n\r", *uipCrc);
	Crc_t sCrc = {mModRtuRaw, 6, *uipCrc};
	
	if(isCrcOk(sCrc)&&isMySlave()){
		m_modRtuDone = true;
		printf("Ok Crc\n\r");
	}
	else printf("Crc Error\n\r");	
}

static bool bRtuTimeoutFlag = true;

static void timeOutRtu(){
	bRtuTimeoutFlag = true;
}

void modBus::clearAsciiDone(){
	m_modAsciiDone = false;
}

bool modBus::isAsciiDone(){
	if(pMySer->readable()){
		parseModAscii(getchar());
//		putchar('.');
	}
	return m_modAsciiDone;
}

void modBus::clearRtuDone(){
	m_modRtuDone = false;
}

bool modBus::isRtuDone(){
	if(pMySer->readable()) parseModRtu(getchar());
	if(m_status.start && bRtuTimeoutFlag) setModRData();
	else m_modRtuDone = false;
	return m_modRtuDone;
}

Timeout rtuT;
void modBus::parseModRtu(uint8_t ucChar)
{
	static uint8_t* ucpModData = mModRtuRaw;	
	rtuT.attach(&timeOutRtu, 0.5);
	
	if(bRtuTimeoutFlag&&(!m_status.start)){
		ucpModData = mModRtuRaw;
		m_status.flag=false;
		m_status.count=0;
		m_status.start=true;
		m_modRtuDone=false;
		bRtuTimeoutFlag = false;
		*ucpModData++ = ucChar;
		printf("%02x\n\r",ucChar);
		m_status.count++;
	}
	else{
		bRtuTimeoutFlag = false;
		printf("%02x\n\r",ucChar);
		*ucpModData++ = ucChar;
		m_status.count++;
	}
}

bool modBus::parseModAscii(uint8_t ucChar)
{
	static uint8_t* ucpModAData = mModAsciiRaw;
//	putchar('~');
	if((ucChar==sofModA)&&(!m_status.start)){
		ucpModAData = mModAsciiRaw;
		m_status.flag=false;
		m_status.count=0;
		m_status.start=true;
		m_modAsciiDone=false;
//		putchar('H');
	}
	else if(ucChar==eofModA){
		printf("eofModA\n\r");
		ucpModAData--;
		if((m_status.start)&&(*ucpModAData==crModA)){
			/*
			printf("\n\r");
			for(int i=0;i<sizeof(mtData);i++) putchar(mtData[i]);
			printf("\n\r");
			*/
			printf("Length = %d\n\r", m_status.count);
			m_modRFrame.lengthForLrc = m_status.count -3;
			setModAData(m_modRFrame.lengthForLrc);
			m_status.flag=true;
			m_status.count=0;
			m_status.start=false;
		}
		else{
			m_status.flag=false;
			m_status.count=0;
			m_status.start=false;
			m_modAsciiDone=false;
		}
	}
	else if(m_status.count>modA_FLENGTH){
			m_status.flag=false;
			m_status.count=0;
			m_status.start=false;
			m_modAsciiDone=false;
	}
	else{
		*ucpModAData++ = ucChar;
		m_status.count++;
	}
	return m_modAsciiDone;
}

uint16_t modBus::ModRTU_CRC(uint8_t* buf, int len)
{
  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  return crc;
}

uint8_t* modBus::generateModAscii(modRtu_Frame_t* pFrame){
	uint8_t* ucpSend = mModAsciiTx;
	uint8_t* ucpSrc = (uint8_t*)pFrame;
	
	pFrame->lengthForLrc = (pFrame->length+2)*2;
	pFrame->lengthForSend = pFrame->lengthForLrc + 5;
	
	printf("lengthForLrc = %d\n\r", pFrame->lengthForLrc);
	printf("lengthForSend = %d\n\r", pFrame->lengthForSend);
	
	*ucpSend++ = ':';
	for(int i = 0; i<pFrame->length+2; i++){
		sprintf((char*)ucpSend, "%02x", *ucpSrc++);
		ucpSend += 2;		
	}
	uint8_t ucTotal = 0;
	for(int i = 1; i<pFrame->lengthForLrc+1; i++){
		printf("%c",mModAsciiTx[i]);
		ucTotal += mModAsciiTx[i];
	}
	printf("\n\r");
	ucTotal = ~ucTotal;
	ucTotal++;
	
	sprintf((char*)ucpSend, "%02x\r\n", ucTotal);
	printf("End of Generation: %02x\n\r", ucTotal);
	return mModAsciiTx;
}

uint8_t* modBus::generateModRtu(modRtu_Frame_t* pFrame){
	uint8_t* ucpSend = mModRtuTx;
	uint8_t* ucpSrc = (uint8_t*)pFrame;
	
	Serial rtuSerial(p12, p11);
	rtuSerial.baud(115200);
	
	pFrame->lengthForCrc = pFrame->length + 2;
	memcpy(mModRtuTx, (uint8_t*)pFrame, pFrame->lengthForCrc);
	pFrame->crc = ModRTU_CRC(mModRtuTx, pFrame->lengthForCrc);
	
	uint16_t* uipDst = (uint16_t*)(ucpSend + pFrame->lengthForCrc);
	*uipDst  = pFrame->crc;
	//Send Frame
	
	pFrame->lengthForSend = pFrame->lengthForCrc + 2;
//	for(int i = 0; i<2; i++) rtuSerial.putc('U'); //for test only
	/*
	*/
	for(int i = 0; i< pFrame->lengthForSend; i++)
		rtuSerial.putc(mModRtuTx[i]);
//		rtuSerial.printf("%02x\n\r", mModRtuTx[i]);
	
	Serial uart(p9, p11);
	uart.baud(115200);
	printf("End of Generation: %02x\n\r", pFrame->crc);
	return mModRtuTx;
}

rfFrame_t* modBus::returnModAscii(){
	Flash myFlash;
	Serial asciiSerial(p12, p11);
	asciiSerial.baud(115200);
	
	rfFrame_t* pFrame = 
		&myFlash.getFlashFrame()->rfFrame;
	uint16_t uiCmd = m_modRFrame.function*256 + m_modRFrame.data[0];
	asciiSerial.printf("uiCmd = %x\n\r", uiCmd);
	
	switch(m_modRFrame.function){
		case eReadReg:
			asciiSerial.printf("For Jungho Project, have to return status\n\r");
			break;
		case eWriteMultiReg:
			asciiSerial.printf("For Jungho Project. have to return ack\n\r");
			asciiSerial.printf("And execute Server Control Command \n\r");
			break;
	}
	Serial uart(p9, p11);
	uart.baud(115200);
	
	return pFrame;
}
rfFrame_t* modBus::returnModRtu(){
	Flash myFlash;
	rfFrame_t* pFrame = 
		&myFlash.getFlashFrame()->rfFrame;

	uint16_t uiCmd = m_modRFrame.function*256 + m_modRFrame.data[0];
	printf("+++++++++++++++++\n\r");
	printf("uiCmd = %x\n\r", uiCmd);
	
	modRtu_Frame_t Frame = m_modRFrame;
	
	switch(uiCmd){
		case eWriteDim:
			printf("Set Dimming Level, leng= %d\n\r",
		Frame.length);
			break;
		case eEmergeDim:
			printf("Set eEmergeDim Level, leng= %d\n\r",
		Frame.length);
			generateModRtu(&Frame);
			break;
		case eEmergeTime:
			printf("Set eEmergeTime \n\r");
			generateModRtu(&Frame);
			break;
		case eReadDim:
			printf("get Dimming Level\n\r");
			Frame.length = 3;
			Frame.data[0] = 0x02; //return dim level cmd
			Frame.data[1] = 0x00; //Null
			Frame.data[2] = 0x32; //return dim level 50
			generateModRtu(&Frame);
			break;
		default:
			break;
	}
	return pFrame;
}


