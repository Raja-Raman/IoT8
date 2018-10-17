// myfi.h

#ifndef MYFI_H
#define MYFI_H

#include "common.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

class MyFi {
public:
    MyFi();
    void init(Config* configptr);
    void update();
    void disable();
    bool isConnected();
private:
    Config *pC;
    ESP8266WiFiMulti wifi_multi_client;
    bool wifi_connected;
};

#endif 
