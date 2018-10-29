# Serial helper class with double buffers 
# Specially designed to read thermal cameras

# python -m serial.tools.list_ports
# python -m serial.tools.miniterm  COM4  115200

from time import sleep
import numpy as np
import threading
import serial
import sys

class CameraSerial (threading.Thread):

    def open(self, port=1, baud=115200, timeout=0):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self.ser = None
        self.datastr = "DataStringPlaceHolder"  
        try:
            print ('Opening serial port...')
            self.ser = serial.Serial('COM'+str(self.port), self.baud, timeout=self.timeout)
            #print (self.ser)
            #print("ROWS: %d" %ROWS)
            #print("COLS: %d" %COLS)
            return True
        except Exception as e:
            print (e)   
            print (self.ser)
            return False     
        
        
    def run(self):
        try:
            if self.ser is None:
                print ("Serial port is not open[1]")
                return   
        except Exception as e:
            print (e) 
            print ("Serial port is not open[2]")
            return 
        self.ser.flushInput()
        while not terminate:
            try:
                if (self.ser is None):
                    if not self.open(self.port, self.baud, self.timeout):
                        for i in range(10):
                            if terminate: break
                            sleep(1)             
                        continue
                #if self.ser.inWaiting():
                self.datastr = self.ser.readline()
                if (self.datastr is None or len(self.datastr)==0):
                    continue
                #print (len(self.datastr))
                #print (type(self.datastr))                
                #print (self.datastr)
                self.process()
            except serial.serialutil.SerialException:  # this gives it a chance to reopen the port
                print('- Cannot read serial port -')
                if (self.ser is not None):
                    self.ser.close()  # Note: do not use self.close() ! it will set terminate=True
                    self.ser = None   # this is needed for reattempts
            except Exception as e:
                print(e)
        self.close()
        print ('Serial thread exits.')
                
                
    def close(self):  
        global terminate    # make this local ?
        terminate = True    # ASSUMPTION: there are no other threads using this variable
        print ('Closing serial port...')
        try:    
            if self.ser is None:
                print ("Serial port is not open[3]")
                return   
            if not self.ser.isOpen():
                print ("Serial port is not open[4]")
                return  
        except Exception as e: 
            print (e) 
            print ("Serial port is not open[5]")
            return 
        try :       
            self.ser.flushOutput()
            self.ser.close()
            self.ser = None
        except Exception as e:
            print (e)  
            
        
    # if the serial packets are not very frquent, it may be good to delegate our
    # data processing to the worker thread, instead of the main thread. TODO: revisit
    def process(self):
        global buffer1, buffer2, use_first_buffer
        #print ('Processing..')
        try:
            #print(self.datastr)
            # The string is of the form 'b 99 99 99 ... 99\n'   including the single quotes !   
            # Note: this is applicable for Python 3; But Python 2 continues to be without the 'b  
            
            self.datastr = self.datastr.decode('UTF-8')  # convert bytes to chars 
            #self.datastr = self.datastr.strip()   
            if (use_first_buffer):   # fill the secondary buffer            
                buffer2 = [float(n) for n in self.datastr.split()]  # int(float(n))
                #print (len(buffer2))
                if (len(buffer2) != ROWS*COLS):
                    print ('- Data error 1 -')
                    return
                buffer2 = np.array (buffer2)
                #buffer2 = buffer2.reshape(ROWS, COLS)
                #print (buffer2.shape)
                use_first_buffer = False
            else:
                buffer1 = [float(n) for n in self.datastr.split()]  # int(float(n))
                #print (len(buffer1))
                if (len(buffer1) != ROWS*COLS):
                    print ('- Data error 2 -')
                    return
                buffer1 = np.array (buffer1)
                #buffer1 = buffer1.reshape(ROWS, COLS)
                #print (buffer1.shape)
                use_first_buffer = True
        except Exception as e:
            print (e)              
            
#----------------------------------------------------------------------------------------------------------
# place holder

def process_image (pixels):
    print (pixels)
    print ("min: %d" %min(pixels))
    print ("max: %d" %max(pixels))    
    print()
#--------------------------------------------------------------- 
# globally shared
terminate = False   # make this local ?

'''
CAMERA_TYPE = 'CAM_90640' 
ROWS = 24   
COLS = 32
'''
CAMERA_TYPE = 'CAM_90621' 
ROWS = 4   
COLS = 16

buffer1 = []
buffer2 = []
use_first_buffer = True

# https://stackoverflow.com/questions/419163/what-does-if-name-main-do
def main():
    port = 12
    if (len(sys.argv) > 1):
        port = int(sys.argv[1])
    print ("Serial port: COM" +str(port))
    
    seri = CameraSerial()
    # without timeout, packets are fragmented
    if not seri.open(port, timeout=0.5):   # assuming packet interval of 250 or 330 mSec
        print ('Cannot open serial port.')
        sys.exit(1)
    seri.start()
                
    previous_flag = True
    print ('Press ^C to exit...')
    while True:
        try:
            if (previous_flag == use_first_buffer):  # no new data available
                sleep(0.1)
                continue
            if (use_first_buffer):
                pixels = buffer1  # not a deep copy; just points to the same buffer *
            else:
                pixels = buffer2
            previous_flag = use_first_buffer
            
            process_image (pixels) # place holder
            
        except KeyboardInterrupt:
            break        
        except Exception as e:        
            print (e)
    
    #terminate = True
    seri.close()
    sleep (1.0)
    print("Bye !")       
     
if __name__ == "__main__":
    main()   
      