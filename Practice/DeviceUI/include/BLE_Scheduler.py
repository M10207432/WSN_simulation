import collections
import serial
import time,datetime
import os,sys
import threading
import wx

'''================================
        Include File
================================'''
import GW_BLEBuilder as GW_B
import GW_BLEParser as GW_P
from HCI_CMD import *

'''================================
        Dongle Setting
================================'''
BLEport="COM6"
BLEBaudrate=115200

'''================================
        Global value
================================'''
RecvPkt=[]
Frame=[{"Exe":0.3,"Period":1,"Deadline":1,'Arrival':0,'id':1},
       {"Exe":0.6,"Period":2,"Deadline":2,'Arrival':0,'id':2},
       {"Exe":1,"Period":3,"Deadline":3,'Arrival':0,'id':3}]

'''================================
        Scheduler
================================''' 
class Scheduler():
    def __init__(self):
        global BLEport
        global BLEBaudrate
        global RecvPkt
        
        self.serial_port = serial.Serial(port=BLEport, baudrate=BLEBaudrate)
        self.ble_builder = GW_B.GW_Builder(self.serial_port)
        self.ble_parser = GW_P.GW_BLEParser(self.serial_port, callback=self.analyse_packet)    #(analyse_packet) callback(packet, dictionary)
        self.shutdownflag=False
        self.RecvPkt=RecvPkt
        print "BLE Builder Init Done"
        
        '''
        if Schepropose==1:
            self.run()
        else:
            self.ble_parser.stop()
        '''
    def run(self):
        Frame=[{"Exe":0.5,"Period":1,"Deadline":1,'Arrival':0,'id':1},
               {"Exe":0.5,"Period":2,"Deadline":2,'Arrival':0,'id':2},
               {"Exe":0.5,"Period":3,"Deadline":3,'Arrival':0,'id':3}]
        Timeslot=0
        try:
            self.BLECMD=self.ble_builder.BLECMD
            print "-------------------------Init Dongle Device-------------------------"
            print_output(self.BLECMD("fe00"))
            time.sleep(1)

            print "-------------------------Connect-------------------------"
            
            print_output(self.BLECMD("fe09", peer_addr="\x02\xC2\x36\x29\x6A\xBC"))#CC2540EM
            time.sleep(1)
            
            I=raw_input("PAUSE")
            #================================================GATT_WriteCharValue
            print datetime.datetime.now().strftime("%H:%M:%S.%f")," Start"
            self.BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
            TDMAcounter=100
            value=1
            while self.shutdownflag==False:
                self.BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                    value="\x0D")
                time.sleep(Frame[0]["Exe"])
        except:
            pass
        #=======================================================Disconnect
        print "Finish"
        
        #self.ble_parser.stop()
    def shutdown(self):
        try:
            self.shutdownflag=True
            print_output(self.BLECMD("fe0a", conn_handle="\x00\x00"))
            time.sleep(1)
            self.ble_parser.stop()
            
        except:
            print "ERROR"
        
    def analyse_packet(self,(packet, dictionary)):
        ParseBLERSP(print_output((packet, dictionary)),RecvPkt)