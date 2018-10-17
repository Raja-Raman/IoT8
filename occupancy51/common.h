//common.h
#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
extern "C" {
  #include "user_interface.h"
}

// comment out this line to disable some informative messages
#define  VERBOSE_MODE 

// comment out this line to disable all serial messages
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define  SERIAL_PRINT(x)       Serial.print(x)
  #define  SERIAL_PRINTLN(x)     Serial.println(x)
  #define  SERIAL_PRINTLNF(x,y)  Serial.println(x,y)   
  #define  SERIAL_PRINTF(x,y)    Serial.printf(x,y) 
#else
  #define  SERIAL_PRINT(x)
  #define  SERIAL_PRINTLN(x)
  #define  SERIAL_PRINTLNF(x,y)
  #define  SERIAL_PRINTF(x,y)
#endif

#define  MAX_STRING_LENGTH      128
#define  HTTP_RESPONSE_LENGTH   128

#define  STATUS_NONE            0
#define  STATUS_OCCUPIED        1
#define  STATUS_FREE            2
#define  STATUS_WARNED1         3
#define  STATUS_WARNED2         4
#define  STATUS_RESTARTED       5
#define  STATUS_SENSOR_ERROR    6
#define  EPROM_ERROR            7
#define  STATUS_CMD_ACK         8
#define  STATUS_CMD_SUCCESS     9
#define  STATUS_CMD_FAILED      10

// Tx data structure: the comments are json tags sent to the server
// Battery voltage cannot be included here, as the ADC pin is used for input
struct SensorData {  
   long device_id;              // ID
   long group_id;               // G
   int node_status;             // S
   int status_param;            // P
   int temperature;             // T
   int humidity;                // H
   int heat_index;              // I
   int light;                   // L       
};

// when command parsing fails, the command will be 0. This is sent to the
// null sink 'CMD_HELLO' where it is ignored.

#define  CMD_HELLO                0
#define  CMD_GET_DATA             1
#define  CMD_SET_THRESHOLD        2   
#define  CMD_SET_DATA_INTERVAL    3  
#define  CMD_TARGET_PROD_URL      4
#define  CMD_TARGET_TEST_URL      5
 
#define  CMD_SET_DEVICE_ID        6
#define  CMD_SET_GROUP_ID         7
#define  CMD_SET_FW_SERVER_URL    8
#define  CMD_UPDATE_FIRMWARE      9
#define  CMD_WRITE_FLASH_CONFIG   10
#define  CMD_RESTART              11

#define  CMDX_PARSING_ERROR       12
#define  CMDX_REGULAR_DATA        13
#define  CMDX_EVENT_DATA          14

// CMDX_PARSING_ERROR is generated internally by the Json parser
// CMDX_REGULAR_DATA is a dummy command used by the device to indicate it is a routine packet, and not an event packet
// CMDX_EVENT_DATA is when an occupancy status change or sensor event happens

// TODO: make this a class with a copy constructor
//  Rx data structure: the comments are json tags to be sent by the server 
struct CommandData {  
   long device_id;                      // ID
   long group_id;                       // G
   long command;                        // C
   long long_param;                     // L
   char string_param[MAX_STRING_LENGTH];// S    
};

#endif

/********************************  
cheat sheet:
pin mapping of 8266 -  Dx -> GPIO pins:
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
*********************************/

  
