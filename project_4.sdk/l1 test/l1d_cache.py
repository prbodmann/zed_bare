# Python script for performing serial communication in radiation experiments
# Author: Lucas Antunes Tambara (lucas.a.tambara@gmail.com)
# Date: May 2016

from __future__ import print_function

# imports (includes)
import os
import sys
import time
import serial
from time import sleep
import io
from datetime import datetime
import struct
from array import *
#from bitstring import *
import subprocess
import switch
from os.path import basename
global serial_port
# methods
def write_logs(str_in):
    global serial_port
    print(str(getTime()) +" "+ basename(serial_port) + " " + str_in, end = '\n')
    log_outf.write(str(getTime()) +" "+ basename(serial_port) + " " + str_in + '\n')
    log_outf.flush()
    return

def getTime():
    return datetime.now().strftime("%d-%m-%Y %H-%M-%S")

def porZynq():

    global s

    global port
    s.cmd(port, 'Off')
    sleep(2)
    #print("CLICK")
    s.cmd(port, 'On')
    sleep(2)
    #print("UNCLICK")
    return

def reset():
    global serial_interface
    write_logs("[INFO] resetting the device...")
    #serial_interface.close() # close the serial before reconfiguring the fpga
    porZynq()
    #subprocess.call(['xsdb','script.tcl',board_serial, execcode, bitstream])
    # open the serial after reconfiguring the fpga
    serial_interface = serial.Serial(port = serial_port, baudrate = serial_baud, bytesize = serial.EIGHTBITS, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, timeout = 20)
    # initialize the test again
    #sem_ip_state = 0

# open log
filename_inj = 'logs/log_' + datetime.now().strftime("%d-%m-%Y_%H-%M-%S") + '.txt'
filemode_inj = 'a'
log_outf = open(filename_inj, filemode_inj)


serial_port = sys.argv[1] # serial port to read data from
s = switch.Switch(sys.argv[2],4)
port= int(sys.argv[3])

serial_baud = 115200 # baud rate for serial port

#configure FPGA
write_logs("[INFO] starting radiation experiment...")
write_logs( "[INFO] configuring device...")
reset()

########################
#serial port parameters

########################

serial_interface = serial.Serial(port = serial_port, baudrate = serial_baud, bytesize = serial.EIGHTBITS, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, timeout = 20)

# variables
sem_ip_state = 0
read_data_serial = []
exit_script = 0
total_bitflips = 0
total_bits_evaluated = 0
num_words =0
while(exit_script == 0):
    timeout_read = 1
    if(sem_ip_state == 0):
        for c in serial_interface.read():
            read_data_serial.append(chr(c))
            #print(c)
            if(chr(c) == '\r'):
                read_data_serial_string = ''.join(str(v) for v in read_data_serial)
                #print(read_data_serial_string)
                read_data_serial_string.replace("\r", "")
                if(total_bitflips == 2):
                    write_logs( "[ZYNQ-7000] " + read_data_serial_string)
                    sem_ip_state = 0
                if(total_bitflips == 1):
                    sem_ip_state = 0
                    write_logs( "[ZYNQ-7000] TOTAL OF BYTES WITH BITFLIPS: " + read_data_serial_string)
                    #num_words=2*int(read_data_serial_string)
                    total_bitflips = 2
                if("start" in read_data_serial_string):
                    write_logs( "[ZYNQ-7000] RUN STARTED")
                    total_bitflips=0
                    sem_ip_state = 0
                    num_words = 0
                if("no_error" in read_data_serial_string):
                    write_logs( "[ZYNQ-7000] no errors detected")
                    sem_ip_state = 0
                    num_words = 0
                    total_bitflips=0
                if("total_worlds_w" in read_data_serial_string):
                    total_bitflips = 1
                    sem_ip_state = 0
                #print(read_data_serial_string)

                read_data_serial = []
                read_data_serial_string = None

                # else:
                    # write_logs( " [INFO] " + read_data_serial_string.replace("\r", "") + " - unexpected data. reset...")
                    # read_data_serial = []
                    # sem_ip_state = 10
            timeout_read = 0
        if(timeout_read == 1):
            write_logs( "[INFO] timeout. reset...")
            read_data_serial = []
            sem_ip_state = 10
    # RESET
    if(sem_ip_state == 10):
        reset()
        sem_ip_state = 0

write_logs("[INFO] ending radiation experiment at " + getTime()) # write line of text to file and print to screen
serial_interface.close()
