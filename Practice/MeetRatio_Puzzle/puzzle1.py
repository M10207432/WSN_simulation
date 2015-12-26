import Image
import turtle
from PIL import Image
import canvas
from Tkinter import *
import Tkinter
import tkMessageBox
    
def Drawpath():
    wn=turtle.Screen()
    alex=turtle.Turtle()

    alex.forward(50)
    alex.left(90)
    alex.right(30)

    wn.mainloop()
    
def Puzzle():
    

    top = Tkinter.Tk()

    C = Tkinter.Canvas(top, bg="blue", height=250, width=300)

    coord = 10, 50, 240, 210
    arc = C.create_arc(coord, start=0, extent=150, fill="red")

    C.pack()
    top.mainloop()
    '''
    root=Tk()
    root.protocol("WM_DELETE_WINDOW", root.quit())
    canvas=Canvas(root,width=810,height=600)
    canvas.pack()
    Button(root, text='Quit', command=root.quit).pack(side=BOTTOM, anchor=SE)

    offset = 5
    dx = 40
    xs = offset
    xf = xs + dx
    yf = ys = offset
    zs = zf = 460

    for i in range(1,21):
        canvas.create_line(xs, ys, xf, yf, width=2,fill="blue")
        canvas.create_line(xs, zs, xf, zf, width=2,fill="green")
        canvas.create_line(xs, ys, xs+1, ys, fill='white')
        canvas.create_line(xs, zs, xs+1, zs, fill='red')
        xs = xf
        ys = yf
        zs = zf
        xf = xf + dx
        yf = i*i + offset
        zf = 460 - i*i

    root.mainloop()
    '''
def main():
    img=Image.new('RGB', (255,255), "black")
    pixels = img.load()
    
    print "X Size:",img.size[0]
    print "Y Size:",img.size[1]
        
    for i in range(img.size[0]):
        for j in range(img.size[1]):
            pixels[i,j]=(i,j, 1)
    
    img.show()
    
if __name__=="__main__":
    #main()
    Puzzle()
