//config.h
#ifndef CONFIG_H
#define CONFIG_H
 
#include "common.h"
#include "keys.h"

// increment this number for every version
#define  FIRMWARE_VERSION      1
#define  FW_SERVER_URL        "http://192.168.0.100:8000/ota/occupancy.bin"
#define  FW_VERSION_URL       ""


#define  BROADCAST_DEVICE_ID   0
#define  BROADCAST_GROUP_ID    0

class Config {
public :
int   current_firmware_version =  FIRMWARE_VERSION;  

char firmware_server_url [MAX_STRING_LENGTH];
char version_check_url [MAX_STRING_LENGTH];
bool verison_check_enabled = false;

const char * gateway_prod_url  = "http://192.168.0.100:8000/status";
const char * gateway_test_url  = "http://192.168.0.100:8000/test";
char *http_post_url = (char *)gateway_prod_url;

// the actual IDs and threshold will be read from EEPROM and plugged in here
long device_id = BROADCAST_DEVICE_ID;      
long group_id  = BROADCAST_GROUP_ID;
long threshold = 0L;

// * Timer durations SHOULD be unsigned LONG int, if they are > 16 bit! 
unsigned long sensing_interval = 55270L;   // milliSec, to read temperature,humidity etc
unsigned long data_interval    = 60000L;   // milliSec, to send data to gateway

unsigned int tick_interval     = 100;      // in milliSec; 10 ticks = 1 second 
unsigned int release_ticks1    = 120*10;   // n*10 ticks = n seconds   // 60
unsigned int release_ticks2    = 480*10;   // n*10 ticks = n seconds   // 600
unsigned int buzzer_ticks1     = 110*10;   // n*10 ticks = n seconds   // 50
unsigned int buzzer_ticks2     = 470*10;   // n*10 ticks = n seconds   // 590

/* The following constants should be updated in  "keys.h" file  */
const char *wifi_ssid1        = WIFI_SSID1;
const char *wifi_password1    = WIFI_PASSWORD1;
const char *wifi_ssid2        = WIFI_SSID2;
const char *wifi_password2    = WIFI_PASSWORD2;

// serial
long baud_rate = 115200L;   // 9600L  

Config();
void init();
void dump();
void loadDeviceData (); 
void targetProdUrl ();
void targetTestUrl ();
};  
#endif 
 
