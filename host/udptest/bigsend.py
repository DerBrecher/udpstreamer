import socket
import random
import time
import sys

UDP_IP = "192.168.10.81"
UDP_RECEIVE_IP = "192.168.10.1"
UDP_PORT = 5006
UDP_RECEIVE_PORT = 5007
MESSAGE = ""

for x in range(int(sys.argv[1])):
  MESSAGE += str(random.randint(0,9))

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT
print "message:", MESSAGE
print "message is", len(MESSAGE), "bytes long" 

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP Transmit
sock2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP Receive

startsend = time.time()
sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
stopsend = time.time()

sock2.bind((UDP_RECEIVE_IP, UDP_PORT))

print "Sending took", (stopsend - startsend)*1000 , "ms"

while True:
    receivetime = time.time()
    data, addr = sock2.recvfrom(1024) # buffer size is 1024 bytes
    print "received message:", data
    print "Waited", (receivetime - stopsend)*1000, "ms for an answer"
    exit(1)
