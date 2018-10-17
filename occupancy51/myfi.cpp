// myfi.cpp

#include "myfi.h"

MyFi::MyFi() {
}
 
void MyFi::init(Config* configptr) {
    this->pC = configptr;
    
    // it is important to set STA mode: https://github.com/knolleary/pubsubclient/issues/138
    WiFi.mode(WIFI_STA); 
    //wifi_set_sleep_type(NONE_SLEEP_T);  // revisit & understand this
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    
    wifi_multi_client.addAP(pC->wifi_ssid1, pC->wifi_password1);
    if (strlen(pC->wifi_ssid2) > 0)
        wifi_multi_client.addAP(pC->wifi_ssid2, pC->wifi_password2);   
    
    SERIAL_PRINT ("connecting to SSID: ");
    SERIAL_PRINTLN (pC->wifi_ssid1);
    int MAX_ATTEMPTS = 40;  // 10 sec
    int attempts = 0;
    while(wifi_multi_client.run() != WL_CONNECTED) {   
        delay(250);
        SERIAL_PRINT ("."); 
        attempts++;
        if (attempts >= MAX_ATTEMPTS) {
            SERIAL_PRINTLN ("!\n- Could not connect to Wi-Fi -"); 
            wifi_connected = false;
            return;
        }
    }
    SERIAL_PRINTLN ("!\nWi-Fi connected.");  
    wifi_connected = true;   
} 

bool MyFi::isConnected() {
    return (wifi_connected); 
}

void MyFi::update () {
    wifi_connected = (wifi_multi_client.run()==WL_CONNECTED);
}

void MyFi::disable() {
    SERIAL_PRINTLN ("\nSwitching off wifi..");
    WiFi.disconnect(); 
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    wifi_connected = false;
    delay(10); 
}

