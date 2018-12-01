import socket
import sys

UDP_IP = "192.168.10.81"
UDP_PORT = 5006
MESSAGE = ""

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT

width = int(sys.argv[1])
heigth = int(sys.argv[2])

pixelCount = width*heigth
wPixelCount = pixelCount
packageSizes = []

while wPixelCount > 0: # Calculate Ammount of packages for Frame
  if wPixelCount >= 165:
    packageSizes.append(165)
  else:
    packageSizes.append(wPixelCount)
  wPixelCount -= 165

print "Need Packages" + str(packageSizes)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # open UDP

debugCounter = 16

#build and send each package
for packetNo in range(len(packageSizes)):
  MESSAGE =  chr(0x03) #Command Byte 0x03 = Frame Transmit
  MESSAGE += chr(packetNo + 1) #Package No
  MESSAGE += chr(len(packageSizes)) #Amout of total Packages
  MESSAGE += chr(width) #Frame Width
  MESSAGE += chr(heigth) #Frame Heigth
  MESSAGE += chr(packageSizes[packetNo]) #PixelCount in Current Frame
  for pixel in range(packageSizes[packetNo]):
    if debugCounter > 240:
      debugCounter = 16
    MESSAGE += chr(debugCounter) #RED
    debugCounter += 1
    MESSAGE += chr(debugCounter) #GREEN
    debugCounter += 1
    MESSAGE += chr(debugCounter)   #BLUE
    debugCounter += 1

  HEXMESSAGE = ":".join("{:02x}".format(ord(c)) for c in MESSAGE)
  print str(packetNo) + ". MESSAGE: " + HEXMESSAGE
  sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))



