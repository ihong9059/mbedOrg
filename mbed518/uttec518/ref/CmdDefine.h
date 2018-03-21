#ifndef __UttecCmdDef_H__
#define __UttecCmdDef_H__

typedef enum{
	eRf = 0,
	e485 = 1
} eRf485_t;

typedef enum{
	edDummy=0, edSensor, edRepeat, edLifeEnd,
	edNewSet, //0~4
	edNewSetAck, edReady, edReadyAck, edSearch,
	edSearchAck, //5~9
	edBack, edAsk, edAskAck, edVolume,
	edDayLight, //10~14
	
	edColor,	//15
	edSchedule = 20, edMonitor, edEmergency,
	edServerReq=100, edClientAck, edClientReq,
	edServerAck, edReportLife, //100~104
	edDefaultAck=200, edTestMode
} eCmd_t;

typedef enum{
	edsStart=0, edsFromRpt, edsRepeat,
	edsFromTx,	//0~3
	edsPowerReset=100, edsPowerRead, edsMonitor,
	edsControl, edsNewSet,	//100~104
	edsPing, edsPhoto, edsColor, edsCmd_485NewSet,
	edsCmd_Alternative,	//105~109
	edsCmd_Status, edsApprove, edsAck, edsSchedule1,
	edsSchedule2				//110~114
} eSubCmd_t;

#endif




