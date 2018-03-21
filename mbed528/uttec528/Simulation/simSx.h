#ifndef __SIMSX1276_H__
#define __SIMSX1276_H__
/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   1

/* Set this flag to '1' to use the LoRa modulation or to '0' to use FSK modulation */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   !USE_MODEM_LORA

//for UTTEC LoRa spectrum(905MHz~925MHz) step 500KHz, using 250KHz Bandwidth,
//Total 41 Channel, 925MHz is setting Channel(41 Channel)
//#define LoRaFreqBase	915000000-500000*20	//905000000
#define LoRaFreqBase	905000000
#define LoRaStep 500000
#define RF_FREQUENCY                                    915000000+500000*10 // Hz
//#define RF_FREQUENCY                                    868000000 // Hz
#define TX_OUTPUT_POWER                                 14        // 14 dBm

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
																															//  1: 250 kHz,
																															//  2: 500 kHz,
																															//  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]        org
//    #define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,           org
//    #define LORA_CODINGRATE                             4         // [1: 4/5,
																															//  2: 4/6,
																															//  3: 4/7,
																															//  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_FHSS_ENABLED                           false  
#define LORA_NB_SYMB_HOP                            4     
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            true
    

//#define RX_TIMEOUT_VALUE                                3500000   // in us org
#define RX_TIMEOUT_VALUE                                30000000   // in us 30Sec
#define BUFFER_SIZE                                     64        // Define the payload size here: max for LoRa

#include "DimmerRf.h"

typedef enum
{
	LOWPOWER = 0,
	IDLE,
	
	RX,
	RX_TIMEOUT,
	RX_ERROR,
	
	TX,
	TX_TIMEOUT,
	
	CAD,
	CAD_DONE
}AppStates_t;

typedef struct{
	uint8_t pid;
	uint8_t rxtx;
	uint8_t sensorType;
	uint8_t micom;	
	uint16_t gid;

	uint8_t high;
	uint8_t low;
	uint8_t level;
	uint8_t type;
	uint8_t rate;
	uint8_t status;
	uint16_t dtime;
	uint8_t cmd;
	uint8_t sub;
	uint16_t time;

	uint8_t srcPid;
	uint8_t dstPid;
	uint16_t srcGid;
	uint16_t dstGid;
	uint8_t mac[6];
	uint16_t crc;
/*
	uint8_t zone;
	uint8_t sum;
	*/
} sxFrame_t;

typedef struct{    
	sxFrame_t* ptrBuf;
	uint8_t size;    
} sxTxFrame_t;

typedef struct{
	bool sxRxFlag;
	sxFrame_t* ptrBuf;
	uint16_t size;
	int16_t rssi;
	int8_t snr;
} sxRxFrame_t;

class simSx
{    
private:
	static DimmerRf* pMySim;
	void enableSxTx();
//	void disableSxTx();
public:
	static sxRxFrame_t m_simSxRx;
	static sxTxFrame_t m_simSxTx;
//	static sxFrame_t m_simSx;

	simSx();
	simSx(DimmerRf*);
	void sendLoRa(sxTxFrame_t sFrame);
	sxRxFrame_t* readLoRa();
	void initSx1276();
	void initSx1276(uint16_t ucCh);
	void testTxRx(uint32_t);
	void clearSxRxFlag();
	bool isSxRxDone();
	void setSxRxFlag();
};
#endif // __UttecSx1276_H__
//#endif

