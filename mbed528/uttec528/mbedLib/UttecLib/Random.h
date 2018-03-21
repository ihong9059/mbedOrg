 #ifdef __cplusplus
extern "C" {
#endif
	
#ifndef __Random_H__
#define __Random_H__

#include <stdint.h>

#define DeRandomBuffSize	16
	
class Random
{
private: 
//	uint32_t m_range;
	void init();
public:
    Random();
    Random(uint32_t);
	uint32_t generate();
	uint32_t generate_uint();
};


#endif

#ifdef __cplusplus
}
#endif

