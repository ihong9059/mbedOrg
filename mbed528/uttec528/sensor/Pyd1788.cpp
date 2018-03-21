#include "mbed.h"

#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"
#include "nrf_gpio.h"

#include "Pyd1788.h"

Pyd1788::Pyd1788(uint32_t io_num){
	m_port=io_num;
	m_state=eBitTrigStart;
	m_cycle=0;
	for(int i=0;i<10;i++) m_data[i]=0;
	nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_INPUT);	
}

void Pyd1788::setPort(){
	switch(m_state){
		case eBitTrigStart:
			nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_OUTPUT);
			nrf_gpio_pin_clear(m_port);
			m_state=eBitTrigEnd;
			break;
		case eBitTrigEnd:
			nrf_gpio_pin_set(m_port);
			m_state=eBitRead;
		default:
			break;
	}
}

void Pyd1788::testPyd1788(){
	nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_OUTPUT);
	nrf_gpio_pin_set(m_port);
	wait(0.00000);
	nrf_gpio_pin_clear(m_port);	
}

bool Pyd1788::getPort(){	
	bool bResult=false;
	nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_INPUT);	
	if(nrf_gpio_pin_read(m_port)){
		bResult=true;
	}
	return bResult;
}

void Pyd1788::myTrigDelay(){	//2uSec
//	for(int i=0;i<8;i++);
}
void Pyd1788::myReadDelay(){	//25uSec
	for(int i=0;i<5;i++);
}
void Pyd1788::myStrobeDelay(){	//25uSec
	for(int i=0;i<100;i++);
}

pydReturn_t* Pyd1788::processPyd1788(){
	static uint32_t ucCount = 0;
	ucCount++;
	if(ucCount%20){
		return &m_return;
	}
	uint16_t uiDelta;
//	putchar('?');		
	nrf_gpio_cfg_input(m_port,NRF_GPIO_PIN_NOPULL);
	while(!nrf_gpio_pin_read(m_port)){
	}
	myStrobeDelay();	//Total 100uSec(4Cycle)
	myStrobeDelay();	//Total 100uSec(4Cycle)
	myStrobeDelay();	//Total 100uSec(4Cycle)
	
	for(int i=0;i<14;i++){		
		
//Read Trigger Operation		
		nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_OUTPUT);
		myTrigDelay();
		nrf_gpio_pin_clear(m_port);
		myTrigDelay();
		nrf_gpio_pin_set(m_port);		
//		myTrigDelay();
		
//Read Operation		
		nrf_gpio_cfg_input(m_port,NRF_GPIO_PIN_NOPULL);
		myReadDelay();
		if(nrf_gpio_pin_read(m_port)) 
			m_channel++;
		m_channel<<=1;
		nrf_gpio_pin_clear(m_port);
		nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_OUTPUT);
	}	
	for(int i=0;i<14;i++){

//Read Trigger Operation		
		nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_OUTPUT);
		myTrigDelay();
		nrf_gpio_pin_clear(m_port);
		myTrigDelay();	//2uSec Delay
		nrf_gpio_pin_set(m_port);		
//		myTrigDelay();	//2uSec Delay

//Read Operation		
		nrf_gpio_cfg_input(m_port,NRF_GPIO_PIN_NOPULL);
		myReadDelay();	//25uSec Delay
		if(nrf_gpio_pin_read(m_port)) 
			m_dataTemp++;
		m_dataTemp<<=1;
		nrf_gpio_pin_clear(m_port);
		nrf_gpio_pin_dir_set(m_port,NRF_GPIO_PIN_DIR_OUTPUT);
	}	
	m_channel>>=1;
	m_dataTemp>>=1;

	m_iChannel=m_prechannel-m_channel;
	uiDelta=abs(m_iChannel);
	m_return.bSensing=false;
	
	if(uiDelta>=m_senseRate){
		putchar('!');
//		printf("Ch:%d, D:%d, Tem:%d\n\r",m_channel,uiDelta,m_dataTemp);
		m_return.bSensing=true;
	}		
	m_prechannel=m_channel;
	m_return.uiLevel=m_prechannel;
	m_return.uiTemperature=m_dataTemp;
	m_dataTemp=m_channel=0;
	return &m_return;
}

bool Pyd1788::read(){
	return m_return.bSensing;
}
pydReturn_t* Pyd1788::getPydValue(){
	return &m_return;
}
void Pyd1788::clearFlag(){
	m_return.bSensing = false;
}

void Pyd1788::setSenseRate(uint8_t ucRate){
	printf("&&&&&&&&&&&&&&&& rate = %d\n\r",ucRate);
	m_senseRate = ucRate;
}


