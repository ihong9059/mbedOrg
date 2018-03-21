#ifndef __PROCSEC_H__
#define __PROCSEC_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "nrf.h"
#include "uttecLib.h"
#include "procServer.h"

typedef struct{
	bool rcu;
	bool rf;
	bool ble;
	bool rs485;
	bool sx1276;
} productType_t;

class procSec
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

	void testFrame(rfFrame_t*);
	void setProductType();
	void proc1Sec();

public:
	static productType_t m_product;

	procSec(uttecLib_t, procServer*);
	void secTask(rfFrame_t*);
};

#endif
