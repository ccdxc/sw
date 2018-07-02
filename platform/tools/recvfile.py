#!/usr/bin/python3

import socket
import sys

if len(sys.argv) < 4:
    print("Usage: <hostname> <port> <filename>")
    sys.exit(1)

hostname = sys.argv[1]
portnum = int(sys.argv[2])
filename = sys.argv[3]

s = socket.socket()
s.bind((hostname, portnum))
s.listen(10)

while True:
    sc, address = s.accept()

    print(address)
    i=1
    f = open(filename, 'wb')
    i=i+1
    while (True):       
        l = sc.recv(1024)
        while (l):
                f.write(l)
                l = sc.recv(1024)
    f.close()


    sc.close()

s.close()
