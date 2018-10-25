// hardware.cpp

#include "hardware.h"

#define DHT_PIN  12    // D6,  CN3
DHT D (DHT_PIN, DHT11); 
    
Hardware::Hardware() {
}
    
void Hardware::init(Config* configptr) {
    this->pC = configptr;
    pinMode(relay, OUTPUT);   
    digitalWrite(relay, HIGH);  // active high; start in ON position    
    pinMode(buzzer, OUTPUT);   
    digitalWrite(buzzer, HIGH);  // active low    
    pinMode(led1, OUTPUT);     
    pinMode(led2, OUTPUT);      
    digitalWrite(led1, HIGH);  // active low
    digitalWrite(led2, HIGH);
    pinMode(pir, INPUT);      
    pinMode(radar, INPUT);          
    D.begin();  // start DHT sensor (it sets the pin mode)         
}
 
void Hardware::readPM() {
    pir_status = digitalRead(pir);   
    radar_status = digitalRead(radar);
    digitalWrite (led1, !pir_status);  // active low
    digitalWrite (led2, !radar_status);
}

#ifdef VERBOSE_MODE
  char display_str[64];
#endif  
int N = 4;
// Reading temperature or humidity takes about 250 milliseconds!
void Hardware::readTHL() {
    temperature = 0;   
    humidity = 0;
    light = 0;
    int t,h;
    
    int num_readings = 0;
    for (int i=0; i<2*N; i++) {
        t = D.readTemperature();  // Celsius
        if (isnan(t) || t < 5 || t > 60) continue;
        else {
            temperature += t;
            num_readings++;
        }
        if (num_readings >= N) break;
        delay(10);
    }
    if (num_readings==0)
        temperature = 0;
    else        
        temperature = (int)temperature/num_readings+0.5; 
    yield();
    
    num_readings = 0;
    for (int i=0; i<2*N; i++) {    
          h = D.readHumidity();
          if (isnan(h) || h < 0 || h > 100) continue;
          else {
              humidity += h;
              num_readings++;
          }
          if (num_readings >= N) break;
          delay(10);
      }
      if (num_readings==0)
           humidity = 0;
      else              
           humidity = (int)humidity/num_readings+0.5;      
      heat_index = D.computeHeatIndex (temperature, humidity, false);
      yield();
    
    for (int i=0; i<N; i++) {           
        light += analogRead(ldr);   // 10 bit analog
        delay(10);
    }
    light = (int)light/N +0.5;  
    
    #ifdef VERBOSE_MODE    
      sprintf (display_str, 
              "Temperature: %d  Humidity: %d  Heat_Index: %d  Light: %d",
              temperature, humidity, heat_index, light);              
      SERIAL_PRINTLN(display_str);
    #endif
}

void Hardware::switchLightsOn() {
    if (pC->relay_enabled) {
        digitalWrite (relay, RELAY_ON);
        SERIAL_PRINTLN("Relay is ON.");
    }
}

void Hardware::switchLightsOff() {
    if (pC->relay_enabled) {
        digitalWrite (relay, RELAY_OFF);   
        SERIAL_PRINTLN("Relay is OFF."); 
    }      
}

void Hardware::beep(Timer& T){
    if (pC->buzzer_enabled)  
        T.pulse(buzzer, BEEP_DURATION, HIGH);
}
    
void Hardware::warn(Timer& T) {
    SERIAL_PRINTLN("10 seec warning before releaseing !");
    if (pC->buzzer_enabled)
          T.oscillate(buzzer, 70, HIGH, 4);  // HIGH = end state is led OFF
//    else
//        T.oscillate (led2,500, HIGH, 10);  // this may not be useful inside a black box
}

// includes a random startup delay to stagger the packets ***
void Hardware::blinker() {
   SERIAL_PRINTLN("hardware blinker...");
    for (int i=0; i<5; i++) {
        digitalWrite(led1, LOW); 
        delay(200);
        digitalWrite(led1, HIGH); 
        delay(200);    
    }
    for (int i=0; i<12; i++) {
        digitalWrite(led2, LOW); 
        delay(50);
        digitalWrite(led2, HIGH); 
        delay(50);    
    }
    // stagger the data packets across devices
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    delay(random(2, 5000));
    digitalWrite(led1, HIGH);    
    digitalWrite(led2, HIGH); 
}
