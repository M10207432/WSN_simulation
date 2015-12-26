import collections
import serial
import time,datetime
import os,sys
import threading
import wx

'''================================
        Include File
================================'''
os.sys.path.append("./include")
import GW_BLEBuilder as GW_B
import GW_BLEParser as GW_P
import Monitor_Frame as GW_Monitor
from HCI_CMD import *

'''================================
        Dongle Setting
================================'''
BLEport="COM6"
BLEBaudrate=115200

'''================================
        Global value
================================'''
mutex=threading.Lock() 
TDMAState=[0,0]
TDMAState[0]=True
device1="\x1C\xBA\xD6\x14\x33\x88" #"\x02\xC2\x36\x29\x6A\xBC"
device2="\xF0\xBF\x36\x29\x6A\xBC"
device3="\xF6\x8E\x27\xAF\x59\x90"
'''
Frame=[{"Exe":0.52,"Period":1,"Deadline":1,'Arrival':0,'id':1},
        {"Exe":0.54,"Period":2,"Deadline":2,'Arrival':0,'id':2},
        {"Exe":0.56,"Period":3,"Deadline":3,'Arrival':0,'id':3}]
'''
'''
Frame=[ {"Exe":0.1,"Period":0.150,"Deadline":0.15,'Arrival':0,'id':1},
        {"Exe":0.1,"Period":0.50,"Deadline":0.5,'Arrival':0,'id':2},
        {"Exe":0.1,"Period":1.0,"Deadline":1.0,'Arrival':0,'id':3}]
'''
Nodeinfo=[[200,200],
          [400,400],
          [800,800],
          {"meet":0, "miss":0,"meetratio":0}]
'''
Frame=[{"Exe":0.05,"Period":0.200,"Deadline":0.200,'Arrival':0,'id':1},
        {"Exe":0.05,"Period":0.400,"Deadline":0.400,'Arrival':0,'id':2},
        {"Exe":0.05,"Period":0.800,"Deadline":0.800,'Arrival':0,'id':3}]
'''

Frame=[{"Exe":0.1,"Period":0.200,"Deadline":0.200,'Arrival':0,'id':1},
        {"Exe":0.1,"Period":0.400,"Deadline":0.400,'Arrival':0,'id':2},
        {"Exe":0.1,"Period":0.800,"Deadline":0.800,'Arrival':0,'id':3}]

'''
Nodeinfo=[[200,200],
          [350,350],
          [500,500],
          {"meet":0, "miss":0,"meetratio":0}]
Frame=[{"Exe":0.05,"Period":0.200,"Deadline":0.200,'Arrival':0,'id':1},
        {"Exe":0.05,"Period":0.350,"Deadline":0.350,'Arrival':0,'id':2},
        {"Exe":0.05,"Period":0.500,"Deadline":0.500,'Arrival':0,'id':3}]
'''
'''
Frame=[{"Exe":0.1,"Period":0.200,"Deadline":0.200,'Arrival':0,'id':1},
        {"Exe":0.1,"Period":0.350,"Deadline":0.350,'Arrival':0,'id':2},
        {"Exe":0.1,"Period":0.500,"Deadline":0.500,'Arrival':0,'id':3}]
'''
'''
Nodeinfo=[[150,500],
          [500,1000],
          [1000,1000],
          {"meet":0, "miss":0,"meetratio":0}]

Frame=[{"Exe":0.05,"Period":0.150,"Deadline":0.15,'Arrival':0,'id':1},
        {"Exe":0.05,"Period":0.50,"Deadline":0.5,'Arrival':0,'id':2},
        {"Exe":0.05,"Period":1.0,"Deadline":1.0,'Arrival':0,'id':3}]
'''
'''
Frame=[{"Exe":0.33,"Period":1,"Deadline":1,'Arrival':0,'id':1},
        {"Exe":0.66,"Period":2,"Deadline":2,'Arrival':0,'id':2},
        {"Exe":0.99,"Period":3,"Deadline":3,'Arrival':0,'id':3}]

Frame=[{"Exe":0.33,"Period":1,"Deadline":1,'Arrival':0,'id':1},
        {"Exe":0.33,"Period":2,"Deadline":2,'Arrival':0,'id':2},
        {"Exe":0.33,"Period":3,"Deadline":3,'Arrival':0,'id':3}]
'''

Timeslot=0
td1=0.4
td2=0.4
td3=0.65
def analyse_packet((packet, dictionary)):
    ParseBLERSP(print_output((packet, dictionary)),Nodeinfo)
   

serial_port = serial.Serial(port=BLEport, baudrate=BLEBaudrate)
ble_builder = GW_B.GW_Builder(serial_port)
ble_parser = GW_P.GW_BLEParser(serial_port, callback=analyse_packet)    #(analyse_packet) callback(packet, dictionary)


def CC2540EM():
    try:
        BLECMD=ble_builder.BLECMD
        
        print "-------------------------Init device-------------------------"
        print_output(BLECMD("fe00"))
        
        time.sleep(1)
        
        print "-------------------------Connect CC2540EM-------------------------"
        print_output(BLECMD("fe09", peer_addr="\x1C\xBA\xD6\x14\x33\x88"))#\x1C\xBA\xD6\x14\x33\x88
        
        I=raw_input("PAUSE")
        #================================================GATT_WriteCharValue
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
        print datetime.datetime.now().strftime("%H:%M:%S.%f")
        while True:
            if TDMAState[0]==True:
                TDMAState[0]=False
                print_output(BLECMD("fd9b",
                                  conn_handle="\x00\x00",
                                  handle="\x25\x00",
                                  value="\x0D"))
            
            #time.sleep(0.3)
        '''
        print_output(BLECMD("fd92",
                                  conn_handle="\x00\x00",
                                  handle="\x25\x00",
                                  value="\x02"))
        time.sleep(10)
        '''
        print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
        
    except:
        print "=====================ERROR====================="
        pass
    ble_parser.stop()
    
def Keyfob():
    try:
        BLECMD=ble_builder.BLECMD
        
        print "-------------------------Init device-------------------------"
        print_output(BLECMD("fe00"))
        
        
        time.sleep(1)
        
        print "-------------------------Connect Keyfob-------------------------"
        print_output(BLECMD("fe09", peer_addr="\x8C\x64\xAB\x29\x6A\xBC"))
        
        I=raw_input("PAUSE")
        #================================================GATT_WriteCharValue
         
        print_output(BLECMD("fd92",
                                  conn_handle="\x00\x00",
                                  handle="\x34\x00",
                                  value="\x01\x00"))
        time.sleep(2)
        print_output(BLECMD("fd92",
                                  conn_handle="\x00\x00",
                                  handle="\x3B\x00",
                                  value="\x01\x00"))

        time.sleep(10)
        print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
        
    except:
        print "=====================ERROR====================="
        pass
    ble_parser.stop()
    
def HM10():
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init device-------------------------"
        print_output(BLECMD("fe00"))
        
        time.sleep(1)
        
        print "-------------------------Connect HM10-------------------------"
        print_output(BLECMD("fe09", peer_addr="\x72\xC3\xFD\x8F\xC3\x20"))
        
        I=raw_input("PAUSE")
        #================================================GATT_WriteCharValue
         
        print_output(BLECMD("fd92",
                                  conn_handle="\x00\x00",
                                  handle="\x36\x00",
                                  value="\x05\x04\x03\x02\x01\x00"))
        
        time.sleep(10)
        print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
        
    except:
        print "=====================ERROR====================="
        pass
    ble_parser.stop()
    
def StarNode():
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init device-------------------------"
        print_output(BLECMD("fe00"))
        time.sleep(1)

        print "-------------------------Connect-------------------------"
        print_output(BLECMD("fe09", peer_addr="\xF0\xBF\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(1)
        #print_output(BLECMD("fe09", peer_addr="\x72\xC3\xFD\x8F\xC3\x20"))#HM10
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")
        I=raw_input("PAUSE")
        #================================================GATT_WriteCharValue
        print datetime.datetime.now().strftime("%H:%M:%S.%f")
        for i in range(0,60):
            time.sleep(0.99999)
            print i            
            print_output(BLECMD("fd9b",
                                      conn_handle="\x00\x00",
                                      handle="\x25\x00",
                                      value="\x0D"))
        '''
        print_output(BLECMD("fd92",
                                  conn_handle="\x01\x00",
                                  handle="\x36\x00",
                                  value="\x05\x04\x03\x02\x01\x00"))
        '''
        #time.sleep(60)

        print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
        time.sleep(1)
        #print_output(BLECMD("fe0a", conn_handle="\x01\x00"))
        
    except:
        pass
    ble_parser.stop()

def SyncNode():
    global TDMAState
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init device-------------------------"
        print_output(BLECMD("fe00"))
        time.sleep(1)

        print "-------------------------Connect-------------------------"
        
        print_output(BLECMD("fe09", peer_addr="\x02\xC2\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(1)
        print_output(BLECMD("fe09", peer_addr="\xF0\xBF\x36\x29\x6A\xBC"))#CC2540EM
        
        I=raw_input("PAUSE")
        
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x01\x00", handle="\x25\x00",value="\x0E")#Trigger
        
        #================================================GATT_WriteCharValue
        print datetime.datetime.now().strftime("%H:%M:%S.%f")," Start"

        TDMAcounter=100
        while True:

            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                    value="\x0D"))
            
            #time.sleep(5)#2*connection interval
            
            print_output(BLECMD("fd9b",
                                    conn_handle="\x01\x00",
                                    handle="\x25\x00",
                                    value="\x0D"))
            time.sleep(1)#2*connection interval
            
            '''
            if i%5==0:
                print datetime.datetime.now().strftime("%H:%M:%S.%f")," SCAN CMD"
                
                print_output(BLECMD("fd9b",
                                      conn_handle="\x00\x00",
                                      handle="\x25\x00",
                                      value="\x0C"))
                #time.sleep(0.25)
            '''
    except:
        pass
    #=======================================================Disconnect
    print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x01\x00"))
    
    ble_parser.stop()



def EIMA():
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init Dongle Device-------------------------"
        print_output(BLECMD("fe00"))
        time.sleep(1)

        print "-------------------------Connect-------------------------"
        
        print_output(BLECMD("fe09", peer_addr="\x02\xC2\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(1)
        print_output(BLECMD("fe09", peer_addr="\xF0\xBF\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(1)
        print_output(BLECMD("fe09", peer_addr="\xF6\x8E\x27\xAF\x59\x90"))#CC2540EM
        
        I=raw_input("PAUSE")
        
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x01\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x02\x00", handle="\x25\x00",value="\x0E")#Trigger
        
        #================================================GATT_WriteCharValue
        print datetime.datetime.now().strftime("%H:%M:%S.%f")," Start"

        TDMAcounter=100
        value=1
        while True:
            #=================Sche
            scheframe=None #list, deadline
            for d in Frame:
                if scheframe==None:
                    if Timeslot>=scheframe['Arrival']:
                        scheframe=d
                else:
                    if scheframe['Deadline']>d['Deadline'] and Timeslot>=scheframe['Arrival']:
                        scheframe=d
            
            #=================Transmission
            print Timeslot
            if scheframe!=None:
                print scheframe['id'],scheframe['Deadline']
                time.sleep(scheframe['Exe'])
                
                scheframe['Deadline']=scheframe['Deadline']+scheframe['Period']


            '''
            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td1)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x01\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td2)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x02\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td3)#Connection interval
            '''
            '''
            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td1)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x01\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td2)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td1)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x02\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td3)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td1)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x01\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td2)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td1)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x02\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td3)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td1)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x00\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td1)#Connection interval

            print_output(BLECMD("fd9b",
                                    conn_handle="\x01\x00",
                                    handle="\x25\x00",
                                   value="\x0D"))
            time.sleep(td2)#Connection interval
            '''
    except:
        pass
    #=======================================================Disconnect
    print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x01\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x02\x00"))
    
    ble_parser.stop()
    
def ScheEIMA():
    '''
    Frame=[{"Exe":0.52,"Period":1,"Deadline":1,'Arrival':0,'id':1},
           {"Exe":0.54,"Period":2,"Deadline":2,'Arrival':0,'id':2},
           {"Exe":0.56,"Period":3,"Deadline":3,'Arrival':0,'id':3}]
    '''
    Timeslot=0
    NPEDFRD_f=False
    
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init Dongle Device-------------------------"
        print_output(BLECMD("fe00"))
        time.sleep(1)

        print "-------------------------Connect-------------------------"
        
        print_output(BLECMD("fe09", peer_addr="\x02\xC2\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(1)
        print_output(BLECMD("fe09", peer_addr="\xF0\xBF\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(1)
        print_output(BLECMD("fe09", peer_addr="\xF6\x8E\x27\xAF\x59\x90"))#CC2540EM
        
        I=raw_input("PAUSE")
        
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x01\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x02\x00", handle="\x25\x00",value="\x0E")#Trigger
        
        #================================================GATT_WriteCharValue
        print datetime.datetime.now().strftime("%H:%M:%S.%f")," Start"

        TDMAcounter=100
        value=1
        while True:
            #=================Sche
            scheframe=None #list, deadline
            critical=None

            #Find Critical
            for d in Frame:
                if critical==None:
                    critical=d
                elif critical['Deadline']>d['Deadline']:
                    critical=d
            
            #Find Next Ready Frame
            for d in Frame:
                if scheframe==None:
                    if Timeslot>=d['Arrival']:
                        scheframe=d
                else:
                    if scheframe['Deadline']>d['Deadline'] and Timeslot>=scheframe['Arrival']:
                        scheframe=d

            if scheframe!=None:

                v=Timeslot+critical['Exe']+scheframe['Exe']
                
                if v>critical['Deadline'] and critical!=scheframe and NPEDFRD_f:
                    scheframe=critical
            #=================Transmission
            
            if scheframe!=None:
                #print "Timeslot=",Timeslot,
                #print "ID=",scheframe['id']," Arrival=",scheframe['Arrival']," Deadline=",scheframe['Deadline']
                            
                scheframe['Deadline']=scheframe['Deadline']+scheframe['Period']
                scheframe['Arrival']=scheframe['Arrival']+scheframe['Period']
                #-----------#
                if scheframe['id']==1:
                    print_output(BLECMD("fd9b",
                                        conn_handle="\x00\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
                if scheframe['id']==2:
                    print_output(BLECMD("fd9b",
                                        conn_handle="\x01\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
                if scheframe['id']==3:
                    print_output(BLECMD("fd9b",
                                        conn_handle="\x02\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))

                time.sleep(scheframe['Exe'])#Connection interval
                #-----------#
                
                Timeslot+=scheframe['Exe']
            else:
                #print "Timeslot=",Timeslot," IDLE"
                time.sleep(0.1)
                Timeslot+=0.1
    except:
        pass
    #=======================================================Disconnect
    print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x01\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x02\x00"))
    
    ble_parser.stop()
    
def EIF():
    '''
    Frame=[{"Exe":0.52,"Period":1,"Deadline":1,'Arrival':0,'id':1},
           {"Exe":0.54,"Period":2,"Deadline":2,'Arrival':0,'id':2},
           {"Exe":0.56,"Period":3,"Deadline":3,'Arrival':0,'id':3}]

    device1="\x02\xC2\x36\x29\x6A\xBC" #"\x02\xC2\x36\x29\x6A\xBC"
    device2="\xF0\xBF\x36\x29\x6A\xBC"
    device3="\xF6\x8E\x27\xAF\x59\x90"
    '''
    Timeslot=0
    NPEDFRD_f=True
    
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init Dongle Device-------------------------"
        print_output(BLECMD("fe00"))
        time.sleep(1)

        print "-------------------------Connect-------------------------"
        
        print_output(BLECMD("fe09", peer_addr="\x1C\xBA\xD6\x14\x33\x88"))#CC2540EM
        time.sleep(2)
        print_output(BLECMD("fe09", peer_addr="\xF0\xBF\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(2)
        print_output(BLECMD("fe09", peer_addr="\xF6\x8E\x27\xAF\x59\x90"))#CC2540EM
        
        I=raw_input("PAUSE")
        
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x01\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x02\x00", handle="\x25\x00",value="\x0E")#Trigger
        
        #================================================GATT_WriteCharValue
        print datetime.datetime.now().strftime("%H:%M:%S.%f")," Start"

        TDMAcounter=100
        value=1
        while True:
            #=================Sche
            scheframe=None #list, deadline
            critical=None

            #Find Critical
            for d in Frame:
                if critical==None:
                    critical=d
                elif critical['Deadline']>d['Deadline']:
                    critical=d
            
            #Find Next Ready Frame
            for d in Frame:
                if scheframe==None:
                    if Timeslot>=d['Arrival']:
                        scheframe=d
                else:
                    if scheframe['Deadline']>d['Deadline'] and Timeslot>=scheframe['Arrival']:
                        scheframe=d

            if scheframe!=None:

                v=Timeslot+critical['Exe']+scheframe['Exe']
                cmp1=(critical['Deadline']/critical['Exe'])*critical['Exe']
                cmp2=Timeslot+scheframe['Exe']
                '''
                if v>critical['Deadline'] and critical!=scheframe and NPEDFRD_f:
                    if Timeslot>=critical['Arrival']:
                        scheframe=critical
                    else:
                        scheframe=None
                '''
                
                if v>critical['Deadline'] and critical!=scheframe and NPEDFRD_f:
                    if cmp1<cmp2:
                        if Timeslot>=critical['Arrival']:
                            scheframe=critical
                        else:
                            scheframe=None
                
            #=================Transmission
            
            if scheframe!=None:
                #print "Timeslot=",Timeslot,
                #print "ID=",scheframe['id']," Arrival=",scheframe['Arrival']," Deadline=",scheframe['Deadline']
                            
                scheframe['Deadline']=scheframe['Deadline']+scheframe['Period']
                scheframe['Arrival']=scheframe['Arrival']+scheframe['Period']
                #-----------#
                if scheframe['id']==1:
                    print_output(BLECMD("fd9b",
                                        conn_handle="\x00\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
                if scheframe['id']==2:
                    print_output(BLECMD("fd9b",
                                        conn_handle="\x01\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
                if scheframe['id']==3:
                    print_output(BLECMD("fd9b",
                                        conn_handle="\x02\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))

                time.sleep(scheframe['Exe'])#Connection interval
                #-----------#
                
                Timeslot+=scheframe['Exe']
            else:
                #print "Timeslot=",Timeslot," IDLE"
                time.sleep(0.01)
                Timeslot+=0.01
    except:
        pass
    #=======================================================Disconnect
    print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x01\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x02\x00"))
    
    ble_parser.stop()
    
def Table():
    '''
    Frame=[{"Exe":0.52,"Period":1,"Deadline":1,'Arrival':0,'id':1},
           {"Exe":0.54,"Period":2,"Deadline":2,'Arrival':0,'id':2},
           {"Exe":0.56,"Period":3,"Deadline":3,'Arrival':0,'id':3}]
    '''
    Timeslot=0
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init Dongle Device-------------------------"
        print_output(BLECMD("fe00"))
        time.sleep(1)

        print "-------------------------Connect-------------------------"
        
        print_output(BLECMD("fe09", peer_addr="\x1C\xBA\xD6\x14\x33\x88"))#CC2540EM
        time.sleep(2)
        print_output(BLECMD("fe09", peer_addr="\xF0\xBF\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(2)
        print_output(BLECMD("fe09", peer_addr="\xF6\x8E\x27\xAF\x59\x90"))#CC2540EM
        
        I=raw_input("PAUSE")
        
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x01\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x02\x00", handle="\x25\x00",value="\x0E")#Trigger
        
        #================================================GATT_WriteCharValue
        print datetime.datetime.now().strftime("%H:%M:%S.%f")," Start"

        TDMAcounter=100
        value=1
        while True:
            
            print_output(BLECMD("fd9b",
                                        conn_handle="\x00\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[0]['Exe'])#Connection interval

            print_output(BLECMD("fd9b",
                                        conn_handle="\x01\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[1]['Exe'])#Connection interval
                
            print_output(BLECMD("fd9b",
                                        conn_handle="\x02\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[2]['Exe'])#Connection interval
    except:
        pass
    #=======================================================Disconnect
    print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x01\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x02\x00"))
    
    ble_parser.stop()
    
def polling():
    '''
    Frame=[{"Exe":0.52,"Period":1,"Deadline":1,'Arrival':0,'id':1},
           {"Exe":0.54,"Period":2,"Deadline":2,'Arrival':0,'id':2},
           {"Exe":0.56,"Period":3,"Deadline":3,'Arrival':0,'id':3}]
    '''
    Timeslot=0
    try:
        BLECMD=ble_builder.BLECMD
        print "-------------------------Init Dongle Device-------------------------"
        print_output(BLECMD("fe00"))
        time.sleep(1)

        print "-------------------------Connect-------------------------"
        
        print_output(BLECMD("fe09", peer_addr="\x1C\xBA\xD6\x14\x33\x88"))#CC2540EM
        time.sleep(1)
        print_output(BLECMD("fe09", peer_addr="\xF0\xBF\x36\x29\x6A\xBC"))#CC2540EM
        time.sleep(1)
        print_output(BLECMD("fe09", peer_addr="\xF6\x8E\x27\xAF\x59\x90"))#CC2540EM
        
        I=raw_input("PAUSE")
        
        BLECMD("fd9b",conn_handle="\x00\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x01\x00", handle="\x25\x00",value="\x0E")#Trigger
        BLECMD("fd9b",conn_handle="\x02\x00", handle="\x25\x00",value="\x0E")#Trigger
        
        #================================================GATT_WriteCharValue
        print datetime.datetime.now().strftime("%H:%M:%S.%f")," Start"

        TDMAcounter=100
        value=1
        while True:
            #==================================R1
            print_output(BLECMD("fd9b",
                                        conn_handle="\x00\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[0]['Exe'])#Connection interval
            #==================================R2
            print_output(BLECMD("fd9b",
                                        conn_handle="\x00\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[0]['Exe'])#Connection interval
            print_output(BLECMD("fd9b",
                                        conn_handle="\x01\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[1]['Exe'])#Connection interval
            #==================================R3
            print_output(BLECMD("fd9b",
                                        conn_handle="\x00\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[0]['Exe'])#Connection interval

            print_output(BLECMD("fd9b",
                                        conn_handle="\x01\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[1]['Exe'])#Connection interval
                
            print_output(BLECMD("fd9b",
                                        conn_handle="\x02\x00",
                                        handle="\x25\x00",
                                       value="\x0D"))
            time.sleep(Frame[2]['Exe'])#Connection interval
    except:
        pass
    #=======================================================Disconnect
    print_output(BLECMD("fe0a", conn_handle="\x00\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x01\x00"))
    time.sleep(1)
    print_output(BLECMD("fe0a", conn_handle="\x02\x00"))
    
    ble_parser.stop()
    
def WSNFrame():
    app=wx.App()
    frame = GW_Monitor.IOT_Interface()
    frame.Show()
    app.MainLoop()
    
def main():
    WSNFrame()

if __name__ == "__main__":
    #main()
    
    #Keyfob()
    #CC2540EM()
    #HM10()
    #SyncNode()
    #ScheEIMA()
    
    #polling()
    Table()
    #EIF()
    try:
        meetratio=float(Nodeinfo[3]["meet"])/float(Nodeinfo[3]["meet"]+Nodeinfo[3]["miss"])
        print "Meet number:",Nodeinfo[3]["meet"]
        print "Miss number:",Nodeinfo[3]["miss"]
        print "Meet Ratio:",meetratio
    except:
        pass


    
