/*
    TODO: 
    surround all constant strings with F()
    The JsonParser is very heavy. Implement your own.
    Bring all the HTTP work in OTA helper into HttpPoster class
    Develop the utility class with other utility functions
    Disable serial port and test it.
*/
#include "occupancy.h"

//#define  SUMULATION_MODE

Config C;
Timer T;
MyJson  J;
Hardware H; 
OtaHelper O;
////CommandHandler CMD;
////WifiPoster POSTER; 
HttpPoster POSTER; 

SensorData  tx_data;
////CommandData rx_data;

// shared globals
int data_timer_id = 0;
boolean  occupied = true;   
unsigned int pir_tick_counter = 0;
unsigned int radar_tick_counter = 0;

void setup() {
    init_serial ();
    C.init();
    C.targetTestUrl ();
    C.dump();   
    H.init(&C);
    H.switchLightsOn(); // light should be ON at startup    
    POSTER.init(&C);
    O.init(&C);
    ////CMD.init(&C);
    yield();  
    H.blinker();  // includes a random startup delay
    T.after (10000L, send_restart_message);
    T.every (C.data_interval,  send_status);
    T.every (20 * C.data_interval + 500, check_for_updates);  
    T.every (C.tick_interval, ticker);    
    T.every (C.sensing_interval, read_sensors);
    ////T.every (C.command_interval,check_for_commands);
    SERIAL_PRINTLN("Setup complete.");
}

void loop() {
    T.update();
}  

void ticker () {
    H.readPM();  // read PIR and Radar
    update_status();
}

// Uses shared variables; Call read_sensors before calling this !
void update_status() {    
    pir_tick_counter++;      
    radar_tick_counter++;    
    if (!occupied) {
        if (H.pir_status && H.radar_status)    // both the sensors fired, so occupy the room
            occupy_room();
    }
    if (radar_tick_counter == C.buzzer_ticks1) {
        if (occupied)
            warn(STATUS_WARNED1);  // warn about the imminent release
    }
    if (pir_tick_counter == C.buzzer_ticks2) {
        if (occupied)
            warn(STATUS_WARNED2);  // warn about the imminent release
    }
    if (radar_tick_counter >= C.release_ticks1){  // Even the radar was silent for 2 minutes.
         radar_tick_counter = 0; // -> to avoid counter overflow during nights/holidays
         if (occupied)
            release_room();  
    }
    if (pir_tick_counter >= C.release_ticks2){  // PIR was silent for 10 minutes.
         pir_tick_counter = 0; // -> to avoid counter overflow during nights/holidays
         if (occupied)
            release_room();              
    }  
}

// Read temperature, humidity and light
void read_sensors () {
    H.readTHL();  // temp, humidity, light are stored inside H
}

void occupy_room() {
    occupied = true; 
    H.switchLightsOn(); 
    SERIAL_PRINTLN ("Room is now ccupied.");    
    send_event(STATUS_OCCUPIED); 
}

// pre-release warning flashes
void warn(byte warning_type) { 
    SERIAL_PRINTLN ("About to release the room.."); 
    H.warn(T);
    send_event(warning_type); 
}

void release_room() {
    occupied = false; 
    H.switchLightsOff();      
    SERIAL_PRINTLN ("Room is now released.");
    send_event(STATUS_FREE); 
}
//------------------------------------------------------------------------------
void send_restart_message() {
    SERIAL_PRINTLN("\n-------------------->>> sending restart msg...\n");  
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = STATUS_RESTARTED;                   
    tx_data.status_param = C.current_firmware_version; // it is not CMDX_REGULAR_DATA *  
    const char* msg = J.serialize (tx_data);
    POSTER.sendStatus(msg);
}

int packet_count = 0;  
#ifdef SUMULATION_MODE
// periodic staus updates   
void  send_status() {
    SERIAL_PRINT("------>>> sending simulated data. Pkt# =");
    SERIAL_PRINTLN(packet_count);
    packet_count = (packet_count+1)%10;
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = random(1, 4);
    tx_data.status_param = CMDX_REGULAR_DATA;   
    tx_data.temperature = random(19, 36);
    tx_data.humidity = random (10, 90);
    tx_data.light = random (10, 90);   
    tx_data.heat_index = random(19, 40);
    const char* msg = J.serialize (tx_data);
    SERIAL_PRINTLN(msg);
//    SERIAL_PRINT("Json payload size: ");
//    SERIAL_PRINTLN(strlen(msg));
    int result = POSTER.sendStatus(msg);
    SERIAL_PRINT ("Http post result: ");
    SERIAL_PRINTLN(result);
}   
#else
void  send_status() {
    SERIAL_PRINT("------>>> sending data. Pkt# =");
    SERIAL_PRINTLN(packet_count);
    packet_count = (packet_count+1)%10;
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = (occupied ? STATUS_OCCUPIED : STATUS_FREE);
    tx_data.status_param = CMDX_REGULAR_DATA;   
    tx_data.temperature = H.temperature;
    tx_data.humidity = H.humidity;
    tx_data.light = H.light;   
    tx_data.heat_index = H.heat_index;
    const char* msg = J.serialize (tx_data);
    SERIAL_PRINTLN(msg);
//    SERIAL_PRINT("Json payload size: ");
//    SERIAL_PRINTLN(strlen(msg));
    int result = POSTER.sendStatus(msg);
    SERIAL_PRINT ("Http post result: ");
    SERIAL_PRINTLN(result);
} 
#endif

// when an event happens, notify immediately
void send_event (int status_code) {
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = status_code;
    tx_data.status_param = CMDX_EVENT_DATA;
    const char* msg = J.serialize (tx_data);
    SERIAL_PRINTLN("Main: sending event data..");    
    SERIAL_PRINT("Json payload size: ");
    SERIAL_PRINTLN(strlen(msg));
    int result = POSTER.sendStatus(msg);
    SERIAL_PRINT ("Http post result: ");
    SERIAL_PRINTLN(result);
}

/***
// externally called from CommandHandler
void send_response(long command, int response) {
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = response;                   
    tx_data.status_param = command;  
    const char* msg = J.serialize (tx_data);
    int result = POSTER.sendStatus(msg);
    SERIAL_PRINT ("Http post result: ");
    SERIAL_PRINTLN(result); 
}

void check_for_commands() {
    int result = POSTER.checkForCommand();
    SERIAL_PRINT ("check_for_commands result: ");
    SERIAL_PRINTLN(result);   
    const char* cmd = POSTER.getCommand();
    SERIAL_PRINT ("Command from serer: ");
    SERIAL_PRINTLN(cmd);     
    execute_command (cmd);
}
void execute_command (const char* jsonString) {
    if (strlen(jsonString) == 0)
        return;
    SERIAL_PRINTLN (jsonString);
    rx_data = J.deserialize (jsonString); // rx_data is a global cache
    // NOTE: the actual command object is allocated inside jayson.cpp
    // TODO: make it reentrant; block another callback till you send an ACK to server     
    CMD.handleCommand(rx_data);
}
***/

// Call this atleast once in a day. Otherwise your device will be brick !
void check_for_updates() {
    SERIAL_PRINTLN ("\n<<<<<<---------  checking for FW updates... ----------->>>>>>\n");
    int result = O.check_and_update();  // if there was an update, this will restart 8266
    SERIAL_PRINT ("OtaHelper: response code: ");  
    SERIAL_PRINTLN (result);
}

void init_serial () {
    #ifdef ENABLE_DEBUG
        //Serial.begin(C.baud_rate);  // there is no C !
        Serial.begin(115200); 
        #ifdef VERBOSE_MODE
           Serial.setDebugOutput(true);
        #endif
        Serial.setTimeout(250);
    #endif    
    SERIAL_PRINTLN("\n\n********************* Vz IoT starting... ********************\n"); 
}
