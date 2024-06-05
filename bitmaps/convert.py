"""
Script to parse bmp files into a txt file containing a C struct, representing the bitmap, which can be copied into the bitmaps.h file.
"""

from PIL import Image
import os

file_name = "projectile" #name of the .bmp file


#function to convert pixel to 16bit hex color
def to565string(pixel):
    r = (pixel[0] >> 3) & 0x1F
    g = (pixel[1] >> 2) & 0x3F
    b = (pixel[2] >> 3) & 0x1F
    rgb565 = (r << 11) + (g << 5) + b
    return hex(rgb565)

#open input and output file
__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
print(__location__)
file_path_img = os.path.join(__location__, file_name+".bmp") #input path
file_path_out = os.path.join(__location__, file_name+".txt") #output path
img = Image.open(file_path_img)
open(file_path_out,"w").close()

out= open(file_path_out,"a")
width= img.size[0]
height = img.size[1]
print("Image Size: %d x %d" % (width, height))

out.write("Bitmap " +  file_name + "={%d,%d, {" % (width, height) )

#now iterate over every pixel
for i in range(height):
    for j in range(width):
        pix = img.getpixel((j,i)) #get every pixel
        out.write(to565string(pix)) #convert pixel to 16bit color
        out.write("" if ((j == width-1) and (i == height-1)) else ", ") #separate every pixel with a coma, except the last one
out.write("}};")
out.close