#include "mbed.h"
	
#include <stdio.h>
	
#include "nrf.h"
#include "nrf_nvmc.h"
#include "Flash.h"
//#include "ble_flash.h"

Flash_t Flash::m_flash={0,};

static volatile bool m_radio_active = false;  /**< TRUE if radio is active (or about to become active), FALSE otherwise. */
uint16_t ble_flash_crc16_compute(uint8_t * p_data, uint16_t size, uint16_t * p_crc)
{
    uint16_t i;
    uint16_t crc = (p_crc == NULL) ? 0xffff : *p_crc;

    for (i = 0; i < size; i++)
    {
        crc  = (unsigned char)(crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (unsigned char)(crc & 0xff) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xff) << 4) << 1;
    }
    return crc;
}
static void flash_page_erase(uint32_t * p_page)
{
    // Turn on flash erase enable and wait until the NVMC is ready.
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Erase page.
    NRF_NVMC->ERASEPAGE = (uint32_t)p_page;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash erase enable and wait until the NVMC is ready.
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing
    }
}

static void flash_word_write(uint32_t * p_address, uint32_t value)
{
    // If radio is active, wait for it to become inactive.
    while (m_radio_active)
    {
        // Do nothing (just wait for radio to become inactive).
 //       (void) sd_app_evt_wait();
    }

    // Turn on flash write enable and wait until the NVMC is ready.
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    *p_address = value;
    // Wait flash write to finish
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
    // Turn off flash write enable and wait until the NVMC is ready.
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing
    }
}
/*
*/
bool Flash::writeFlash(void)
{
	uint32_t* addrDst;
	uint32_t* addrSrc;
	uint32_t   pg_size,pg_num;
	bool fResult=true;

	pg_size = NRF_FICR->CODEPAGESIZE;
	pg_num  = NRF_FICR->CODESIZE - 1;  // Use last page in flash
	
	addrDst = (uint32_t *)(pg_size * pg_num);
//	nrf_nvmc_page_erase((uint32_t)addrDst);
	flash_page_erase(addrDst);
	addrSrc=(uint32_t *)&m_flash;
	
	for(int i=0;i<sizeof(Flash_t)/4;i++){		//Flash Write
//		nrf_nvmc_write_word((uint32_t)addrDst++, *addrSrc++);
		flash_word_write(addrDst++, *addrSrc++);
	}
	
	addrDst = (uint32_t *)(pg_size * pg_num);
	addrSrc=(uint32_t *)&m_flash;
	
	for(int i=0;i<sizeof(Flash_t)/4;i++){		//Verify Write
		if((*addrDst)!=*addrSrc){
			printf("%d,%d\n\r",i,*addrSrc);
			fResult=false;
		}
		addrDst++; addrSrc++;
	}
	if(fResult) printf("WFlash Ok!\n\r");
	else printf("Write Fail!\n\r");
	
	return fResult;
}

void Flash::ReadAllFlash(void)
{
	uint32_t* addrDst;
	uint32_t* addrSrc;
	uint32_t    pg_size,pg_num;
	bool fResult=true;

	pg_size = NRF_FICR->CODEPAGESIZE;
	pg_num  = NRF_FICR->CODESIZE - 1;  // Use last page in flash
	
	addrSrc = (uint32_t *)(pg_size * pg_num);
	addrDst=(uint32_t *)&m_flash;
	for(int i=0;i<sizeof(Flash_t)/4;i++){
		*addrDst++=*addrSrc++;
	}
	addrSrc = (uint32_t *)(pg_size * pg_num);
	addrDst=(uint32_t *)&m_flash;
	for(int i=0;i<sizeof(Flash_t)/4;i++){
		if((*addrDst)!=*addrSrc){
			printf("%d,%d\n\r",i,*addrSrc);
			fResult=false;
		}
		addrDst++; addrSrc++;
	}	
	if(!fResult) printf("Read Fail!\n\r");
}

bool Flash::isFactoryMode(){
	printf("Gid=%d\n\r",getFlashFrame()->rfFrame.MyAddr.GroupAddr);
	if(m_flash.FactoryModeId!=DeFieldMode){
//		printf("\f\n New Factors \n\r");
		initOrgFlash();
		return false;
	}
	else return true;
}

void Flash::clearsgFlashBuff(void)
{
	uint8_t *ucpTest;
	ucpTest=(uint8_t *)&m_flash;
	for(int i=0;i<sizeof(Flash_t);i++) *ucpTest++=0;
}


void Flash::TestFlashFunction(void)
{
	uint32_t uiTest=0x12345678;		
	m_flash.Life=uiTest;
	writeFlash();
	clearsgFlashBuff();
	printf("Before Life=%x\n\r",m_flash.Life);
	ReadAllFlash();
	printf("After Life=%x\n\r",m_flash.Life);
}


Flash::Flash()
{
}

Flash_t* Flash::getFlashFrame(){
	return &m_flash;
}

#define DeLifeEnd	43800	//356*24*5
//#define DeLifeEnd	10	//Test
void Flash::resetFlash(){
	uint8_t* ucpFrame=(uint8_t*)getFlashFrame();
	for(int i=0;i<sizeof(Flash_t);i++) *ucpFrame++=0;	
	writeFlash();
	wait(0.5);
	printf("End of resetFlash();\n\r");
	while(1);
//	NVIC_SystemReset();
}

#define DeFactoryChannel 1
void Flash::initOrgFlash(){
	
	uint8_t* ucpFrame=(uint8_t*)getFlashFrame();
	rfFrame_t rfFrame = getFlashFrame()->rfFrame;
	
	for(int i=0;i<sizeof(Flash_t);i++) *ucpFrame++=0;	
	rfFrame = rfFrame;
	
//	rfFrame.MyAddr.GroupAddr=DeFactoryChannel;		//For Test Only
	rfFrame.MyAddr.GroupAddr=3;		//For Test Only
	rfFrame.MyAddr.PrivateAddr=10;
	rfFrame.MyAddr.Micom.Bit.nRf518=1;
	rfFrame.MyAddr.RxTx.iRxTx=eRx;
	rfFrame.MyAddr.SensorType.iSensor=ePir;
	
	rfFrame.Ctr.High=100;
	rfFrame.Ctr.Low=0;
	rfFrame.Ctr.Level=0;
	rfFrame.Ctr.SensorRate=1;
	rfFrame.Ctr.DTime=1;
		
	m_flash.rfFrame=rfFrame;
	
	m_flash.DaliOrg.device=0;
	m_flash.DaliOrg.version=1;
	m_flash.DaliOrg.physicalMinLevel=254;
		
	m_flash.FactoryModeId=DeFieldMode;
	m_flash.Life=DeLifeEnd;
	m_flash.UttecDevice[0]=0;
	m_flash.UttecDevice[1]=0;

	channel_t m_Ch;
	ucpFrame=(uint8_t*)&m_Ch;
	for(int i=0;i<sizeof(channel_t);i++) *ucpFrame++=0;
	m_Ch.MatchFlag=m_Ch.SetupMode=false;
//	m_Ch.Hopping=eHoppingMid;
	m_Ch.Hopping=eNoHopping;
	m_Ch.bps=bps_250K;
	m_Ch.dbm=dbm_p4;	
	m_Ch.countError=0;
	m_Ch.channel=rfFrame.MyAddr.GroupAddr;

	m_flash.Channel=m_Ch;
	wait(0.1);
//	nrf_delay_ms(100);
	writeFlash();
	
//	NVIC_SystemReset();
}
