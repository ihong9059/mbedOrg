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

	void procControlSub(rfFrame_t*);
	void procNewSetSub(rfFrame_t*);
	void procNewFactSetSub(rfFrame_t*);
	void procAltSub(rfFrame_t*);
	bool procStatus(rfFrame_t*);
	void setAckFrame(rfFrame_t*);

public:
	procServer(uttecLib_t);
	bool taskServer(rfFrame_t*);
	void taskClient(rfFrame_t*);
};

#endif
