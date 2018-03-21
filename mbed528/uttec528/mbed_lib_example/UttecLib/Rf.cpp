#include <stdio.h>
#include <string.h>
#include "mbed.h"

#include "nrf.h"

#include  "Rf.h"

rfFrame_t Rf::m_SxRxFrame={0};

rfFrame_t Rf::m_RxFrame={0};
rfFrame_t Rf::m_TxFrame={0};
channel_t Rf::m_Ch={0}; 
RfRxMode_t Rf::m_RfRxTx=eRxMode;

bool Rf::TxEndFlag=false;
bool Rf::SxRxEndFlag=false;
bool Rf::RxEndFlag=false;
uint32_t Rf::m_blockingTime = 0;

void clock_initialization()
{
    /* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
        // Do nothing.
    }

    /* Start low frequency crystal oscillator for app_timer(used by bsp)*/
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Synth << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }
	/*
	*/
}


Rf::Rf(){
	TxEndFlag=RxEndFlag=false;
	uint8_t* ucpRx=(uint8_t*)&m_RxFrame;
	uint8_t* ucpTx=(uint8_t*)&m_TxFrame;
	for(int i=0;i<DePacketSize;i++) 
		*ucpRx++=*ucpTx++=0;
	m_RfRxTx=eRxMode;
	/*
	m_Ch.MatchFlag=m_Ch.SetupMode=false;
	m_Ch.Hopping=eNoHopping;
	m_Ch.bps=bps_250K;
	m_Ch.dbm=dbm_p4;	
	m_Ch.countError=0;
	m_Ch.channel=1;
	*/
	m_test=0xaaaaaaaa;
	printf("New Rf\n\r");
}
void Rf::setRadio(channel_t sCh){
//	printf("$= %d\n\r",sCh.channel);
    // Radio config
    NRF_RADIO->TXPOWER   = (sCh.dbm << RADIO_TXPOWER_TXPOWER_Pos);
	if(sCh.SetupMode){
		NRF_RADIO->FREQUENCY = DeSetChannelOld;  
	}
	else{
		if(sCh.Hopping==eNoHopping){
			NRF_RADIO->FREQUENCY = sCh.channel%DeRfChannel
			+FREQ_OFFSET;
		}
		else{
			NRF_RADIO->FREQUENCY = sCh.channel%DeNewRfChannel
			+DeNewRfChannel*sCh.Hopping+FREQ_OFFSET;
			printf("Hop=%d\n\r",sCh.Hopping);
		}
	}
    NRF_RADIO->MODE      = (sCh.bps << RADIO_MODE_MODE_Pos);
/*	
    NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
    NRF_RADIO->FREQUENCY = 7UL;  // Frequency bin 7, 2407MHz
    NRF_RADIO->MODE      = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);
	*/
	
	radio_configureNew(sCh.channel);
}

void Rf::initRf(channel_t* pCh)
{
	m_Ch=*pCh;
	clock_initialization();
	setRadio(m_Ch);
	wait(0.002);
//	nrf_delay_ms(2);		//Important for Irq operation
	setRfMode(eRxMode);
//	nrf_delay_ms(10);
//	printf("FREQUENCY = %d \n\r", NRF_RADIO->FREQUENCY);
//	printf("-------initRf Ch = %d\n\r", m_Ch.channel);
}

uint8_t CalCheckSum(rfFrame_t *tFrame)
{
	uint8_t *cpFrame;
	uint8_t cTemp=0;
	uint8_t cCount=0;
	uint8_t cSize=sizeof(rfFrame_t)/sizeof(uint8_t)-1;
	
	cpFrame=(uint8_t *)tFrame;
	while(cSize-cCount){
		cTemp+=*cpFrame++;
		cCount++;
	}
	return cTemp;
}

void Rf::sendInternal(){
	clearRfMode();
	setRfMode(eTxMode);
    while (NRF_RADIO->EVENTS_END == 0U) {}; //Wait Event End 		
	NRF_RADIO->EVENTS_END=0;
	TxEndFlag=true;
/*		
	printf("Ch=%d, Hop=%d -> ",NRF_RADIO->FREQUENCY,
		m_Ch.Hopping);
*/
	clearRfMode();
	setRfMode(eRxMode);
}

void Rf::sendRf(const rfFrame_t* ucpTx)
{
	UttecUtil myUtil;
	printf("sendRf Ch =%d -> ", m_Ch.channel);
	uint8_t ucTempHop=0;
	m_blockingTime=30;
	m_TxFrame=*ucpTx;
	m_TxFrame.Trans.Reserve[0]=m_TxFrame.Cmd.Time;
	m_TxFrame.Trans.Reserve[1]=m_TxFrame.Cmd.Time+1;
	m_TxFrame.Trans.Reserve[2]=
	myUtil.gen_crc16((uint8_t*)m_TxFrame.Trans.Reserve,4);
	m_TxFrame.Trans.CheckSum=CalCheckSum(&m_TxFrame);
	
	ucTempHop=m_Ch.Hopping;
	if(m_Ch.Hopping!=eNoHopping){
		for(int i=0;i<eNoHopping;i++){
			m_Ch.Hopping=i; 	setRadio(m_Ch);
			sendInternal();
		}
	}
	else sendInternal();
	m_Ch.Hopping=ucTempHop;
}

void Rf::sendSxRf(const rfFrame_t* ucpTx)
{
	UttecUtil myUtil;
	uint8_t ucTempHop=0;
	m_TxFrame=*ucpTx;
	
	ucTempHop=m_Ch.Hopping;
	if(m_Ch.Hopping!=eNoHopping){
		for(int i=0;i<eNoHopping;i++){
			m_Ch.Hopping=i; 	setRadio(m_Ch);
			sendInternal();
		}
	}
	else sendInternal();
	m_Ch.Hopping=ucTempHop;
}



bool Rf::isOkCheckSum(uint16_t myAddr){
	UttecUtil myUtil;
	bool bResut=false;
	uint8_t ucResult;
		
	printf("Cmd=%d\n\r",m_RxFrame.Cmd.Command);
	if(m_RxFrame.MyAddr.GroupAddr==myAddr){
		bResut=true; 
	}
	else {
		printf("Address Error=%d,%d\n\r",myAddr,
			m_RxFrame.MyAddr.GroupAddr); 
		return false;
	}		
	ucResult=CalCheckSum(&m_RxFrame);
	if(m_RxFrame.Trans.CheckSum==ucResult){
		bResut=true;
	}
	else{
		printf("CheckSum Error %d,%d\n\r",
		m_RxFrame.Trans.CheckSum,ucResult);
		return false;
	}
	uint16_t uiResult=myUtil.gen_crc16((uint8_t*)m_RxFrame.Trans.Reserve,4);
	if(uiResult==m_RxFrame.Trans.Reserve[2]){
		bResut=true; 
	}
	else{
		printf("CheckSum Error %d,%d\n\r",
		m_RxFrame.Trans.Reserve[2],uiResult);
		return false;
	}
	return bResut;
}

uint32_t Rf::receiveAction(){
   uint32_t result = 0;
    if (NRF_RADIO->CRCSTATUS == 1U)
    {
        result = m_RxFrame.MyAddr.GroupAddr;	
				m_SxRxFrame = m_RxFrame;	
//				printf("m_SxRxFrame.gid = %d",m_SxRxFrame.MyAddr.GroupAddr);
//		RxEndFlag=isOkCheckSum();
		RxEndFlag=true;
		SxRxEndFlag=true;
    }
	clearRfMode();
	setRfMode(eRxMode);
    return result;
}

bool Rf::isRxDone(){
	if(m_RfRxTx&&NRF_RADIO->EVENTS_END){
		NRF_RADIO->EVENTS_END=0;
		receiveAction();
//		printf("\n\rRxDone=%d\n\r",receiveAction());
	}
	return RxEndFlag;
}

bool Rf::isSxRxDone(){
	if(m_RfRxTx&&NRF_RADIO->EVENTS_END){
		NRF_RADIO->EVENTS_END=0;
		receiveAction();
//		printf("\n\rRxDone=%d\n\r",receiveAction());
	}
	return SxRxEndFlag;
}

void Rf::clearRxFlag(){
	RxEndFlag=false;
}

void Rf::clearSxRxFlag(){
	SxRxEndFlag=false;
}

void Rf::setRxFlag(){	//For rfFrame simulation
	RxEndFlag=true;
}

void Rf::setSxRxFlag(){	//For rfFrame simulation
	SxRxEndFlag=true;
}

void Rf::setRfMode(RfRxMode_t eMode){
	if(eMode==eRxMode){
		m_RfRxTx=eRxMode;
		NRF_RADIO->PACKETPTR = (uint32_t)&m_RxFrame;
		NRF_RADIO->EVENTS_READY = 0U;
		NRF_RADIO->TASKS_RXEN = 1U;
		NRF_RADIO->TASKS_TXEN = 0U;
	}
	else{
		m_RfRxTx=eTxMode;
		NRF_RADIO->PACKETPTR = (uint32_t)&m_TxFrame;
		NRF_RADIO->EVENTS_READY = 0U;
		NRF_RADIO->TASKS_RXEN = 0;
		NRF_RADIO->TASKS_TXEN   = 1;
	}
	while (NRF_RADIO->EVENTS_READY == 0U) {}; //Wait Event Ready
	NRF_RADIO->EVENTS_END  = 0U;
	NRF_RADIO->TASKS_START = 1U;	
}

void Rf::clearRfMode(){
    NRF_RADIO->EVENTS_DISABLED = 0U;
    // Disable radio
    NRF_RADIO->TASKS_DISABLE = 1U;

    while (NRF_RADIO->EVENTS_DISABLED == 0U)
    {
        // wait
    }
	m_RfRxTx=eRxMode;
}

rfFrame_t* Rf::returnSxRxBuf(){
	return &m_SxRxFrame;
}

rfFrame_t* Rf::returnRxBuf(){
	return &m_RxFrame;
}
Rf* Rf::returnRfClass(){
	return this;
}
const channel_t* Rf::returnRadio() {
	return &m_Ch;
}

void Rf::changeGroup(uint32_t uiGroup){
	m_Ch.channel=uiGroup;
	printf("TxPower=%d\n\r",m_Ch.dbm);
	setRadio(m_Ch);
}

void Rf::changeGroup(uint32_t uiGroup,TxDbm_t eDbm){
	m_Ch.channel=uiGroup;
	m_Ch.dbm=eDbm;
	setRadio(m_Ch);
}

void Rf::changeGroup(channel_t sChannel){
	m_Ch=sChannel;
	setRadio(m_Ch);
}
#define DeHoppingChangeNum	5
uint8_t Rf::m_HoppingCount=DeHoppingChangeNum;

void Rf::isOkChannel(){
	static uint8_t HoppingNum=0;
	if(m_Ch.Hopping!=eNoHopping){
		if(m_HoppingCount)	m_HoppingCount--;
		else{
			printf("&&&Hopping from %d ",m_Ch.Hopping);
			HoppingNum++; HoppingNum%=eNoHopping;
			m_Ch.Hopping=HoppingNum;
			printf(" to %d\n\r",m_Ch.Hopping);
			m_HoppingCount=DeHoppingChangeNum;
			setRadio(m_Ch);
		}
	}
}

void Rf::setHoppingCount(){
	m_HoppingCount=DeHoppingChangeNum;
}





