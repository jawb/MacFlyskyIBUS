import sys
import serial
import time
import threading
import signal
import subprocess
import binascii

IBUS_BUFFSIZE = 32
ibusIndex = 0
ibus = [0 for i in range(0, IBUS_BUFFSIZE)]
rcValue = [0 for i in range(0, 8)]
running = True
p = None
t = None

stream = serial.Serial(
    port='/dev/cu.wchusbserial1d1120',\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
    timeout=0)

def process(v):
    global IBUS_BUFFSIZE
    global ibusIndex
    global ibus
    global rcValue
    global p
    if ibusIndex == 0 and v != 0x20:
        return
    if ibusIndex == 1 and v != 0x40:
        ibusIndex = 0
        return
    if ibusIndex < IBUS_BUFFSIZE:
        ibus[ibusIndex] = v
    ibusIndex = ibusIndex + 1

    if ibusIndex == IBUS_BUFFSIZE:
        ibusIndex = 0
        chksum = 0xFFFF
        for i in range(0,30):
            chksum -= ibus[i]
        rxsum = ibus[30] + (ibus[31]<<8)
        if chksum == rxsum:
            rcValue[0] = (ibus[ 3]<<8) | ibus[ 2] 
            rcValue[1] = (ibus[ 5]<<8) | ibus[ 4]
            rcValue[2] = (ibus[ 7]<<8) | ibus[ 6]
            rcValue[3] = (ibus[ 9]<<8) | ibus[ 8]
            rcValue[4] = (ibus[11]<<8) | ibus[10]
            rcValue[5] = (ibus[13]<<8) | ibus[12]
            rcValue[6] = (ibus[15]<<8) | ibus[14]
            rcValue[7] = (ibus[17]<<8) | ibus[16]
            for c in rcValue:
                p.stdin.write(int2bytes(c))

def serial_listener():
    global running
    while running:
        for c in stream.read():
            process(ord(c))
    return

def int2bytes(i):
    hex_string = '%x' % i
    n = len(hex_string)
    return binascii.unhexlify(hex_string.zfill(n + (n & 1)))

def main():
    global t
    global p
    p = subprocess.Popen(['./vjoy'], stdin=subprocess.PIPE)
    t = threading.Thread(target=serial_listener)
    t.start()
    
def signal_handler(signal, frame):
    global running
    global t
    global p
    global stream
    print 'Exit'
    running = False
    t.join()
    p.terminate()
    stream.close()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
main()
signal.pause()
