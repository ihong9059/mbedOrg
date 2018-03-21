#ifndef __COMERR_H__
#define __COMERR_H__

#include <stdint.h>
#include <stdbool.h>

#include "mbed.h"
#include "uttecLib.h"

typedef struct{
	uint8_t id;
	uint8_t level;
	uint16_t gid;
} sCElement_t;

typedef struct{
	uint32_t timeout;
	sCElement_t element[10];
} comErr_t;

class comErr
{
private:
	static comErr_t m_comErr;

public:	
	comErr();
	void initComErr(uint32_t);
	void setComErr(uint16_t, uint8_t);
	void clearFlag();
	bool isComErr();
	void procComErrTimeout(DimmerRf*);
	void recoverComErrTimeout(DimmerRf*);
};	

#endif
