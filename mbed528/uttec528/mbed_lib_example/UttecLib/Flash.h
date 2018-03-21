#ifndef _FLASH_H
#define _FLASH_H

#include <stdint.h>

#include "Channel.h"

#define DeFieldMode	0x55aa55aa
#define DeDaliStatus	0x20

typedef enum{
	eRx=	1<<0,
	eTx=	1<<1,
	eSRx=	1<<2,
	eMx=	1<<3,
	eRpt=	1<<4,
	eGW=	1<<5,
	eMst=	1<<6,
	eReserved= 1<<7
} eRole_t;

typedef union
{
  struct
  {
    uint8_t Rx:1;
    uint8_t Tx:1;
    uint8_t SRx:1;
    uint8_t Mx:1;
    uint8_t Rpt:1;
    uint8_t GW:1;
    uint8_t Mst:1;
    uint8_t reserved0:1;    
  } Bit;
  uint8_t iRxTx;                        
} RxTx_t;

typedef enum{
	eNoSensor=1,
	ePir=2,
	eMicroWave=4,
	eDayLight=8,
	eUtraSonic=16,
	eHyBrid=32,
	eVolume=64,
	eTestMode=128
} eSensorType_t;

typedef union
{
  struct
  {
    uint8_t NoSensor:1;
    uint8_t Pir:1;
    uint8_t MicroWave:1;
    uint8_t DayLight:1;
    uint8_t UltraSonic:1;
    uint8_t Hybrid:1;    
    uint8_t Volume:1;    
    uint8_t TestMode:1;    
  } Bit;
  uint8_t iSensor;                        
} Sensor_t;

typedef union
{
  struct
  {
    uint8_t nRf24:1;
    uint8_t nRf518:1;
    uint8_t Rs485:1;
    uint8_t reserved0:5;    
  } Bit;
  uint8_t iType;                        
} Micom_t;

typedef struct
{
	uint8_t PrivateAddr;
	RxTx_t RxTx;
	Sensor_t SensorType;
	Micom_t Micom;	
	uint16_t GroupAddr;
} MyAddr_t;

typedef union
{
  struct
  {
    uint8_t H_L:1;
    uint8_t PowerMeter:1;
    uint8_t ServerMode:1;
    uint8_t GError:1;    
    uint8_t NewSet:1;    
    uint8_t reserved0:3;    
  } Bit;
  uint8_t iData;                        
} Status_t;

typedef struct
{	
	uint8_t High;
	uint8_t Low;
	uint8_t Level;
	uint8_t Type;
	uint8_t SensorRate;
	Status_t Status;
	uint16_t DTime;	
} Ctr_t;

typedef struct
{
	uint8_t Command;
	uint8_t SubCmd;
	uint16_t Time;
} Cmd_t;

typedef struct{
	uint8_t pid;
	uint8_t rxtx;
	uint16_t gid;
} dst_t;

typedef struct
{
	uint8_t SrcPrivateAddr;
	uint8_t DstPrivateAddr;
	uint16_t SrcGroupAddr;
	uint16_t DstGroupAddr;
	uint16_t Reserve[3];
	uint8_t Zone;
	uint8_t CheckSum;
} Trans_t;

typedef struct
{
	MyAddr_t 	MyAddr;
	Ctr_t 		Ctr;
	Cmd_t 		Cmd;
	Trans_t 	Trans;
} rfFrame_t;

typedef struct{
	uint8_t version;		//rom
	uint8_t device;		//rom
	uint8_t physicalMinLevel;	//rom
	uint8_t reserve;
} DaliFlash_t;

typedef struct{
	uint16_t gid;
	uint8_t dimLevel;
	uint8_t initLevel;
	uint16_t eTime;
} Jitc_t;

typedef struct
{
	rfFrame_t rfFrame;
	DaliFlash_t DaliOrg;
	channel_t Channel;
	uint8_t MacAddr[6];
	uint16_t MacFlag;
	uint32_t FactoryModeId;
	uint32_t Life;
	uint32_t AgingCycle;
	uint32_t UttecDevice[2];
	float VolumeCheck;
	Jitc_t jitcData[10];
} Flash_t;

class Flash
{
private: 
	static Flash_t m_flash;
public:
    Flash();
	bool writeFlash();
	void initOrgFlash();
	void ReadAllFlash();
	void clearsgFlashBuff();
	bool isFactoryMode();
	void TestFlashFunction();
	Flash_t* getFlashFrame();
	void resetFlash();
};
//extern Flash myFlash;

#endif

