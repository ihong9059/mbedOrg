#ifndef MBED_RANDOM_H
#define MBED_RANDOM_H
  
#include "mbed.h"
typedef struct{
	uint8_t mac[6];
} Mac_t;

class Rand {
private:	
	static PinName m_port;
	Mac_t m_mac;
public: 
	Rand();
	Rand(PinName);
	uint32_t getRanddom(); 
	Mac_t getMacAddr();
	bool isMacEqual(Mac_t, Mac_t);
	void testRand();
};
 
#endif
