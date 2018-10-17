// httpPoster.h

#ifndef HTTP_POSTER_H
#define HTTP_POSTER_H

#include "common.h"
#include "config.h"
#include <ESP8266WiFi.h>        // http://arduino.esp8266.com/versions/2.4.1/package_esp8266com_index.json
#include <ESP8266HTTPClient.h>  // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient
 
class HttpPoster {
public:
    HttpPoster();
    void init(Config *configptr);
    bool sendStatus (const char *payload);
    const char* getResponse();
    int getErrorCode();
    
private:
    Config *pC;  
    HTTPClient http_client; 
    int result_code = 0;
    char response [HTTP_RESPONSE_LENGTH];      
};
 
#endif 
