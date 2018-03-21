#ifndef __PROCSERVER_H__
#define __PROCSERVER_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "uttecLib.h"

#include "DimmerRf.h"
#include "mSecExe.h"

class procServer
{
private:
	static Flash* mpFlash;
	static Flash_t* mpFlashFrame;
	static rfFrame_t* mp_rfFrame;
	static DimmerRf* pMyRf;
	static rs485* pMy485;
	static UttecBle* pMyBle;
	static mSecExe* pMy_mSec;

	bool procControlSub(rfFrame_t*);
	bool procNewSetSub(rfFrame_t*);
	bool procNewFactSetSub(rfFrame_t*);
	bool procAltSub(rfFrame_t*);
	bool procStatus(rfFrame_t*);
	bool procPowerRead(rfFrame_t*);
	void setAckFrame(rfFrame_t*);

public:
	procServer(uttecLib_t);
	bool taskServer(rfFrame_t*);
	void taskClient(rfFrame_t*);
};

#endif
