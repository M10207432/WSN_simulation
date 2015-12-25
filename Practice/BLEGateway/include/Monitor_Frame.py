from socket import socket, AF_INET, SOCK_STREAM
import msvcrt as m
import sys, os
import wx,string
import wx.media
import wx.lib.agw.aquabutton as AB
import time
import random

wx.Log.SetLogLevel(0)

class IOT_Interface(wx.Frame):
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

        '''=========================
                Set Up
        ========================='''
        self.BGimg='./include/Element/BG1.jpg'
        wx.Frame.__init__(self, parent=None, title="Gateway monitor", size=(self.panel_xsize, self.panel_ysize))
        self.panel= MainPanel(self)
        self.picture=None
        self.clock=None
        self.connect=False
        self.timerset="NULL"
        
        '''=========================
                Create Component
        ========================='''
        self.Component()                #Create component
            
    def Component(self):
        panel=self.panel

        '''==========================
                BUTTON
        =========================='''
        
        #--------------------TextCtrl
        wx.StaticText(parent=panel, label=":", pos=(950,32))
        
        #--------------------Connection & Disconnection Btn
        self.conn_btn=wx.Button(parent=panel, label="Conn", pos=(50,800))#size=(size_x, size_y)
        self.Bind(wx.EVT_BUTTON, self.ConnFunc, self.conn_btn)

        self.disconn_btn=wx.Button(parent=panel, label="Disconn", pos=(50,850))#size=(size_x, size_y)
        self.Bind(wx.EVT_BUTTON, self.DisConnFunc, self.disconn_btn)

        #--------------------Start
        self.Start_btn=wx.Button(parent=panel, label="Start", pos=(50,900))#size=(size_x, size_y)
        self.Bind(wx.EVT_BUTTON, self.StartSchedule, self.Start_btn)
            
    def ConnFunc(self, event):
        print "Conn event"
        
    def DisConnFunc(self, event):
        print "Dis Conn event"
    
    def StartSchedule(self, event):
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
        
    def Timecheck(self, event):
        print "Timecheck"
        show_hour=''
        show_min=''
        
        try:
            h=int(self.hour.GetValue())
            m=int(self.min.GetValue())
            if h<24 and h>0 and m<=60 and m>=0:
                print "OK"
                if len(str(h))==1:
                    show_hour="0"+str(h)
                else:
                    show_hour=str(h)
                    
                if len(str(m))==1:
                    show_min="0"+str(m)
                else:
                    show_min=str(m)
                    
                self.timeset.SetLabel("Check at "+show_hour+":"+show_min)
                self.timerset=show_hour+":"+show_min
            else:
                print "Wrong range"
                self.timeset.SetLabel("No setting")
                self.timerset='NULL'
        except:
            print "Not int"
            self.timeset.SetLabel("No setting")
            self.timerset='NULL'
        

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
        