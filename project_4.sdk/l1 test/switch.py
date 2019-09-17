#!/usr/bin/python
import threading
import socket
import time
import os
from datetime import datetime
import sys
class Switch():
    def __init__(self, ip, portCount):
        self.ip = ip
        self.portCount = portCount
        self.portList = []
        for i in range(0, self.portCount):
            self.portList.append(
                        'pw%1dName=&P6%1d=%%s&P6%1d_TS=&P6%1d_TC=&' %
                        (i+1, i, i, i)
                    )

    def cmd(self, port, c):
        assert(port <= self.portCount)

        cmd = 'curl --data \"'

        # the port list is indexed from 0, so fix it
        port = port - 1

        for i in range(0, self.portCount):
            if i == (port):
                cmd += self.portList[i] % c
            else:
                cmd += self.portList[i]

        cmd += '&Apply=Apply\" '
        cmd += 'http://%s/tgi/iocontrol.tgi ' % (self.ip)
        cmd += '-o /dev/null 2>/dev/null'
        #print cmd
        return os.system(cmd)

    def on(self, port):
        return self.cmd(port, 'On')

    def off(self, port):
        return self.cmd(port, 'Off')

#s = Switch("192.168.1.100", 4)
#print "Port 1"
#s.cmd(int(1), 'On')
#s.cmd(int(1), 'Off')
#print "Port 2"


