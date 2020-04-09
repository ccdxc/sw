#!/usr/bin/python3
import grpc
import os
import sys
import time
asic = os.environ.get('ASIC', 'capri')
paths = [
    '/nic/build/x86_64/iris/' + asic + '/gen/proto/',
    '/nic/cli/', # To get tenjin_wrapper.
    '/mbt/'
]

if 'WS_TOP' in os.environ:
    ws_top = os.environ['WS_TOP']
else:
    ws_top = os.path.dirname(sys.argv[0]) + '/..'
    ws_top = os.path.abspath(ws_top)
    os.environ['WS_TOP'] = ws_top

for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

from infra.common.logging import logger
from tenjin import *
from tenjin_wrapper import *

def genProxyServerMethods(proxy_handler, template, out_file, ws_dir=None):
    tenjin_prefix = "//::"

    dic = {
            "proxy_handler" : proxy_handler,
            "ws_top" : ws_dir
          }
    with open(out_file, "w") as of:
        render_template(of, template, dic, './', prefix=tenjin_prefix)
        of.close()

def initClient():
    HalChannel = grpc.insecure_channel('localhost:%s'%(port))
    grpc.channel_ready_future(HalChannel).result()
    print( "Connected to HAL" )

def serve(server):
    if 'MBT_GRPC_PORT' in os.environ:
        port = os.environ['MBT_GRPC_PORT']
    else:
        port = '50051'
    logger.info("Starting Model Based Tester GRPC Server on port %s" %(port))
    server.add_insecure_port('[::]:%s' %(port))
    server.start()
    try:
        while True:
            time.sleep(1000000)
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    serve()
