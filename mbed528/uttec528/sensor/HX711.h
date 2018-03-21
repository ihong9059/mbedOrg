#ifndef UTSOL_LOADCELL_H
#define UTSOL_LOADCELL_H

#include <stdint.h>

#define DeHXErrorValue	35000
#define DeTareCount	30
#define De10gScale		110

class HX711 
{
private:

	bool m_TareFinish;
	uint8_t GAIN;		// amplification factor
	uint8_t AverageNum;
	uint16_t m_10g;
	uint16_t m_resolution;
	uint16_t m_TareCount;	

	long OFFSET ;	// used for tare weight
	long m_Value;
	long m_AverageValue;
	
	float m_KgResult;

	void HX711LoadCellData();
	void HX711BatteryData();
	void HX711PowerUp();
	
	long averageing(long);
	long read_average();

public:
	void HX711PowerDown();
	HX711();
	void resetHX711();
	virtual ~HX711();

	void set_gain(uint8_t gain = 128);
	float get_value();
	long read();
	bool returnTareFinish();
	void testHX711();
};


#endif

