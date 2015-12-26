from socket import socket, AF_INET, SOCK_STREAM
import msvcrt as m
import sys, os
import wx,string
import wx.media
import wx.lib.agw.aquabutton as AB
import time
import random
import pprint

#===============================
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
    FigureCanvasWxAgg as FigCanvas, \
    NavigationToolbar2WxAgg as NavigationToolbar
import numpy as np
import pylab

#===============================
import BLE_Scheduler as BLESche
import wx_mpl_dynamic_graph as CanvasDraw
from threading import Thread

REFRESH_INTERVAL_MS = 100

class IOT_Interface(CanvasDraw.GraphFrame):
    def __init__(self):
        '''=========================
                Init Parameter
        ========================='''
        self.panel_xsize=1000
        self.panel_ysize=1000
        
        self.choose_xpos=700
        self.choose_ypos=450
        self.pre_xpos=100
        self.pre_ypos=550
        self.next_xpos=300
        self.next_ypos=550
        self.buy_xpos=200
        self.buy_ypos=600
        self.data=[]
        '''=========================
                Set Up
        ========================='''
        wx.Frame.__init__(self, parent=None, title="Gateway monitor", size=(self.panel_xsize, self.panel_ysize))
        self.panel=wx.Panel(self)
        self.picture=None
        self.clock=None
        self.connect=False
        self.timerset="NULL"
        
        '''=========================
                Create Component
        ========================='''
        self.redraw_timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.on_redraw_timer, self.redraw_timer)        
        self.redraw_timer.Start(REFRESH_INTERVAL_MS)
        
        self.Component()                #Create component
        
        '''=========================
                Create Component
        ========================='''
        self.BLESche=BLESche.Scheduler()
        self.t=Thread(target=self.BLESche.run)
        
    def Component(self):
        panel=self.panel
        self.init_plot()
        self.canvas = FigCanvas(self.panel, -1, self.fig)
        
        '''==========================
                BUTTON
        =========================='''
        #--------------------TextCtrl
        wx.StaticText(parent=panel, label=":", pos=(950,32))
        
        #--------------------Connection & Disconnection Btn
        self.conn_btn=wx.Button(parent=panel, label="Conn", pos=(600,900))#size=(size_x, size_y)
        self.Bind(wx.EVT_BUTTON, self.ConnFunc, self.conn_btn)

        self.disconn_btn=wx.Button(parent=panel, label="Disconn", pos=(700,900))#size=(size_x, size_y)
        self.Bind(wx.EVT_BUTTON, self.DisConnFunc, self.disconn_btn)

        #--------------------Start
        self.Start_btn=wx.Button(parent=panel, label="Start", pos=(800,900))#size=(size_x, size_y)
        self.Bind(wx.EVT_BUTTON, self.StartSchedule, self.Start_btn)
        
        #--------------------Box
        self.xmin_control = CanvasDraw.BoundControlBox(self.panel, -1, "X min", 0)
        self.xmax_control = CanvasDraw.BoundControlBox(self.panel, -1, "X max", 50)
        self.ymin_control = CanvasDraw.BoundControlBox(self.panel, -1, "Y min", 0)
        self.ymax_control = CanvasDraw.BoundControlBox(self.panel, -1, "Y max", 100)
                
        self.cb_grid = wx.CheckBox(self.panel, -1, 
            "Show Grid",
            style=wx.ALIGN_RIGHT)
        self.Bind(wx.EVT_CHECKBOX, self.on_cb_grid, self.cb_grid)
        self.cb_grid.SetValue(True)
        
        self.cb_xlab = wx.CheckBox(self.panel, -1, 
            "Show X labels",
            style=wx.ALIGN_RIGHT)
        self.Bind(wx.EVT_CHECKBOX, self.on_cb_xlab, self.cb_xlab)
        self.cb_xlab.SetValue(True)
        
        self.hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox1.AddSpacer(20)
        self.hbox1.Add(self.cb_grid, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
        self.hbox1.AddSpacer(10)
        self.hbox1.Add(self.cb_xlab, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
        
        self.hbox2 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox2.Add(self.xmin_control, border=5, flag=wx.ALL)
        self.hbox2.Add(self.xmax_control, border=5, flag=wx.ALL)
        self.hbox2.AddSpacer(24)
        self.hbox2.Add(self.ymin_control, border=5, flag=wx.ALL)
        self.hbox2.Add(self.ymax_control, border=5, flag=wx.ALL)
        
        self.vbox = wx.BoxSizer(wx.VERTICAL)
        self.vbox.Add(self.canvas, 1, flag=wx.LEFT | wx.TOP | wx.GROW)        
        self.vbox.Add(self.hbox1, 0, flag=wx.ALIGN_LEFT | wx.TOP)
        self.vbox.Add(self.hbox2, 0, flag=wx.ALIGN_LEFT | wx.TOP)
        
        self.panel.SetSizer(self.vbox)
        self.vbox.Fit(self)
        
    def ConnFunc(self, event):
        if self.t.is_alive():
            print "Alive"
        else:
            print "Dead"
        print "Conn event"
        
    def DisConnFunc(self, event):
        self.BLESche.shutdown()
        print "Dis Conn event"
        
    def StartSchedule(self, event):
        self.t.start()
        print "Start Schedule"
    
        
    def ShowCloth(self, imgfile):
        panel=self.panel

        Clothid=imgfile.split('/')[1].split('.')[0]
        if int(Clothid) <5:
            self.Cloth_status=wx.StaticText(parent=panel, label="Get", pos=(450,150))
            self.Cloth_status.SetForegroundColour((255,0,0))
        else:
            self.Cloth_status=wx.StaticText(parent=panel, label="Buy", pos=(450,150))
            self.Cloth_status.SetForegroundColour((255,0,0))
        
        if imgfile.find('png')>0 or imgfile.find('PNG')>0:
            bmp = wx.Image(imgfile, wx.BITMAP_TYPE_PNG).ConvertToBitmap()
        if imgfile.find('jpg')>0 or imgfile.find('JPG')>0:
            bmp = wx.Image(imgfile, wx.BITMAP_TYPE_JPEG).ConvertToBitmap()

        print "ShowCloth",imgfile
        image = wx.ImageFromBitmap(bmp)
        image = image.Scale(self.imgsize_x, self.imgsize_y, wx.IMAGE_QUALITY_HIGH)
        result = wx.BitmapFromImage(image)
    
        self.picture=wx.StaticBitmap(panel, -1, result, (self.imgpos_x, self.imgpos_y), (self.imgsize_x, self.imgsize_y))#(bmp.GetWidth(), bmp.GetHeight())
        
        '''
        gif = wx.Image(opj('bitmaps/image.gif'), wx.BITMAP_TYPE_GIF).ConvertToBitmap()
        png = wx.Image(opj('bitmaps/image.png'), wx.BITMAP_TYPE_PNG).ConvertToBitmap()
        jpg = wx.Image(opj('bitmaps/image.jpg'), wx.BITMAP_TYPE_JPEG).ConvertToBitmap()
        '''
    def shameF_Func(self,evt):
        print "shameF"
        
        if self.connect:
            self.clientsocket.send("Get,f,\r")
            
    def shameB_Func(self,evt):
        print "shameF"
        
        if self.connect:
            self.clientsocket.send("Get,b,\r")
    def on_redraw_timer(self, event):
        # if paused do not add data, but still redraw the plot
        # (to respond to scale modifications, grid change, etc.)
        #
        '''
        if not self.paused:
            self.data.append(self.datagen.next())
        self.draw_plot()
        '''
        try:
            while len(self.BLESche.RecvPkt)>0:
                #print "Frame"
                #print self.BLESche.RecvPkt
                self.data.append(self.BLESche.RecvPkt[0])
                self.draw_plot()
                #print "end"
                self.BLESche.RecvPkt.pop()
        except:
            pass
        
        
    def onClose(self):
        self.Close()
     
class MainPanel(wx.Panel):
    def __init__(self, parent):
        
        """Constructor"""
        wx.Panel.__init__(self, parent=parent)

        self.frame = parent
        
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        
    def OnEraseBackground(self, evt):
        dc = evt.GetDC()
 
        if not dc:
            dc = wx.ClientDC(self)
            rect = self.GetUpdateRegion().GetBox()
            dc.SetClippingRect(rect)
        dc.Clear() 
        
class Img_MainPanel(wx.Panel):
    def __init__(self, parent, BGimg):
        
        self.BGimg=BGimg
        """Constructor"""
        wx.Panel.__init__(self, parent=parent)

        self.frame = parent
        
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        
    def OnEraseBackground(self, evt):
        dc = evt.GetDC()
 
        if not dc:
            dc = wx.ClientDC(self)
            rect = self.GetUpdateRegion().GetBox()
            dc.SetClippingRect(rect)
        dc.Clear()
        bmp = wx.Bitmap(self.BGimg)
        
        dc.DrawBitmap(bmp, 0, 0)
        