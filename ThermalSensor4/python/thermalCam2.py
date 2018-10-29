# read from Arduino MLX90621 thermal sensor & display it raw, using Matplotlib
# use it with the Arduino program  MLX90621_Basic3.ino
# uses the new CameraSerial external library (swaps two buffers)
# do not move/resize/obscure the plotting window; it freezes (TODO) 
# color maps: https://matplotlib.org/examples/color/colormaps_reference.html

# python -m serial.tools.list_ports
# python -m serial.tools.miniterm  COM12  115200

import CameraSerial as cs
from random import randint 
from time import sleep
import numpy as np
import sys
import os
import math
import time
from matplotlib import pyplot as plt

MIN_TEMP = 28  # adjust them for your dynamic range
MAX_TEMP = 41

pixels = np.array([randint(MIN_TEMP, MAX_TEMP) for i in range(cs.ROWS*cs.COLS)])
pixels = pixels.reshape(cs.ROWS, cs.COLS)
fig = plt.figure()
ax = fig.add_subplot(111)

# do NOT use flag palette with interpolation !
#img = ax.imshow(pixels, cmap="flag", interpolation='hamming')  # 'none'
#img = ax.imshow(pixels, cmap="flag", interpolation='none')  # 'none'
#img = ax.imshow(pixels, cmap="bwr")  
#img = ax.imshow(pixels, cmap="bwr", interpolation='bicubic')  
#img = ax.imshow(pixels, cmap="bwr", interpolation='bilinear')  
#img = ax.imshow(pixels, cmap="bwr", interpolation='hamming')  
img = ax.imshow(pixels, cmap="bwr", interpolation='catrom')  


plt.title(cs.CAMERA_TYPE)
#plt.set_axis_off()
#plt.clim()   # clamp the color limits
#fig.tight_layout()
plt.show(block=False) # NOTE: non-blocking
       
port = 12
if (len(sys.argv) > 1):
    port = int(sys.argv[1])
print ("Serial port: COM" +str(port))

print ("Camera type: " +cs.CAMERA_TYPE)
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
        print ("min : " +str(min(pixels)) +", max : " +str(max(pixels)))
        pixels = pixels.reshape(cs.ROWS, cs.COLS)
        img.set_array(pixels) 
        fig.canvas.draw()
        
    except KeyboardInterrupt:
        break        
    except Exception as e:        
        print (e)

seri.close()
sleep (1.0)
print("Bye !")
              