import socket
import sys
import time
import png
from PIL import Image

UDP_IP = "192.168.10.81"
UDP_PORT = 5006

#Returns Tuple with: Width, Height, RGBList
def PNGtoRGBList(pngfile):
  image = png.Reader(filename="images/" + pngfile)
  width, height, pixelrgblist, metadata = image.read_flat()

  if metadata['alpha']: #Delete Alphachannel if we have one
    del pixelrgblist[3::4]

  return (width,height,pixelrgblist)

def sendPNG(imageFile):
  startconvert = time.time()
  frameTuple = PNGtoRGBList(imageFile)
  stopconvert = time.time()
  sendFrame(frameTuple)
  stopsending = time.time()

  print "Converting took: " + str(stopconvert - startconvert)
  print "Sending took: " + str(stopsending - stopconvert)

#takes in frameTuple(Width, Height, RGBArray)
def sendFrame(frameTuple):
  packageSizes = getPackageSizes(frameTuple[0:2])
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # open one UDP Socket for the Frame

  for packetNo in range(len(packageSizes)):
    MESSAGE =  chr(0x03) #Command Byte 0x03 = Frame Transmit
    MESSAGE += chr(packetNo + 1) #Package No
    MESSAGE += chr(len(packageSizes)) #Amout of total Packages
    MESSAGE += chr(frameTuple[0]) #Frame Width
    MESSAGE += chr(frameTuple[1]) #Frame height
    MESSAGE += chr(packageSizes[packetNo]) #PixelCount in Current Frame
    for pixel in frameTuple[2]: #add Pixel Values to Message
      MESSAGE += chr(pixel)

    #--only for Debuging
    #HEXMESSAGE = ":".join("{:02x}".format(ord(c)) for c in MESSAGE)
    #print str(packetNo) + ". MESSAGE: " + HEXMESSAGE
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

# Calculate Ammount of packages needed to transmit
def getPackageSizes(frameTuple):
  print frameTuple
  pixelCount = frameTuple[0]*frameTuple[1]
  packageSizes = []
  while pixelCount > 0: 
    if pixelCount >= 165:
      packageSizes.append(165)
    else:
      packageSizes.append(pixelCount)
    pixelCount -= 165
  return packageSizes

if __name__ == "__main__":
  sendPNG(sys.argv[1])
  #sendFrame()