// jayson.h
// NOTE : The Json parser class in unique to every application.
// Different apps will have different implementations with the same file names.
// TODO: make them derive from the same base class

#ifndef JAYSON_H
#define JAYSON_H

#include "common.h"
#include "flashHelper.h"
#include <Arduino.h>
#include <ArduinoJson.h>   // https://github.com/bblanchon/ArduinoJson 

#define JSON_BUFFER_SIZE        256   // must be larger than the content inside
#define JSTRING_BUFFER_SIZE     128    

// Use https://arduinojson.org/v5/assistant/ compute the actual capacity.
   
class MyJson {   // TODO: convert everyting to static variables and methods
public:   
    const char* serialize (const SensorData&  tx_payload);
    const CommandData& deserialize (const char*  rx_payload);
    
private:
    char tx_str_buffer[JSTRING_BUFFER_SIZE];     // serialize SensorData into this string and return to caller      
    CommandData command_data_cache;      
    const CommandData cmd_parsing_error = {0L,0L,CMDX_PARSING_ERROR,0L,""};  
};

#endif


 
