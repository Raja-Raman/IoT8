// httpPoster.cpp

#include "httpPoster.h"
 
HttpPoster::HttpPoster(){
}

void HttpPoster::init(Config *configptr){
    this->pC = configptr;
}

bool HttpPoster::sendStatus (const char *payload){
  if (WiFi.status() != WL_CONNECTED) {
    SERIAL_PRINTLN("HttpPoster: No Wi-Fi connection");
    return false;
  }
  
  bool server_connected = http_client.begin (pC->http_post_url); // ,pC->thumb_print);
  if (!server_connected) {
    SERIAL_PRINTLN("Could not connect to gateway.");
    return false;
  }
  #ifdef VERBOSE_MODE
    SERIAL_PRINTLN("Connected to gateway.");
  #endif
  
  http_client.addHeader("Content-Type", "application/json");
  result_code = http_client.POST(payload);
  SERIAL_PRINTLN(result_code);
  
  if (result_code < 0) {
    SERIAL_PRINTLN("Cannot POST to gateway !");
    SERIAL_PRINTLN(http_client.errorToString(result_code).c_str());
  } else {
    strncpy (response, http_client.getString().c_str(), HTTP_RESPONSE_LENGTH);
    response [HTTP_RESPONSE_LENGTH-1] = '\0';
    SERIAL_PRINTLN(response);
  }
  http_client.end();   // close the connection
  return (result_code >= 0);
}

const char* HttpPoster::getResponse(){
    return ((const char*) response);
}
 
int HttpPoster::getErrorCode() {
    return (result_code);
} 
     
 
