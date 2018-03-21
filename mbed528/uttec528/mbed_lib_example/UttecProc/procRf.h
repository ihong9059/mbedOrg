#ifndef __PROCRF_H__
#define __PROCRF_H__

#include <stdint.h>
#include <stdbool.h>

#include "uttecLib.h"
#include "procServer.h"

#include "DimmerRf.h"
#include "mSecExe.h"

#define DeRfSetTimeout 20

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
public:
	procRf(uttecLib_t, procServer*);
	procRf(Flash*, DimmerRf*, mSecExe*);
	void taskRf(rfFrame_t*);
};
#endif
