#include "mbed.h"

#include "UttecUtil.h"

static Serial mySer(p9,p11);
 
dimFactors_t UttecUtil::myDimFact = {0,};

productType_t UttecUtil::m_product = {0,};

void UttecUtil::setProductType(){
	m_product.rcu = true;
	m_product.rf = true;
	m_product.ble	= false;
	m_product.rs485 = true;
	m_product.sx1276 = true;
}

UttecUtil::UttecUtil(){
}
	
uint16_t UttecUtil::gen_crc16(const uint8_t *data, uint16_t size)
{
	uint16_t out = 0;
	int bits_read = 0, bit_flag;
	uint16_t CRC16=0x8005;

    if(data == NULL)
        return 0;
    while(size > 0)
    {
        bit_flag = out >> 15;
        out <<= 1;    out |= (*data >> bits_read) & 1; // item a) work from the least significant bits
        bits_read++;
        if(bits_read > 7)
        {
            bits_read = 0;            data++;            size--;
        }
        if(bit_flag)       out ^= CRC16;
    }
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;        out <<= 1;        if(bit_flag)	out ^= CRC16;
    }
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }
    return crc;
}

uint8_t UttecUtil::Hex2Dec(uint8_t cHex)
{
	switch(cHex){
		case '0':		case '1':		case '2':		case '3':		case '4':
		case '5':		case '6':		case '7':		case '8':		case '9':
			cHex-='0';	break;
		case 'a':		case 'b':		case 'c':		case 'd':		case 'e':		case 'f':
			cHex=cHex-'a'+10;	break;
		case 'A':		case 'B':		case 'C':		case 'D':		case 'E':		case 'F':
			cHex=cHex-'A'+10;	break;
		default:
			cHex=0x55;
	}
	return cHex;
}

#include <stdio.h>
#include <string.h>
char cdRxTx[5];

char* UttecUtil::dispRxTx(uint8_t ucRxTx){
	memset(cdRxTx,0,5);
	switch(ucRxTx){
		case eRx: sprintf(cdRxTx, "Rx"); break;
		case eTx: sprintf(cdRxTx, "Tx"); break;
		case eSRx: sprintf(cdRxTx, "SRx"); break;
		case eRpt: sprintf(cdRxTx, "Rpt"); break;
		case eGW: sprintf(cdRxTx, "GW"); break;
		case eMst: sprintf(cdRxTx, "Mst"); break;
		case eReserved: sprintf(cdRxTx, "TBD"); break;
		default: printf("RxTx Error value %d\n\r", ucRxTx);
	}
	return cdRxTx;
}

char* UttecUtil::dispRxTx(rfFrame_t* pFrame){
	memset(cdRxTx,0,5);	
	switch(pFrame->MyAddr.RxTx.iRxTx){
		case eRx:
			sprintf(cdRxTx,"Rx");
			break;
		case eTx:
			sprintf(cdRxTx,"Tx");
			break;
		case eSRx:
			sprintf(cdRxTx,"SRx");
			break;
		case eRpt:
			sprintf(cdRxTx,"Rpt");
			break;
		case eGW:
			sprintf(cdRxTx,"Gw");
			break;
		case eMst:
			sprintf(cdRxTx,"Mst");
			break;
		default:
			break;
	}
	return cdRxTx;
}

char* dispSensor(uint8_t ucSensor){
	memset(cdRxTx,0,5);
	switch(ucSensor){
		case eNoSensor: sprintf(cdRxTx, "No"); break;
		case ePir: sprintf(cdRxTx, "Pir"); break;
		case eMicroWave: sprintf(cdRxTx, "Mw"); break;
		case eDayLight: sprintf(cdRxTx, "Day"); break;
		case eUtraSonic: sprintf(cdRxTx, "Snx"); break;
		case eHyBrid: sprintf(cdRxTx, "Hyb"); break;
		case eVolume: sprintf(cdRxTx, "Vol"); break;
		case eTestMode: sprintf(cdRxTx, "Test"); break;
		default: sprintf(cdRxTx, "sErr"); break;
	}
	return cdRxTx;
}

#include <time.h>
void dispTime(uint32_t ulTime){
	tm myTime;
	myTime.tm_sec = ulTime%60;
	myTime.tm_min = (ulTime/60)%60;
	myTime.tm_hour = (ulTime/3600)%24;
	myTime.tm_mday = ulTime/(3600*24);
	printf("::T-> %d:%d:%d:%d\n\r", myTime.tm_mday,
	myTime.tm_hour, myTime.tm_min, myTime.tm_sec);
}
char* dispForced(bool bForced){
	if(bForced) sprintf(cdRxTx,"manu");
	else sprintf(cdRxTx,"auto");
	return cdRxTx;
}

void UttecUtil::dispSec(rfFrame_t* pFrame, bool bCount){
	static uint32_t ulCount = 0;
	static uint32_t ulTime = 0;
	ulTime++;
	
	if(bCount)
	switch(ulTime%3){
		case 0:
			if(m_Factory.mode == eFactoryTestMode)
				printf("eFactoryTestMode\r\n");
			else if(m_Factory.mode == eFactoryOutMode)
				printf("eFactoryOutMode\r\n");
			else printf("eFieldMode\r\n");
		break;
		
		case 1:
			printf("G:%d P:%d RxTx:%s", 
			pFrame->MyAddr.GroupAddr, pFrame->MyAddr.PrivateAddr,
			dispRxTx(pFrame->MyAddr.RxTx.iRxTx));
			printf(" S:%s\r\n",
			dispSensor(pFrame->MyAddr.SensorType.iSensor));
		break;
		
		case 2:
			printf("H:%d, L:%d, Le:%d, D:%d ", pFrame->Ctr.High,
				pFrame->Ctr.Low, pFrame->Ctr.Level , pFrame->Ctr.DTime );
			dispTime(ulTime);
		break;
		
		case 3:
			if(!isMstOrGw(pFrame)){	
				printf("\r\nDim:%s, ",dispForced(myDimFact.forced));
				printf("Type:%s, ", dispSensor(myDimFact.sensorType));
				printf("tagetPwm = %0.3f, nowPwm = %0.3f\r\n", 
					myDimFact.targetValue, myDimFact.nowValue);
			}	
		break;
	}	
	
}

void UttecUtil::testProc(uint8_t ucName, uint32_t ulValue){
	printf("\n\r-------- Name: %d ::: value: %d -----------\n\r",
	ucName, ulValue);
}
void UttecUtil::testProc(uint8_t ucName, uint32_t ulValue,
	uint32_t ulValue1){
	printf("\n\r------ Name: %d ::: value: %d value: %d ---\n\r",
	ucName, ulValue, ulValue1);
}
void UttecUtil::testProc(uint8_t ucName, uint32_t ulValue,
	float fValue){
	printf("\n\r------ Name: %d ::: value: %d value: %0.3f ---\n\r",
	ucName, ulValue, fValue);
}

void UttecUtil::getDimFactor(dimFactors_t sFact){
	myDimFact.targetValue = sFact.targetValue;
	myDimFact.nowValue = sFact.nowValue;
	myDimFact.sensorType = sFact.sensorType;
	myDimFact.forced = sFact.forced;
}
dimFactors_t UttecUtil::getDimFactor(){
	return myDimFact;
}

void wdt_event_handler(void)
{
	printf("Reset System\n\r");
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}

void attimeout(){
	NVIC_SystemReset();
}

#define WDT_1Sec    32767UL
Timeout timeout;
void UttecUtil::setWdt(uint8_t ucTime)
{
#ifdef my52832
	printf("--------------load setWdtReload\n\r");
	timeout.attach(&attimeout, 3);
#else	
    NRF_WDT->POWER=WDT_POWER_POWER_Enabled;
    NRF_WDT->CRV=WDT_1Sec*ucTime;     //For 1Sec
    NRF_WDT->RREN=WDT_RREN_RR0_Enabled<<WDT_RREN_RR0_Pos;
    NRF_WDT->TASKS_START=1;
#endif
}
void UttecUtil::setWdtReload()
{
#ifdef my52832 
//	printf("--------------load setWdtReload\n\r");
	timeout.attach(&attimeout, 3);
//	nrf_drv_wdt_channel_feed(m_channel_id);
#else	
    NRF_WDT->RR[0]=     WDT_RR_RR_Reload;
#endif
}

uint16_t UttecUtil::changeBytesInWord(uint16_t uiData){
    ChangeByte_t uChange;
    uint8_t ucTemp;
    uChange.u16 = uiData;
    ucTemp = uChange.u8[0];
    uChange.u8[0]= uChange.u8[1];
    uChange.u8[1]= ucTemp;
    return uChange.u16;
}
bool UttecUtil::isTx(rfFrame_t* pFrame){
	if(pFrame->MyAddr.RxTx.iRxTx == eTx) return true;
	else return false;
}
bool UttecUtil::isSRx(rfFrame_t* pFrame){
	if(pFrame->MyAddr.RxTx.iRxTx == eSRx) return true;
	else return false;
}
bool UttecUtil::isRx(rfFrame_t* pFrame){
	if(pFrame->MyAddr.RxTx.iRxTx == eRx) return true;
	else return false;
}
bool UttecUtil::isRpt(rfFrame_t* pFrame){
	if(pFrame->MyAddr.RxTx.iRxTx == eRpt) return true;
	else return false;
}
bool UttecUtil::isGw(rfFrame_t* pFrame){
	if(pFrame->MyAddr.RxTx.iRxTx == eGW) return true;
	else return false;
}
bool UttecUtil::isMst(rfFrame_t* pFrame){
//	printf("------- rxtx = %d", pFrame->MyAddr.RxTx.iRxTx);
	if(pFrame->MyAddr.RxTx.iRxTx == eMst) return true;
	else return false;
}
bool UttecUtil::isMstOrGw(rfFrame_t* pFrame){
	if(pFrame->MyAddr.RxTx.iRxTx == eMst) return true;
	else if(pFrame->MyAddr.RxTx.iRxTx == eGW) return true;
	else return false;
}
bool UttecUtil::isMyRxTx(rfFrame_t* pDst, rfFrame_t* pSrc){
	if(pDst->MyAddr.RxTx.iRxTx == 
		pSrc->MyAddr.RxTx.iRxTx) return true;
	else return false;
}

void UttecUtil::dispRfFactor(rfFrame_t* frame){
	
	printf("\n\r------------- New Frame Factor --------------\n\r");
	printf("Gid=%d\n\r",frame->MyAddr.GroupAddr);
	printf("Pid=%d\n\r",frame->MyAddr.PrivateAddr);
	printf("RxTx=%d\n\r",frame->MyAddr.RxTx.iRxTx);
	printf("Sensor=%d\n\r",frame->MyAddr.SensorType.iSensor);
	
	printf("High=%d\n\r",frame->Ctr.High);
	printf("Low=%d\n\r",frame->Ctr.Low);
	printf("Level=%d\n\r",frame->Ctr.Level);
	printf("Type=%d\n\r",frame->Ctr.Type);
	printf("Rate=%d\n\r",frame->Ctr.SensorRate);
	printf("Status=%d\n\r",frame->Ctr.Status.iData);
	printf("DTime=%d\n\r",frame->Ctr.DTime);
}
#include "CmdDefine.h"
void UttecUtil::dispCmdandSub(char* cpCmd, char* cpSub, rfFrame_t* pFrame){
	memset(cpCmd,0,20);	memset(cpSub,0,20);
	switch(pFrame->Cmd.Command){
		case edVolume:
			sprintf(cpCmd,"edVolume");
			break;
		case edServerReq:
			sprintf(cpCmd,"edServerReq");
			break;
		case edClientAck:
			sprintf(cpCmd,"edClientAck");
			break;
		default:
			break;
	}
	switch(pFrame->Cmd.SubCmd){
		case edsPowerRead:
			sprintf(cpSub,"edsPowerRead");
			break;
		case edsControl:
			sprintf(cpSub,"edsControl");
			break;
		case edsNewSet:
			sprintf(cpSub,"edsNewSet");
			break;
		case edsCmd_485NewSet:
			sprintf(cpSub,"edsCmd_485NewSet");
			break;
		case edsCmd_Alternative:
			sprintf(cpSub,"edsCmd_Alternative");
			break;
		case edsCmd_Status:
			sprintf(cpSub,"edsCmd_Status");
			break;
		default:
			break;
	}
}
void UttecUtil::dispCmd(rfFrame_t* pFrame){
	switch(pFrame->Cmd.Command){
		case edSensor: 
			Putchar('S');
//			printf("Cmd is edSensor\r\n");
			break;
		case edRepeat: 
			Putchar('R');
		//printf("Cmd is edRepeat\r\n");
			break;
		case edVolume: 
			Putchar('V');
//			printf("Cmd is edVolume\r\n");
			break;
		case edDayLight: 
			Putchar('D');
//			printf("Cmd is edDayLight\r\n");
			break;
	}
}

UttecFactory_t UttecUtil::m_Factory = 
	{eFactoryTestMode,DeFactoryModeTimeout};

bool UttecUtil::isNotMyGwGroup(rfFrame_t* pSrc, rfFrame_t* pMy){
	ping_t* pDst = (ping_t*)&pSrc->Trans;
	if(pMy->MyAddr.GroupAddr/10 == pDst->gid/10) 
		return false;
	else return true;
}

bool UttecUtil::isMyGroup(rfFrame_t* pSrc, rfFrame_t* pMy){
	ping_t* pDst = (ping_t*)&pSrc->Trans;
	if(pMy->MyAddr.GroupAddr == pDst->gid) 
		return true;
	else return false;
}

bool UttecUtil::isMyAddr(rfFrame_t* pSrc, rfFrame_t* pMy){
	ping_t* pDst = (ping_t*)&pSrc->Trans;
#ifdef testModePrint	
	printf("dst gid =%d, pid = %d, rxtx =%d\r\n",
	pDst->gid, pDst->pid, pDst->rxtx);
#endif	
	if((pMy->MyAddr.GroupAddr == pDst->gid)&&
		(pMy->MyAddr.PrivateAddr == pDst->pid)
		&&(pMy->MyAddr.RxTx.iRxTx == pDst->rxtx)){	
#ifdef testModePrint		
			printf(" All Address Matching -> ");
#endif
			return true;
		}
	else if((pMy->MyAddr.GroupAddr == pDst->gid)&&
		(!pDst->pid)&&(!isMstOrGw(pMy))){
			printf(" Tx Matching and I'm this group -> ");
			return true;
	}		
	else{
		printf("Not Match and No Action\n\r");
		return false;
	}
}

bool UttecUtil::isFactoryOutMode(){
	Flash myFlash;
	
	bool bResult = false;
	rfFrame_t* pFrame = &myFlash.getFlashFrame()->rfFrame;

	if(m_Factory.timeout) m_Factory.timeout--;
	if(m_Factory.timeout&&(pFrame->MyAddr.GroupAddr == 
		DeFactoryChannel)){
			m_Factory.mode = eFactoryTestMode;
		}			
	else if(pFrame->MyAddr.GroupAddr != DeFactoryChannel){
		m_Factory.mode = eFieldMode;
	}
	else{
		m_Factory.mode = eFactoryOutMode;
		bResult = true;
	}	
	return bResult;
}
#include "UttecLed.h"

void UttecUtil::alertFaultSet(uint8_t ucFrom){
	UttecLed myLed;
	bool bToggle = false;
	while(1){
		setWdtReload();
		bToggle = !bToggle;
		if(bToggle){
			myLed.on(eRfLed);
			myLed.on(eSensLed);
		}
		else{
			myLed.off(eRfLed);
			myLed.off(eSensLed);
		}
		printf("You set Fault value from %d\n\r",ucFrom);
		wait(1);
	}
}

bool UttecUtil::SxRxCrc(rfFrame_t* pFrame){
	uint8_t* pucCrc = (uint8_t*)pFrame;
	uint16_t puiTime = *(uint16_t*)(pucCrc + 16);
	uint16_t puiCrc = *(uint16_t*)(pucCrc + 30);
	
	if(puiTime != gen_crc16(pucCrc, 16)){ 
		printf("_______time CRC Error, %d, %d\r\n",
		puiTime,gen_crc16(pucCrc, 16));
		return false;
	}	
	pucCrc = (uint8_t*)pFrame;
	if(puiCrc != gen_crc16(pucCrc, 30)){
		printf("++++++time CRC Error, %d, %d\r\n",
		puiCrc,gen_crc16(pucCrc, 30));
		return false;
	}
	return true; 

}

void UttecUtil::Putchar(char ucChar){
	mySer.putc(ucChar);
}

