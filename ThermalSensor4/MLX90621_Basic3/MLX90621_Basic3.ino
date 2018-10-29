
// MLX90621_Arduino_Processing code is downloaded  from
// https://github.com/robinvanemden/MLX90621_Arduino_Processing
// and modified by Rajaraman in Oct 2018 to replace i2c_t3 library with standard Wire library

// Original code and discussion at:
// http://forum.arduino.cc/index.php/topic,126244.msg949212.html#msg949212

/*
* Connection Instructions:
* Connect the Anode of a Silicon Diode to 3V Pin of Teensy. The Diode will drop ~0.7V, so the Cathode will be at ~2.7V. 
* These 2.7V will be the supply voltage "VDD" for the sensor.
* Plug in the USB and measure the supply voltage with a multimeter! - it should be somewhere between 
* 2.5V and 2.75V, else it will fry the sensor !
* ...disconnect USB again...
* Connect Teensy Pin 18 to 2.7V with a 4.7kOhm Resistor (Pullup)
* Connect Teensy Pin 19 to 2.7V with a 4.7kOhm Resistor (Pullup)
* Connect I2C Data (SDA) and Clock (SCK) Pins of Sensor with 4.7kOhm Pullup reistors
* Connect GND and 2.7V with a 100nF ceramic Capacitor.
* Connect the VSS Pin of the Sensor to GND.
* Connect the VDD Pin of the Sensor to 2.7V
 */

#include <Arduino.h>
#include "MLX90621.h"

MLX90621 sensor; // create an instance of the Sensor class
bool verbose = true;

void setup(){ 
    Serial.begin(115200);     
    if (verbose)
        Serial.println("\nThermal camera 90621 starting...");
    sensor.initialise (16); // start the thermo cam with 8 frames per second
    //sensor.initialise (4); // start the thermo cam with 2 frames per second
    if (verbose)    
        Serial.println("Sensor initialized.");
}

void loop(){
    sensor.measure(); //get new readings from the sensor
    for(int y=0;y<4;y++) {    //go through all the 4 rows
        for(int x=0;x<16;x++) {   //go through all the 16 columns
            double tempAtXY= sensor.getTemperature(y+x*4); // extract the temperature at position x/y
            Serial.print(tempAtXY);
            Serial.print(" ");
        }
    }
    Serial.println("");
    //delay(31);
    delay(200);
};



