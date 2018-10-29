//config.cpp
#include "config.h"
#include "flashHelper.h"
///#include "externDeclare.h"

extern void safe_strncpy (char *dest, char *src, int length = MAX_STRING_LENGTH);

Config::Config(){
    SERIAL_PRINTLN ("Creating the Config object.. [Ensure it is a singleton]");
    safe_strncpy (data_prod_url,        DATA_PROD_URL);
    safe_strncpy (data_test_url,        DATA_TEST_URL);  // this will change in init()
    safe_strncpy (cmd_prod_url,         CMD_PROD_URL);
    safe_strncpy (cmd_test_url,         CMD_TEST_URL);
    safe_strncpy (firmware_server_url,  FW_SERVER_URL);
    safe_strncpy (version_check_url,    FW_VERSION_URL); 
}

void Config::init() {
    loadDeviceData ();
    char custom_test_url [MAX_STRING_LENGTH];
    sprintf (custom_test_url, "%s%ld", DATA_TEST_URL, device_id);
    safe_strncpy (data_test_url, custom_test_url);    
    SERIAL_PRINTLN ("Test URL to post data: ");
    SERIAL_PRINTLN (custom_test_url);
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

bool Config::storeDeviceData() {
    SERIAL_PRINTLN ("\nSaving the Configuration to EEPROM..");
    FlashHelper F;
    F.init(this);
    F.begin();
    bool result = F.testMemory();
    if (!result) {
        SERIAL_PRINTLN ("Basic Memory Test failed !!! aborting.");
        F.end();
        return false;
    }
    //yield();
    result = F.writeFlash();
    if (!result) {
        SERIAL_PRINTLN ("\nFATAL ERROR: Could not reliably write to EEPROM. Aborting.");
        F.end();
        return false;
    }    
    F.commit();
    //yield();
    SERIAL_PRINTLN ("\nReading back memory..\n");
    result = F.readFlash();
    F.end();   
    if (!result) {
        SERIAL_PRINTLN ("\nFATAL ERROR: Could not read from EEPROM. Aborting.");
        return false;
    }
    // data is now stored back in 'this' object
    SERIAL_PRINTLN ("This configuration has been saved in EEPROM: ");    
    SERIAL_PRINT ("Device ID: ");
    SERIAL_PRINTLN (device_id);     
    SERIAL_PRINT ("Group ID: ");
    SERIAL_PRINTLN (group_id);
    SERIAL_PRINT ("Relay enabled?: ");
    SERIAL_PRINTLN (relay_enabled);
    SERIAL_PRINT ("Buzzer enabled?: ");
    SERIAL_PRINTLN (buzzer_enabled);  
    SERIAL_PRINTLN();
    return true;
}

void Config::repairFlash(const char *config_str) {
  SERIAL_PRINTLN("New Flash data:");
  SERIAL_PRINTLN(config_str);
  long did,gid, re,be;
  int num_ints = sscanf (config_str, "%ld %ld %ld %ld", &did,&gid, &re,&be);
  SERIAL_PRINT("number of integers scanned= ");
  SERIAL_PRINTLN(num_ints);  
  SERIAL_PRINT("device id= ");
  SERIAL_PRINTLN(did);  
  SERIAL_PRINT("group id= ");
  SERIAL_PRINTLN(gid);  
  SERIAL_PRINT("relay enabled= ");
  SERIAL_PRINTLN(re);  
  SERIAL_PRINT("buzzer enabled= ");
  SERIAL_PRINTLN(be);  
  if (num_ints != 4) {
      SERIAL_PRINT("ERROR: Expected 4 integers, but found ");
      SERIAL_PRINTLN(num_ints);
      return;
  }
  device_id = did;
  group_id = gid;
  relay_enabled = re;
  buzzer_enabled = be;
  storeDeviceData();  // manually restart after this
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
 

 
