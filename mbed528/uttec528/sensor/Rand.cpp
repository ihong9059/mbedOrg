#include "Rand.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

PinName Rand::m_port = p2;

Rand::Rand(PinName ulPort){  
	m_port = 	ulPort;
}
Rand::Rand(){  
}
 
uint32_t Rand::getRanddom(void) {
AnalogIn aIn(m_port);
//AnalogIn aIn(p20);
	static uint32_t ucTemp = 13;
	static uint32_t ucTemp1 = 133;
	static uint32_t ucTemp2 = 1333;
	
	printf("Rand Adc:%d = %d: \r\n", m_port, aIn.read_u16());
	ucTemp = aIn.read_u16();
	ucTemp1 += ucTemp%1313;
	ucTemp2 += ucTemp1%1234;
	srand(int(ucTemp2%4321));
	return rand();
}

#include "nrf_ble_gap.h"

typedef union{
	uint8_t u8[4];
	uint32_t u32;
} uu32_t;

Mac_t Rand::getMacAddr(){
	uu32_t mac32[2];
	
	ble_gap_addr_t m_address_1;
	m_address_1.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE;
	for(uint8_t i = 0;i<2;i++)
		 m_address_1.addr[i] = 0xbb;
	sd_ble_gap_address_set(BLE_GAP_ADDR_CYCLE_MODE_NONE, &m_address_1);

	for(uint8_t i = 0;i<2;i++)
	{
		mac32[i].u32 = NRF_FICR->DEVICEID[i];
		printf("Addr:%x\n\r",
		NRF_FICR->DEVICEID[i]);
	}
	m_mac.mac[5]=mac32[1].u8[3]; 
	m_mac.mac[4]=mac32[1].u8[2]; 
	m_mac.mac[3]=mac32[1].u8[1]; 
	m_mac.mac[2]=mac32[1].u8[0]; 
	m_mac.mac[1]=mac32[0].u8[3]; 
	m_mac.mac[0]=mac32[0].u8[2]; 
	printf("From Ble Mac Address:\n\r");
	for(int i = 0; i<6; i++){
		printf("Mac[%d]:%x, ",i , m_mac.mac[i]);
	}
	printf("\r\n");
	return m_mac;	
}

bool Rand::isMacEqual(Mac_t src, Mac_t dst){
	uint8_t ucEqualCount = 0;
	for(int i = 0; i<6; i++){
		if(src.mac[i] == dst.mac[i]) ucEqualCount++;
	}
	if(ucEqualCount>5) return true;
	else return false;
}

void Rand::testRand(){
Rand myRand(p2);
	Mac_t ucdTest[2000];
	for(int i = 0; i<2000; i++){
		for(int j = 0; j<6; j++){
			ucdTest[i].mac[j]= myRand.getRanddom();
		}
	}
	for(int j = 0; j<6; j++)
	printf("%d :",ucdTest[11].mac[j]);
	printf("\r\n");
	
	for(int j = 0; j<6; j++)
	printf("%d :", ucdTest[111].mac[j]);
	printf("\r\n");
	
	for(int j = 0; j<6; j++)
	printf("%d :", ucdTest[444].mac[j]);
	printf("\r\n");
	
	for(int j = 0; j<6; j++)
	printf("%d :", ucdTest[987].mac[j]);
	printf("\r\n");
	
	Mac_t ucdNow;
	uint32_t ulEqualCount = 0;
	uint32_t ulTotal = 0;
	
	printf("Now Start\n\r");
	
	while(1){
		for(int j = 0; j<6; j++)
			ucdNow.mac[j]= myRand.getRanddom();
		ulTotal++;
		for(int i = 0; i<2000; i++){
			if(myRand.isMacEqual(ucdTest[i], ucdNow)){
				ulEqualCount++;
				for(int j = 0; j<6; j++)
				printf("%d: %d \n\r", ucdNow.mac[j], ucdTest[i].mac[j]);
				printf("Total Count:%d, EqualCount:%d\r\n",
				ulTotal, ulEqualCount);
			}
		}
		printf(".");
		if(!(ulTotal%100)) printf("Current %d, Equal %d\r\n", 
			ulTotal, ulEqualCount);
	}
}
