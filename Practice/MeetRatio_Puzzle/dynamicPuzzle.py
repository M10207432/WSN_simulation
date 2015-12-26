from Tkinter import *
from PIL import Image
import time

class PaintBox( Frame ):
   def __init__( self ):
      
      Frame.__init__( self )
      
      self.pack( expand = YES, fill = BOTH )
      self.master.title( "title" )
      self.master.geometry( "500x500" )

      self.message = Label( self, text = "Drag the mouse to draw" )
      self.message.pack( side = BOTTOM )
      
      self.myCanvas = Canvas( self )
      self.myCanvas.pack( expand = YES, fill = BOTH )

      self.loadimg("min.jpg")
      self.drawrange=8
      self.resultdata=open("Greedy_RR.txt", 'r')
      #self.myCanvas.after( 1000, self.DrawdataPuzzle )#Trigger point to paint
      
      self.myCanvas.after( 1000, self.paint )#Trigger point to paint
      #self.myCanvas.bind( "<B1-Motion>", self.paint )#Trigger point to paint      
   def paint(self):

      #Read data
      
      #Set the address
      if self.x<0:
         self.x=10
      else:
         self.x=self.x+self.drawrange
      if (self.x-self.drawrange)>self.x_size:
          self.x=self.drawrange
          self.y=self.y+self.drawrange

      data=self.resultdata.readline()
      if data.find("miss")<0:
         self.drawrec(1)
      else:
         self.drawrec(0)
         
      self.myCanvas.after( 1, self.paint )#Trigger point to paint
   
   def drawrec(self, meetflag):
      self.drawx=self.x
      self.drawy=self.y

      for i in range(self.drawrange):
         for j in range(self.drawrange):
            plot_x=self.drawx-self.drawrange+i
            plot_y=self.drawy-self.drawrange+j
            if plot_y<self.y_size and plot_x<self.x_size:
               try:
                  #Find the draw color
                  p1=str(hex(self.pixels[plot_y][plot_x][0]))[2:]
                  p2=str(hex(self.pixels[plot_y][plot_x][1]))[2:]
                  p3=str(hex(self.pixels[plot_y][plot_x][2]))[2:]
                  if len(p1)<2:
                     p1=p1+"0"
                  if len(p2)<2:
                     p2=p2+"0"
                  if len(p3)<2:
                     p3=p3+"0"
                   
                  if meetflag==1:
                     color="#"+p1+p2+p3
                  else:
                     color="#ffffff"
                  
                  #print p1,p2,p3,color
                  #draw
                  self.myCanvas.create_line(self.drawx+i, self.drawy+j, self.drawx+i, self.drawy+j+1, fill=color)
               except:
                  print self.drawx-self.drawrange+i
                  print self.drawy-self.drawrange+j
                  print self.pixels[self.drawx-self.drawrange+i][self.drawy-self.drawrange+j][0]
   def loadimg(self, filename):
      
      '''==========================
            Get Img Pixel
      =========================='''
      im=Image.open(filename)
      pixels=list(im.getdata())
      width, height=im.size
      self.pixels = [pixels[i * width:(i + 1) * width] for i in xrange(height)] #self.pixels[][]
      
      self.x_size=width
      self.y_size=height
       
      self.x=10
      self.y=10
      print self.x_size,self.y_size
      
   def DrawdataPuzzle(self):
      f=open("EIMA_EIF.txt",'r')

      print "Data load fin"
      for i in f:
         #Set the address
         if self.x<0:
            self.x=10
         else:
            self.x=self.x+self.drawrange
         if (self.x-self.drawrange)>self.x_size:
             self.x=self.drawrange
             self.y=self.y+self.drawrange

         #set draw color
         if i.find("miss")>0:
            print "Miss"
            self.drawrec()
         else:
            print "Meet"
            self.drawrec()
            
         time.sleep(0.01)
      #self.myCanvas.after( 1, self.paint )#Trigger point to paint
      

      
def main():
    PaintBox().mainloop()
    
if __name__=="__main__":
    main()

