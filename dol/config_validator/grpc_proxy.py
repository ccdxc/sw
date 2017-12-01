#!/usr/bin/python3
# This is a step before integrating with Config validator. This acts as 
# a standalone pass-through proxy between dol and hal.
from concurrent import futures
import grpc
import os
import sys
import time
paths = [
    '/nic/gen/proto/',
    '/nic/gen/proto/hal/',
    '/nic/cli/' # To get tenjin_wrapper.
]

ws_top = os.path.dirname(sys.argv[0]) + '/../..'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

from tenjin import *
from tenjin_wrapper import *

def genProxyServerMethods():
    tenjin_prefix = "//::"
    
    template = './hal_proto_gen_template.py'
    out_file = './hal_proto_gen.py'
    
    dic = {}
    with open(out_file, "w") as of:
        render_template(of, template, dic, './', prefix=tenjin_prefix)
        of.close()

genProxyServerMethods()
import hal_proto_gen

def initClient():
    # Hardcode the port for now. Will redo this when integrating with DOL.
    HalChannel = grpc.insecure_channel('localhost:50054')
    grpc.channel_ready_future(HalChannel).result()
    print( "Connected to HAL" )

def serve():
    server = hal_proto_gen.proxyServer
    server.add_insecure_port('[::]:50051')
    server.start()
    try:
        while True:
            time.sleep(1000000)
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    initClient()
    serve()
