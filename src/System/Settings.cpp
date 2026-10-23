#include <Arduino.h>
#include "Settings.h"
#include "C_output.h"

extern C_output output;

uint8_t FILTER_IN_SYSTEM = 2;           // max samples currently inserted in the system
bool ENABLE_OUTPUT = true;
bool spool_installed_session = SPOOL_INSTALLED; // the session's answer to "spool on the machine?"

System_state system_state = state_starting;

void set_system_state(System_state state){
    system_state = state;
    // TODO : communicate state to wifi card and 
   /*  switch (state){
        case state_starting:

        break;
        case state_idle:
        
        break;
        case state_sampling:
        
        break;
        case state_refill:
        
        break;
        case state_communicating:
        
        break;
        case state_error:
        
        break;
    } */
}

String format_date_logging(time_t t){
    // add time before printing in format yyyy-mm-dd hh:mm:ss
    String data = String(year(t))+"-";
    if(month(t) < 10)
        data += String("0")+String(month(t));
    else
        data += String(month(t));
    data+="-";
    if(day(t) < 10)
        data += String("0")+String(day(t));
    else
        data += String(day(t));
    data += " ";
    if(hour(t) < 10)
        data += String("0")+String(hour(t));
    else
        data += String(hour(t));
    data+=":";
    if(minute(t) < 10)
        data += String("0")+String(minute(t));
    else
        data += String(minute(t));
    data+=":";
    if(second(t) < 10)
        data += String("0")+String(second(t));
    else
        data += String(second(t));
    return data;
}

