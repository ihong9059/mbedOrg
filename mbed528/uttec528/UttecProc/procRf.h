#ifndef __PROCRF_H__
#define __PROCRF_H__

#include <stdint.h>
#include <stdbool.h>

#include "uttecLib.h"
#include "procServer.h"

#include "DimmerRf.h"
#include "mSecExe.h"

#define DeRfSetTimeout 20

typedef struct{
	uint8_t cmd;
	uint16_t start;
	uint16_t end;
} rfSet_find_t;

typedef struct{
	uint8_t cmd;
	uint16_t gid;
	uint8_t pid;
	uint8_t rxtx;
	uint8_t high;
	uint8_t low;
	uint16_t dtime;
} rfSet_set_t;

class procRf
{
private:
	static Flash* mpFlash;
	static Flash_t* mpFlashFrame;
	static rfFrame_t* mp_rfFrame;
	static DimmerRf* pMyRf;
	static rs485* pMy485;
	static UttecBle* pMyBle;
	static mSecExe* pMy_mSec;
	static procServer* pMyServer;

	void conflictTx();
	void procRfSensorCmd(rfFrame_t*);
	void procRfRepeatCmd(rfFrame_t*);
	void procRfVolumeCmd(rfFrame_t*);
	void procRfDaylightCmd(rfFrame_t*);
	void resendByRfRepeater(rfFrame_t*);
	void resendSensorByRfRepeater(rfFrame_t*);
	void searchRf(rfFrame_t*);
	void setNewFactor();
	void processCmdNewSet(rfFrame_t*);
	void transferMstGwByRf(rfFrame_t*);
	void returnToServer(rfFrame_t*);
	
public:
	procRf(uttecLib_t, procServer*);
	procRf(Flash*, DimmerRf*, mSecExe*);
	void taskRf(rfFrame_t*);

	void sendFind(rfSet_find_t);
	void sendSet(rfSet_set_t);
};
#endif
