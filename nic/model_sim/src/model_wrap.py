import zmq
import os
import sys
import datetime
from struct import *

gl_socket = None

def error_exit():
    print("[%s] [ERROR]: Exiting due to Timeout: Model-NOT-Responding." %\
          str(datetime.datetime.now()))
    sys.exit(1)

def zmq_connect ():
    global gl_socket
    if gl_socket != None:
        return gl_socket

    _model_sock_timeo_sec = 60
    if 'MODEL_TIMEOUT' in os.environ:
        _model_sock_timeo_sec = int(os.environ['MODEL_TIMEOUT'])
    print("MODEL_TIMEOUT = %d" % _model_sock_timeo_sec)
    context = zmq.Context()
    #  Socket to talk to server
    socket = context.socket(zmq.REQ)
    socket.setsockopt(zmq.RCVTIMEO, _model_sock_timeo_sec * 1000) 
    socket.setsockopt(zmq.SNDTIMEO, _model_sock_timeo_sec * 1000)
    model_socket_name = 'zmqsock'
    if 'MODEL_SOCKET_NAME' in os.environ:
        model_socket_name = os.environ['MODEL_SOCKET_NAME']

    model_sock_path = os.environ['ZMQ_SOC_DIR']
    zmqsockstr = 'ipc:///' + model_sock_path + '/' + model_socket_name
    print("ZMQ SOCKET STRING = %s" % zmqsockstr)
    socket.connect(zmqsockstr)
    gl_socket = socket
    return socket

def zmq_close (socket):
    socket.close()
    return

def step_network_pkt (pkt, port):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 0, len(pkt), port-1, 0, 0, 0, 0, 0) + pkt
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def get_next_pkt ():
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 1, 0, 0, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr= unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    pkt = msg[40:eoffset]
    # DOL is 1-based port numbering, model is 0-based.
    return (pkt, port+1, cos)

def get_next_cpu_pkt ():
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 10, 0, 0, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr= unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    pkt = msg[40:eoffset]
    return pkt

def read_reg (addr):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 2, 4, 0, 0, 0, 0, 0, addr)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr = unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    return (msg[40:eoffset])

def write_reg (addr, data):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 3, 4, 0, 0, 0, 0, 0, addr) + data
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr = unpack('iiiiiiiQ', msg[0:40])
    return

def read_mem (addr, size):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 4, size, 0, 0, 0, 0, 0, addr)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr = unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    return (msg[40:eoffset])

def write_mem (addr, data, size):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 5, size, 0, 0, 0, 0, 0, addr) + data
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def write_mem_pcie (addr, data, size):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 24, size, 0, 0, 0, 0, 0, addr) + data
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def step_doorbell(addr, data):
    socket = zmq_connect()
    buff = pack('iiiiiiiQQ', 6, 0, 0, 0, 0, 0, 0, addr, data)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def step_tmr_wheel_update(slowfast, ctime):
    socket = zmq_connect()
    buff = pack('iiiiiiiQQ', 11, 0, 0, 0, 0, slowfast, ctime, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def exit_simulation ():
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 19, 0, 0, 0, 0, 0, 0, 0)
    socket.send(buff)
    return

def config_done ():
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 20, 0, 0, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def testcase_begin (tcid, loopid):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 21, tcid, loopid, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def testcase_end (tcid, loopid):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 22, tcid, loopid, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return

def eos_ignore_addr (addr, size):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 23, size, 0, 0, 0, 0, 0, addr)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    return
