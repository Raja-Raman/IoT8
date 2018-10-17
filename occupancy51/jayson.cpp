// jayson.cpp

#include "jayson.h"
 
// static variables 
DynamicJsonBuffer  jbufferSensors (JSON_BUFFER_SIZE);   // outgoing - serialize
DynamicJsonBuffer  jbufferCommand (JSON_BUFFER_SIZE);   // incoming - deserialize  

const char* MyJson::serialize (const SensorData& tx_payload) {
    jbufferSensors.clear(); // to reuse 
    JsonObject& root = jbufferSensors.createObject();    
    root["ID"] = tx_payload.device_id;     
    root["G"] = tx_payload.group_id; 
    root["S"] = tx_payload.node_status;  
    root["P"] = tx_payload.status_param;     
    root["T"] = tx_payload.temperature;
    root["H"] = tx_payload.humidity;
    root["I"] = tx_payload.heat_index;          
    root["L"] = tx_payload.light;         
    root.printTo(tx_str_buffer, JSTRING_BUFFER_SIZE);
    return ((const char*)tx_str_buffer);
}

/*
  Memory allocation for CommandData object is done in this class.
  So do not call deserialize() more than once, before consuming the command_data_cache object !
 */
const CommandData& MyJson::deserialize (const char* rx_payload) {
    jbufferCommand.clear(); // to reuse 
    JsonObject& root = jbufferCommand.parseObject (rx_payload);
    if (!root.success()) {
        SERIAL_PRINTLN("Command: Json parsing failed !");
        return (cmd_parsing_error);
    }    
    // if any numeric key is absent, it is filled with zero
    command_data_cache.device_id = root["ID"];
    command_data_cache.group_id = root["G"];    
    command_data_cache.command = root["C"]; 
    command_data_cache.long_param = root["L"];   
    // if the string key is garbage, 8266 crashes !
    if (root.containsKey("S")) {  
        strncpy (command_data_cache.string_param, root["S"], MAX_STRING_LENGTH-1);
        command_data_cache.string_param[MAX_STRING_LENGTH-1] = '\0';  // overflow -> unterminated!
    }
    else
        strncpy (command_data_cache.string_param, "", MAX_STRING_LENGTH);
    return(command_data_cache); 
}



