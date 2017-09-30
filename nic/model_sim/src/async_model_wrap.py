import zmq
import os
import sys
import pdb
import random
from struct import *

rsock = None
esock = None

def error_exit():
    print("[ERROR]: Exiting due to Timeout: Model-NOT-Responding.")
    sys.exit(1)

def zmq_connect ():
    global rsock
    global esock
    _model_sock_timeo_sec = 300
    context = zmq.Context()
    model_sock_path = os.environ['MODEL_SOCK_PATH']
    #  Socket to talk to server
    rsock = context.socket(zmq.DEALER)
    esock.setsockopt(zmq.SNDHWM, 10000000) 
    esock.setsockopt(zmq.RCVHWM, 10000000) 
    rsock.setsockopt(zmq.RCVTIMEO, _model_sock_timeo_sec * 1000) 
    rsock.setsockopt(zmq.SNDTIMEO, _model_sock_timeo_sec * 1000) 
    rsock.setsockopt(zmq.IDENTITY, b"PEER2") 
    zmqsockstr1 = 'ipc:///' + model_sock_path + '/zmqsock1'
    rsock.connect(zmqsockstr1)
    
    esock = context.socket(zmq.SUB)
    esock.setsockopt(zmq.RCVHWM, 10000000) 
    esock.setsockopt(zmq.RCVTIMEO, _model_sock_timeo_sec * 1000) 
    esock.setsockopt(zmq.SNDTIMEO, _model_sock_timeo_sec * 1000) 
    esock.setsockopt(zmq.SUBSCRIBE, b"")
    zmqsockstr2 = 'ipc:///' + model_sock_path + '/zmqsock2'
    esock.connect(zmqsockstr2)
    return (rsock, esock)

def zmq_close ():
    global rsock
    global esock
    rsock.close()
    esock.close()
    pass

def step_network_pkt (pkt, port):
    global rsock
    global esock
    buff = pack('iiiiiiiQ', 0, len(pkt), port-1, 0, 0, 0, 0, 0) + pkt
    try:
        rsock.send(buff)
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    pass

def get_next_pkt ():
    global rsock
    global esock
    buff = pack('iiiiiiiQ', 1, 0, 0, 0, 0, 0, 0, 0)
    try:
        msg = esock.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr= unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    pkt = msg[40:eoffset]
    # DOL is 1-based port numbering, model is 0-based.
    return (pkt, port+1, cos)
    pass

def get_next_cpu_pkt ():
    global rsock
    global esock
    buff = pack('iiiiiiiQ', 10, 0, 0, 0, 0, 0, 0, 0)
    try:
        msg = esock.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr= unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    pkt = msg[40:eoffset]
    return pkt
    pass

def read_reg (addr):
    global rsock
    global esock
    buff = pack('iiiiiiiQ', 2, 4, 0, 0, 0, 0, 0, addr)
    try:
        rsock.send(buff)
        msg = rsock.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr = unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    return (msg[40:eoffset])

def write_reg (addr, data):
    global rsock
    global esock
    buff = pack('iiiiiiiQ', 3, 4, 0, 0, 0, 0, 0, addr) + data
    try:
        rsock.send(buff)
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    #btype, size, port, cos, status, slowfast, ctime, addr = unpack('iiiiiiiQ', msg[0:40])
    pass

def read_mem (addr, size):
    global rsock
    global esock
    buff = pack('iiiiiiiQ', 4, size, 0, 0, 0, 0, 0, addr)
    try:
        rsock.send(buff)
        msg = rsock.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr = unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    return (msg[40:eoffset])

def write_mem (addr, data, size):
    global rsock
    global esock
    buff = pack('iiiiiiiQ', 5, size, 0, 0, 0, 0, 0, addr) + data
    try:
        rsock.send(buff)
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    pass

def step_doorbell(addr, data):
    global rsock
    global esock
    buff = pack('iiiiiiiQQ', 6, 0, 0, 0, 0, 0, 0, addr, data)
    try:
        rsock.send(buff)
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    pass
