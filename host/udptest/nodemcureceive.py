import socket

UDP_IP = "192.168.10.1"
UDP_PORT = 5006

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
print "listing on port", UDP_PORT


receivecounter = 0

while True:
  data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
  print "received message:", data, "receive counter @", receivecounter
  receivecounter += 1
