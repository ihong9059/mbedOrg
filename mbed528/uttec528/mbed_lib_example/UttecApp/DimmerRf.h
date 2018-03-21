#ifndef __DimmerRf_H__
#define __DimmerRf_H__

#include <stdint.h>
#include <stdbool.h>

#include "Rf.h"

typedef struct{
	rfFrame_t* rfPointer;
	bool bCommand;
} RfCmdFlag_t;

class DimmerRf : public Rf
{
private:
	static Flash* mpFlash;
	static Flash_t* mpFlashFrame;
	static rfFrame_t* mp_rfFrame;
	/*
*/
public:	
	DimmerRf();
	DimmerRf(Flash* );
	void test();
	void initRfFrame();
	void dispRfFactor(rfFrame_t* pFrame);	
};
//extern DimmerRf* myRf;
#endif
