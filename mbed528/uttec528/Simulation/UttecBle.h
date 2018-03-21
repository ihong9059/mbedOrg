#ifndef __BLEEXE_H__
#define __BLEEXE_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "nrf.h"
#include "DimmerRf.h"

class UttecBle
{
private:
public:
	UttecBle();
	bool isBleRxReady();
	void clearBleRxReady();
	uint8_t* getBleRxData();  
	void sendBle(rfFrame_t*);
};

#endif
