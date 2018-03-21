#ifndef __PROCBLE_H__
#define __PROCBLE_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "nrf.h"
#include "uttecLib.h"
#include "procServer.h"

class procBle
{
private:
	static DimmerRf* m_pRf;
public:
	procBle(uttecLib_t, procServer*);
	void bleTask(uint8_t*);
};

#endif
