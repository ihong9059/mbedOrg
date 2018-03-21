#ifndef __UttecChannel_H__
#define __UttecChannel_H__

#include <stdbool.h>
#include <stdint.h>

#define DeNewRfChannel 23
#define DeRfChannel 70
//#define DeSetChannel 74
#define DeSetChannelOld 73
#define FREQ_OFFSET		1
#define DeChannelErrorCount	120	//120 Sec
#define DeTxChannelRate	5
#define DeRxChannelRate	1

#define DeSetupMode	true
#define DeNormalMode false	

#define DeRxEnable	true
#define DeTxEnable		false

typedef enum{
	eRxMode=true,
	eTxMode=false	
} RfRxMode_t;

typedef enum{
	dbm_p4=0x04UL,
	dbm_0=0x00UL,
	dbm_m4=0xFCUL,
	dbm_m8=0xF8UL,
	dbm_m12=0xF4UL,
	dbm_m16=0xF0UL,
	dbm_m20=0xECUL,
	dbm_m30=0xD8UL,
} TxDbm_t;

typedef enum{
	bps_1M=0x00UL,
	bps_2M=0x01UL,
	bps_250K=0x02UL,
	bps_Ble=0x03UL,
} Bps_t;

typedef enum{
	eHoppingLow=0,
	eHoppingMid=1,
	eHoppingHigh=2,
	eNoHopping=3
} ChannelHopping_t;

typedef struct
{
	bool MatchFlag;
	bool SetupMode;
	uint8_t Hopping;	
	Bps_t bps;
	TxDbm_t dbm;
	uint16_t countError;
	uint32_t channel;			
}channel_t;


#endif




