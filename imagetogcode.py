#!/usr/bin/env python
import sys
from PIL import Image
#verify args
if len(sys.argv) != 12:
   print "python [executable] [grayscale jpeg image] [G-CODE out file name] [max Z thickness] [work speed] [px per mm] [spindle motor speed] [safe Z mm] [move speed] [width mm] [height mm]\n"
   sys.exit(0)
#params
thickness = 2.50
workspeed = 50
pxmm = 10 #cancellare
tooldiam=0.8
spindle = 0
safez = 10
movespeed = 50
prodwidth=100.0
prodheight=100.0
tooldiam=0.8

#get args
thichness = sys.argv[3]
workspeed = sys.argv[4]
pxmm = sys.argv[5]
tooldiam = sys.argv[5]
spindle = sys.argv[6]
safez = sys.argv[7]
movespeed = sys.argv[8]
spindle = sys.argv[6]
safez = sys.argv[7]
movespeed = sys.argv[8]
prodwidth = sys.argv[9]
prodheight = sys.argv[10]
imagesave = sys.argv[11]
#calc final passes before resize image
finw = int(float(prodwidth)/float(tooldiam))
finh = int(float(prodheight)/float(tooldiam))


#G-CODE FILE
f = open(sys.argv[2],"w") 
#open image
im = Image.open(sys.argv[1])  
if im.mode != 'L':
   print "\nImage mode is %s, il wil be converted to grayscale. Please use a pure grayscale image for better final quality\n" % im.mode
   im=im.convert('L')

#resize image
#https://pillow.readthedocs.io/en/5.0.0/handbook/concepts.html#concept-filters
im = im.resize((finw,finh), Image.LANCZOS)
im.save(open(imagesave, 'w'))
#write start
f.write("%\n")
f.write("M3 S%s\nG21\nG00 Z%s F%s\n"% (spindle,safez,movespeed))

#start loop
xl=0;
for x in range(finw):
    for y in range(finh):
        #r,g,b = im.getpixel((x,y))
        #print "r=%d,r=%d,b=%d\n" % (r,g,b)
        #grayscale
        yc=finh-y-1
        ry=y
        if x % 2 != 0:
           ry=finh-y-1
           yc=y
        zval = (thickness/255)*im.getpixel((x,yc))
        if xl == 0:
           f.write("(Image width: %d px height: %d px - Tool mm: %s - Final width: %s mm height: %s mm)\n" % (im.width,im.height,tooldiam,prodwidth,prodheight))
           print "G01 Z-%.2f F%s\n" % (zval,workspeed)
           f.write("G01 Z-%.2f F%s\n" % (zval,workspeed)) 
        print "x=%d y=%d px=%d z=%.2f\n" % (x,y,im.getpixel((x,yc)),zval)
        print"G01 X%.2f Y%.2f Z-%.2f F%s\n" % (float(x)*float(tooldiam),float(ry)*float(tooldiam),zval,workspeed)
        f.write("G01 X%.2f Y%.2f Z-%.2f F%s\n" % (float(x)*float(tooldiam),float(ry)*float(tooldiam),zval,workspeed))
        xl+=1
        
#safe Z, stop motor, go home, end
f.write("G00 Z%s F%s\nM5\nG00 X0.00 Y0.00 F%s\nM2\n" % (safez,movespeed,movespeed));
f.write("%\n")
f.close()
