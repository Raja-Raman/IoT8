//config.cpp
#include "config.h"
#include "flashHelper.h"

Config::Config(){
    SERIAL_PRINTLN ("Creating the Config object.. [Ensure it is a singleton]");
    
    if (strlen(FW_SERVER_URL) >= MAX_STRING_LENGTH)
        SERIAL_PRINTLN("*** Firmware server URL is too long ! truncating. ***");
    strncpy (firmware_server_url, FW_SERVER_URL, MAX_STRING_LENGTH);
    firmware_server_url[MAX_STRING_LENGTH-1] = '\0';
    
    if (strlen(FW_VERSION_URL) >= MAX_STRING_LENGTH)
        SERIAL_PRINTLN("*** Firmware version URL is too long ! truncating. ***");    
    strncpy (version_check_url, FW_VERSION_URL, MAX_STRING_LENGTH); 
    version_check_url[MAX_STRING_LENGTH-1] = '\0';
}

void Config::init() {
    loadDeviceData ();
}

/* 
   Read the device id and group id from Flash and
    embed them into the Config object. This is to be done before
   initializing the wireless and http helpers.
*/
void Config::loadDeviceData () {
    FlashHelper F;
    F.init(this);
    F.begin();
    SERIAL_PRINTLN("Trying to read EEPROM...");
    bool result = F.readFlash ();
    F.end();

    if (!result) {
        SERIAL_PRINTLN("\n*** --------  FATAL ERROR ------------ ***");
        SERIAL_PRINTLN("Could not read device data from flash.");
        device_id = random(100001, 999999);
        group_id = random(100001, 999999);
        threshold = 0L;
        SERIAL_PRINT("Assuming random device id: ");
        SERIAL_PRINTLN(device_id);
        SERIAL_PRINT("Random group id: ");
        SERIAL_PRINTLN(group_id);        
    } 
    else 
        SERIAL_PRINTLN("\Successfully retrieved device data from flash."); 
    // the values are already stored in 'this' pointer    
}

void Config::targetTestUrl () {
    http_post_url = (char *)gateway_test_url;
}
        
void Config::targetProdUrl () {
    http_post_url = (char *)gateway_prod_url;
}

void Config::dump() {
    SERIAL_PRINT ("Firmware version: 1.0.");
    SERIAL_PRINTLN (FIRMWARE_VERSION);
    long freeheap = ESP.getFreeHeap();
    SERIAL_PRINT ("Free heap: ");
    SERIAL_PRINTLN (freeheap);    
    SERIAL_PRINT ("Device ID: ");
    SERIAL_PRINTLN (device_id);     
    SERIAL_PRINT ("Device Group: ");
    SERIAL_PRINTLN (group_id);        
    SERIAL_PRINT ("Threshold: ");
    SERIAL_PRINTLN (threshold);   
    SERIAL_PRINT ("Firmware server URL: ");
    SERIAL_PRINTLN (firmware_server_url);    
    SERIAL_PRINT("Firmware version URL: ");
    SERIAL_PRINTLN(version_check_url);        
    SERIAL_PRINT ("Production Gateway: ");
    SERIAL_PRINTLN (gateway_prod_url);
    SERIAL_PRINT ("Test Gateway: ");
    SERIAL_PRINTLN (gateway_test_url);   
}
 

 
