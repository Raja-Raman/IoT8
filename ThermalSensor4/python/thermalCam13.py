# read MLX90621 thermal sensor & detect objects by HSV color tracker
# use it with the Arduino program  MLX90621_Basic3.ino
# uses CameraSerial, a customized python class for the MLX90621 sensor
# https://www.pyimagesearch.com/2015/09/21/opencv-track-object-movement/
# https://www.learnopencv.com/color-spaces-in-opencv-cpp-python/

# python -m serial.tools.list_ports
# python -m serial.tools.miniterm  COM12  115200

from matplotlib import pyplot as plt
import CameraSerial as cs
from time import sleep
import numpy as np
import math
import time
import sys
import cv2
import os
import io

# config parameters  ---------------------------------------------------
port = 12
PALETTE = 1
INTERPOLATION = 2
MIN_TEMP = 28  # adjust this for your dynamic range
MAX_TEMP = 41  # adjust this for your dynamic range

# * Assuming bwr palette *
# H ranges from 0 to 180 degrees
# S and V range from 0 to 255
 
#------------------------------------------------
# the following values worked well for bwr palette:
MIN_RED = np.array([0, 100,  100], dtype = "uint8")
MAX_RED = np.array([10, 255, 255], dtype = "uint8")
#------------------------------------------------- 
'''
MIN_RED = np.array([0, 120, 150], dtype = "uint8")
MAX_RED = np.array([10, 255, 255], dtype = "uint8")
'''  
# constants  -----------------------------------------------------------
# https://matplotlib.org/gallery/images_contours_and_fields/interpolation_methods.html
interpolation_strings = ['none', 'bilinear', 'bicubic', 'quadric', 'gaussian' ,
    'spline16', 'spline36', 'hanning', 'hamming', 'hermite', 
    'kaiser', 'catrom', 'bessel', 'mitchell', 'sinc', 
    'lanczos', 'nearest']

# https://matplotlib.org/users/colormaps.html
palette_strings = ['flag', 'bwr', 'coolwarm', 'RdYlGn', 'seismic', 
    'BrBG', 'RdGy', 'winter', 'YlOrRd', 'YlOrBr',
    'Greys', 'Greens', 'Oranges', 'Reds', 'bone', 
    'RdYlBu', 'cool', 'gnuplot']

# configure----------------------------------------------------------------

print ('Usage: python ThermalCam.py [serial_port] [interpolation] [color_palette] [min_temperature] [max_tempeature]')

print ("Thermal camera type: {}".format(cs.CAMERA_TYPE))

if (len(sys.argv) > 1):
    port = int(sys.argv[1])
print ("Serial port: COM{}".format (port))

if (len(sys.argv) > 2):
    INTERPOLATION = int(sys.argv[2])
print ("Interpolation : {}".format (interpolation_strings[INTERPOLATION]))

if (len(sys.argv) > 3):
    PALETTE = int(sys.argv[3])
    print ("WARNING: If you change the palette, you must change the HSV filter also")
print ("Color palette : {}".format (palette_strings[PALETTE]))

if (len(sys.argv) > 4):
    MIN_TEMP = int(sys.argv[4])
print ("Minimum temperature : {}".format (MIN_TEMP))

if (len(sys.argv) > 5):
    MAX_TEMP = int(sys.argv[5])
print ("Maximum temperature : {}".format (MAX_TEMP))
  
#TODO:
# set MIN_RED and MAX_RED also   

# main  ------------------------------------------------------------------

pixels = np.zeros ((cs.ROWS, cs.COLS), dtype=float)
plt.figure()
plt.axis("off")

img = plt.imshow(pixels, 
        cmap=palette_strings[PALETTE], 
        interpolation=interpolation_strings[INTERPOLATION],
        vmin = MIN_TEMP, vmax = MAX_TEMP)  
      
seri = cs.CameraSerial()
# without timeout, packets are fragmented
if not seri.open(port, timeout=0.5):   # assuming packet interval of 250 or 330 mSec
    print ('Cannot open serial port.')
    sys.exit(1)
seri.start()

#let the sensor initialize
time.sleep(.2)

previous_flag = True
print ('Press ^C to exit...')
while True:
    try:
        if (previous_flag == cs.use_first_buffer):  # no new data available
            sleep(0.1)
            continue
        if (cs.use_first_buffer):
            pixels = cs.buffer1  # not a deep copy; just points to the same buffer *
        else:
            pixels = cs.buffer2
        previous_flag = cs.use_first_buffer
        
        pixels = np.array (pixels)
        print ("min : {} , max : {}".format (min(pixels), max(pixels)))
        pixels = pixels.reshape(cs.ROWS, cs.COLS)
         
        img.set_array(pixels) 
        
        #------------------------------------------ 
        # this also works:
        #plt.savefig('frame.png', bbox_inches='tight')
        #frame = cv2.imread('frame.png')
        #-------------------------------------------
        
        # using in-memory file:
        buf = io.BytesIO()
        plt.savefig (buf, format='png')
        buf.seek(0)
        rgb_frame = np.asarray (bytearray(buf.read()), dtype=np.uint8)
        rgb_frame = cv2.imdecode (rgb_frame, cv2.IMREAD_COLOR)
        buf.close()
        
        hsv_frame = cv2.cvtColor (rgb_frame, cv2.COLOR_BGR2HSV)       
        red_pixels = cv2.inRange (hsv_frame, MIN_RED, MAX_RED)  
        
        red_pixels = cv2.GaussianBlur(red_pixels, (11, 11), 0)   
        red_pixels = cv2.erode(red_pixels, None, iterations=3)
        red_pixels = cv2.dilate(red_pixels, None, iterations=3)
        
        # finding contours destroys the original image, so make a copy if you need it later
        (junk_img, contours, hierarchy) = cv2.findContours (red_pixels,   # red_pixels.copy(), 
                                          cv2.RETR_EXTERNAL, 
                                          cv2.CHAIN_APPROX_SIMPLE)
        ccount = len(contours)
        print ("{} contours found.".format(ccount))
        if (ccount > 0):
            cntrs = sorted(contours, key = cv2.contourArea, reverse = True)
            cv2.drawContours(rgb_frame, cntrs, -1, (0, 255, 0), 2)
            #rect = np.int32(cv2.BoxPoints(cv2.minAreaRect(cntrs)))
            #cv2.drawContours(rgb_frame, [rect], -1, (0, 255, 0), 2)
            i = 0
            for c in cntrs:
                (x,y,w,h) = cv2.boundingRect(c)
                #cv2.rectangle(rgb_frame, (x,y),(x+w,y+h), (0, 255, 0), 2)
                cv2.putText(rgb_frame, str(i+1), (x,y), 
                            cv2.FONT_HERSHEY_SIMPLEX, 
                            0.8, (0,0,0), 1, cv2.LINE_AA)  
                i += 1
        cv2.imshow("Thermal Camera", rgb_frame)
        key = cv2.waitKey(100) & 0xFF  # TODO: revisit the delay
        if (key==27): 
            break       
    except Exception as e:        
        print (e)

seri.close()
sleep (1.0)
cv2.destroyAllWindows()
print("Bye !")

#================================================================================================
'''
// MLX90621_Arduino_Processing code is downloaded  from
// https://github.com/robinvanemden/MLX90621_Arduino_Processing
// and modified by Rajaraman in Oct 2018 to replace i2c_t3 library with Arduino Wire library

#include <Arduino.h>
#include "MLX90621.h"

MLX90621 sensor;  
bool verbose = false;

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
'''


              