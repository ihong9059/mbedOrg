#ifndef MBED_TEMPERATURE_H
#define MBED_TEMPERATURE_H
 
 
#include "mbed.h"
//#include "TMP102.h"
 
class Temperature {
    public: 
        Temperature();
        ~Temperature();
        float temperature_data_get();
    
};
 
#endif
