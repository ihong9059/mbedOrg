#include "Temperature.h"
#include "nrf_temp.h"
 
Temperature::Temperature(){    
}
 
Temperature::~Temperature(){
}
 
float Temperature::temperature_data_get(void) {
        float temp;
        NRF_TEMP->TASKS_START = 1; /** Start the temperature measurement. */
 
        while (NRF_TEMP->EVENTS_DATARDY == 0) {
            // Do nothing.
        }
        NRF_TEMP->EVENTS_DATARDY = 0;
				
        temp = (nrf_temp_read()/4.0);
 
				printf("nrf_temp_read = %d\r\n",nrf_temp_read());
				
        NRF_TEMP->TASKS_STOP = 1; /** Stop the temperature measurement. */
 
        return (temp-9.0);
}
