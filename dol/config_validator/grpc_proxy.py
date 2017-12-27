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
    '/nic/cli/', # To get tenjin_wrapper.
    '/dol/config_validator/'
]

ws_top = os.path.dirname(sys.argv[0]) + '/../..'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

from infra.common.logging import logger
from tenjin import *
from tenjin_wrapper import *

def genProxyServerMethods():
    tenjin_prefix = "//::"
    
    template = ws_top + '/dol/config_validator/hal_proto_gen_template.py'
    out_file = ws_top + '/dol/config_validator/hal_proto_gen.py'
    
    dic = {}
    with open(out_file, "w") as of:
        render_template(of, template, dic, './', prefix=tenjin_prefix)
        of.close()

genProxyServerMethods()
import hal_proto_gen

def initClient():
    HalChannel = grpc.insecure_channel('localhost:%s'%(port))
    grpc.channel_ready_future(HalChannel).result()
    print( "Connected to HAL" )

def serve():
    if 'CV_GRPC_PORT' in os.environ:
        port = os.environ['CV_GRPC_PORT']
    else:
        port = '50051'
    logger.info("Starting ConfigValidator GRPC Server on port %s" %(port))
    server = hal_proto_gen.proxyServer
    server.add_insecure_port('[::]:%s' %(port))
    server.start()
    try:
        while True:
            time.sleep(1000000)
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    initClient()
    serve()
