from PIL import Image
import sys

x = int(sys.argv[1])
y = int(sys.argv[2])

im = Image.open('images/test2.png') # Can be many different formats.
pix = im.load()
print im.size  # Get the width and hight of the image for iterating over
print pix[x,y]  # Get the RGBA Value of the a pixel of an image
