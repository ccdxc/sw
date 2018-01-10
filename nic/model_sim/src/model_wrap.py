import zmq
import os
import sys
from struct import *

def error_exit():
    print("[ERROR]: Exiting due to Timeout: Model-NOT-Responding.")
    sys.exit(1)

def zmq_connect ():
    _model_sock_timeo_sec = 60
    if 'MODEL_TIMEOUT' in os.environ:
        _model_sock_timeo_sec = int(os.environ['MODEL_TIMEOUT'])
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
    socket.connect(zmqsockstr)
    return socket

def zmq_close (socket):
    socket.close()
    pass

def step_network_pkt (pkt, port):
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 0, len(pkt), port-1, 0, 0, 0, 0, 0) + pkt
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    zmq_close(socket)
    pass

def get_next_pkt ():
    buffsize = 4096
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
    zmq_close(socket)
    # DOL is 1-based port numbering, model is 0-based.
    return (pkt, port+1, cos)
    pass

def get_next_cpu_pkt ():
    buffsize = 4096
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
    zmq_close(socket)
    return pkt
    pass

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
    zmq_close(socket)
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
    zmq_close(socket)
    pass

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
    zmq_close(socket)
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
    zmq_close(socket)
    pass

def step_doorbell(addr, data):
    socket = zmq_connect()
    buff = pack('iiiiiiiQQ', 6, 0, 0, 0, 0, 0, 0, addr, data)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    zmq_close(socket)
    pass

def step_tmr_wheel_update(slowfast, ctime):
    socket = zmq_connect()
    buff = pack('iiiiiiiQQ', 11, 0, 0, 0, 0, slowfast, ctime, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    zmq_close(socket)
    pass

def exit_simulation ():
    buffsize = 4096
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 19, 0, 0, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr= unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    pkt = msg[40:eoffset]
    zmq_close(socket)
    return pkt
    pass

def config_done ():
    buffsize = 4096
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 20, 0, 0, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    btype, size, port, cos, status, slowfast, ctime, addr= unpack('iiiiiiiQ', msg[0:40])
    eoffset = 40 + size
    pkt = msg[40:eoffset]
    zmq_close(socket)
    return pkt
    pass

def testcase_begin (tcid):
    buffsize = 4096
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 21, tcid, 0, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    zmq_close(socket)
    return

def testcase_end (tcid):
    buffsize = 4096
    socket = zmq_connect()
    buff = pack('iiiiiiiQ', 22, tcid, 0, 0, 0, 0, 0, 0)
    try:
        socket.send(buff)
        msg = socket.recv()
    except zmq.ZMQError as e:
        if e.errno == zmq.EAGAIN:
            error_exit()
    zmq_close(socket)
    return
