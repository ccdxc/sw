import zmq
import os
import sys
from struct import *

def zmq_connect ():
    context = zmq.Context()
    #  Socket to talk to server
    socket = context.socket(zmq.REQ)
    model_sock_path = os.environ['MODEL_SOCK_PATH']
    zmqsockstr = 'ipc:///' + model_sock_path + '/zmqsock'
    socket.connect(zmqsockstr)
    return socket

def zmq_close (socket):
    socket.close()
    pass

def step_network_pkt (pkt, port):
    socket = zmq_connect()
    buff = pack('iiiiiq', 0, len(pkt), port-1, 0, 0, 0) + pkt
    socket.send(buff)
    msg = socket.recv()
    zmq_close(socket)
    pass

def get_next_pkt ():
    buffsize = 4096
    socket = zmq_connect()
    buff = pack('iiiiiq', 1, 0, 0, 0, 0, 0)
    socket.send(buff)
    msg = socket.recv()
    btype, size, port, cos, status, addr= unpack('iiiiiq', msg[0:32])
    eoffset = 32 + size
    pkt = msg[32:eoffset]
    zmq_close(socket)
    # DOL is 1-based port numbering, model is 0-based.
    return (pkt, port+1, cos)
    pass

def read_reg (addr):
    socket = zmq_connect()
    buff = pack('iiiiiq', 2, 4, 0, 0, 0, addr)
    socket.send(buff)
    msg = socket.recv()
    btype, size, port, cos, status, addr = unpack('iiiiiq', msg[0:32])
    eoffset = 32 + size
    zmq_close(socket)
    return (msg[32:eoffset])

def write_reg (addr, data):
    socket = zmq_connect()
    buff = pack('iiiiiq', 3, 4, 0, 0, 0, addr) + data
    socket.send(buff)
    msg = socket.recv()
    btype, size, port, cos, status, addr = unpack('iiiiiq', msg[0:32])
    zmq_close(socket)
    pass

def read_mem (addr, size):
    socket = zmq_connect()
    buff = pack('iiiiiq', 4, size, 0, 0, 0, addr)
    socket.send(buff)
    msg = socket.recv()
    btype, size, port, cos, status, addr = unpack('iiiiiq', msg[0:32])
    eoffset = 32 + size
    zmq_close(socket)
    return (msg[32:eoffset])

def write_mem (addr, data, size):
    socket = zmq_connect()
    buff = pack('iiiiiq', 5, size, 0, 0, 0, addr) + data
    socket.send(buff)
    msg = socket.recv()
    zmq_close(socket)
    pass

def doorbell (data, size):
    socket = zmq_connect()
    buff = pack('iiiiiq', 6, size, 0, 0, 0, 0) + data
    socket.send(buff)
    msg = socket.recv()
    zmq_close(socket)
    pass
