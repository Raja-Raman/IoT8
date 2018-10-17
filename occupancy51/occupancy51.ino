// Send simulated data to RaspberryPi gateway
// Respond to commands from the server

#include "occupancy.h"

Timer  T;
Config C;
MyJson J;
MyFi   W;
HttpPoster  P;
CommandHandler H;

// shared globals
int data_timer_id = 0;
boolean  occupied = true;        
int temperature, humidity, heat_index, light;
SensorData  tx_data;   // reusable cache objects
CommandData rx_data;

void setup() {
    randomSeed(micros());
    init_serial();   
    C.init();              // get the device id from flash,...
    C.dump();              // ..have a quick look,.... 
    W.init(&C);            // ...and then pass them on to W, M and O 
    P.init(&C);
    H.init(&C);
    yield();     
    send_restart_message(); // this needs device_id
    data_timer_id = T.every (C.data_interval, send_data);  
}

void loop() {
    T.update();
    W.update();  
}

void execute_command (const char* jsonString) {
    if (strlen(jsonString) == 0)
        return;
    SERIAL_PRINTLN (jsonString);
    rx_data = J.deserialize (jsonString); // store a reference in the global object rx_data
    // NOTE: the actual command object is allocated inside jayson.cpp
    // TODO: make it reentrant; block another callback till you send an ACK to server     
    H.handleCommand(rx_data);
}

void send_restart_message() {
    // device id and group id can change any time in the background, so
    // we need to set them up in the payload every time
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = STATUS_RESTARTED;                   
    tx_data.status_param = FIRMWARE_VERSION;
    //tx_data.heap_space = ESP.getFreeHeap();    
    const char* msg = J.serialize (tx_data);
    if (P.sendStatus(msg)) {
      const char *response = P.getResponse();
      execute_command (response);
    }
}
   
void send_response(long command, int response) {
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = response;                   
    tx_data.status_param = command;  
    const char* msg = J.serialize (tx_data);
    if (P.sendStatus(msg)) {
      const char *response = P.getResponse();
      execute_command (response);
    }
}

// periodic staus updates   
void send_data() {
    tx_data.device_id = C.device_id;
    tx_data.group_id = C.group_id;
    tx_data.node_status = random(1, 5);
    tx_data.status_param = CMDX_REGULAR_DATA;   
    tx_data.temperature = random(19, 36);
    tx_data.humidity = random (0, 101);
    tx_data.light = random (0, 101);   
    tx_data.heat_index = random(19, 40);
    const char* msg = J.serialize (tx_data);
    //SERIAL_PRINT("Json payload size: ");
    //SERIAL_PRINTLN(strlen(msg));
    if (P.sendStatus(msg)) {
      const char *response = P.getResponse();
      execute_command (response);
    }
}

// send cached status with extra data
void send_on_demand_data() {
    send_data();  // TODO: send additional health parameters
}

void reset_data_interval(long new_value) {
  SERIAL_PRINTLN ("reset_data_interval : not implemented");
  // TODO: save the new value in Config object, and restart the timer
}

void init_serial () {
    #ifdef ENABLE_DEBUG
        Serial.begin(C.baud_rate);  
        Serial.setDebugOutput(true);
    #endif    
    SERIAL_PRINTLN("\n\n********************* AWS IoT starting... ********************\n"); 
}

