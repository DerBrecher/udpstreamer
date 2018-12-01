import socket
import random
import time
import sys

UDP_IP = "192.168.10.81"
UDP_RECEIVE_IP = "192.168.10.1"
UDP_PORT = 5006
UDP_RECEIVE_PORT = 5007
MESSAGE = ""
messagelength = int(sys.argv[1])
sendspersec = float(sys.argv[2])
period = 1/sendspersec

lasttime = 0
currenttime = 0


print "UDP target IP:", UDP_IP
print "message is", messagelength, "bytes long"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP Transmit

for x in range(messagelength):
  MESSAGE += str(chr(random.randint(0,255)))

MessageCounter = 0

while True:
  currenttime = time.time()
  if (currenttime - lasttime) > period:
    print "We are at Message", MessageCounter,"and ",(currenttime - lasttime)*1000, "ms since last send"
    MessageCounter += 1
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    lasttime = currenttime

