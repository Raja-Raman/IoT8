//config.cpp
#include "config.h"
#include "flashHelper.h"
///#include "externDeclare.h"

extern void safe_strncpy (char *dest, char *src, int length = MAX_STRING_LENGTH);

Config::Config(){
    SERIAL_PRINTLN ("Creating the Config object.. [Ensure it is a singleton]");
    safe_strncpy (data_prod_url,        DATA_PROD_URL);
    safe_strncpy (data_test_url,        DATA_TEST_URL);
    safe_strncpy (cmd_prod_url,         CMD_PROD_URL);
    safe_strncpy (cmd_test_url,         CMD_TEST_URL);
    safe_strncpy (firmware_server_url,  FW_SERVER_URL);
    safe_strncpy (version_check_url,    FW_VERSION_URL); 
}

void Config::init() {
    loadDeviceData ();
}

void Config::targetTestUrl () {
    data_url = (char *)data_test_url;
    cmd_url = (char *) cmd_test_url;
    SERIAL_PRINTLN("Now in TEST_MODE:"); 
    SERIAL_PRINT ("Gateway data URL set to: ");
    SERIAL_PRINTLN(data_url);
    SERIAL_PRINT ("Gateway command URL set to: ");
    SERIAL_PRINTLN(cmd_url);    
}
        
void Config::targetProdUrl () {
    data_url = (char *)data_prod_url;
    data_url = (char *)cmd_prod_url;    
    SERIAL_PRINTLN("Now in PRODUCTION_MODE:"); 
    SERIAL_PRINT ("Gateway data URL set to: ");
    SERIAL_PRINTLN(data_url);
    SERIAL_PRINT ("Gateway command URL set to: ");
    SERIAL_PRINTLN(cmd_url); 
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
    bool result = F.readFlash ();  // this will populate the config object *pC
    F.end();

    if (!result) {
        SERIAL_PRINTLN("\n*** --------  FATAL ERROR ------------ ***");
        SERIAL_PRINTLN("Could not read device data from flash.");
        device_id = random(100001, 999999);
        group_id = random(100001, 999999);
        relay_enabled = 0;
        buzzer_enabled = 0;
        SERIAL_PRINT("Assuming random device id: ");
        SERIAL_PRINTLN(device_id);
        SERIAL_PRINT("Random group id: ");
        SERIAL_PRINTLN(group_id);        
    } 
    else 
        SERIAL_PRINTLN("\Successfully retrieved device data from flash."); 
    // the values are already stored in 'this' pointer    
}

void Config::dump() {
    SERIAL_PRINTLN("\n-----------------------------------------");
    SERIAL_PRINTLN("               configuration             ");
    SERIAL_PRINTLN("-----------------------------------------");    
    SERIAL_PRINT ("Firmware version: 1.0.");
    SERIAL_PRINTLN (FIRMWARE_VERSION);
    long freeheap = ESP.getFreeHeap();
    SERIAL_PRINT ("Free heap: ");
    SERIAL_PRINTLN (freeheap);    
    SERIAL_PRINT ("Device ID: ");
    SERIAL_PRINTLN (device_id);     
    SERIAL_PRINT ("Device Group: ");
    SERIAL_PRINTLN (group_id);        
    SERIAL_PRINT ("Relay enabled?: ");
    SERIAL_PRINTLN (relay_enabled);   
    SERIAL_PRINT ("Buzzer enabled?: ");
    SERIAL_PRINTLN (buzzer_enabled); 
    SERIAL_PRINT ("Fire and forget mode?: ");
    SERIAL_PRINTLN (fire_and_forget);
    SERIAL_PRINTLN();    
    
    SERIAL_PRINT ("Firmware server URL: ");
    SERIAL_PRINTLN (firmware_server_url);    
    SERIAL_PRINT("Firmware version URL: ");
    SERIAL_PRINTLN(version_check_url);      
      
    SERIAL_PRINT ("Production Gateway [data]: ");
    SERIAL_PRINTLN (data_prod_url);
    SERIAL_PRINT("Production gateway [command]: ");
    SERIAL_PRINTLN(cmd_prod_url); 
        
    SERIAL_PRINT ("Test Gateway [data]: ");
    SERIAL_PRINTLN (data_test_url);   
    SERIAL_PRINT("Test gateway [command]: ");
    SERIAL_PRINTLN(cmd_test_url);     
    SERIAL_PRINTLN();
    
    SERIAL_PRINTLN("Production host, port, URI [data]: ");
    SERIAL_PRINT(data_host); 
    SERIAL_PRINT(" , ");
    SERIAL_PRINT(data_port);
    SERIAL_PRINT(" , ");    
    SERIAL_PRINTLN(data_resource);
    
    SERIAL_PRINTLN("Production host, port, URI [command]: ");
    SERIAL_PRINT(cmd_host); 
    SERIAL_PRINT(" , ");
    SERIAL_PRINT(cmd_port);
    SERIAL_PRINT(" , ");
    SERIAL_PRINTLN(cmd_resource); 
    SERIAL_PRINTLN("-----------------------------------------\n");                     
}
 

 
