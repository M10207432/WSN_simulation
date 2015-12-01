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
import Monitor_Frame as GW_Monitor
import BLE_Scheduler as BLESche

def WSNFrame():
    app=wx.App(False)
    frame = GW_Monitor.IOT_Interface()
    frame.Show()
    app.MainLoop()
    
def main():
    WSNFrame()
    #Sche=BLESche.Scheduler(1)
    
if __name__ == "__main__":
    main()
    #Device()
    #ScheEIMA()
    
