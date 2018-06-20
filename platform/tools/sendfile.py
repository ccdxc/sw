#!/usr/bin/python3

import sys
import socket
import os

if len(sys.argv) < 4:
    print("Usage: <hostname> <port> <filename>")
    sys.exit(1)

hostname = sys.argv[1]
portnum = sys.argv[2]
filename = sys.argv[3]

file = open(filename, "rb")
blocksize = os.path.getsize(filename)
sock = socket.socket()
sock.connect((hostname, int(portnum)))
offset = 0

while True:
    sent = os.sendfile(sock.fileno(), file.fileno(), offset, blocksize)
    if sent == 0:
        break  # EOF
    offset += sent
