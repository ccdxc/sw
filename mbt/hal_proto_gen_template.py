//:: import os, sys
//:: import importlib
import importlib
import grpc
import os
import sys
from concurrent import futures
from infra.common.logging import logger
import ${proxy_handler}

asic = os.environ.get('ASIC', 'capri')

def grpc_server_start():
    proxyServer = grpc.server(futures.ThreadPoolExecutor(max_workers=1))
    return proxyServer

def grpc_client_connect():
    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    channel = grpc.insecure_channel('localhost:%s' %(port))
    return channel

def set_grpc_forward_channel(grpc_channel):
    global channel
    channel = grpc_channel

proxyServer = grpc_server_start()
//:: ws_top = ws_top  or os.path.abspath(os.path.dirname(sys.argv[0]) + '/..')
//:: os.environ['WS_TOP'] = ws_top
//:: fullpath = ws_top + '/nic/build/x86_64/iris/' + asic + '/gen/proto/'
//:: sys.path.insert(0, fullpath)

if 'WS_TOP' in os.environ:
    ws_top = os.environ['WS_TOP']
else:
    ws_top = os.path.dirname(sys.argv[0]) + '/..'
    ws_top = os.path.abspath(ws_top)
    os.environ['WS_TOP'] = ws_top

fullpath = ws_top + '/nic/build/x86_64/iris/' + asic + '/gen/proto/'
sys.path.insert(0, fullpath)

//:: grpc_service_reg_str = ''
//:: # Iterate through all files in the gen/proto/hal folder, and add create
//:: # python classes for the grpc proxy server
//:: for fileFullName in os.listdir(fullpath):
//::     if not fileFullName.endswith(".py"):
//::         continue
//::     #endif
//::     if 'grpc' in fileFullName:
//::         continue
//::     #endif
//::     fileName = fileFullName[:-3]
//::     # fileGrpcName = fileName[:fileName.find('_')] + '_pb2_grpc'
//::     fileGrpcName = fileName[:fileName.rfind('_')] + '_pb2_grpc'
//::     if not os.path.isfile(fullpath + fileGrpcName + ".py"):
//::         continue
//::     #endif
//::     fileModule = importlib.import_module(fileName)
//::     fileGrpcModule = importlib.import_module(fileGrpcName)
//::     for service in fileModule.DESCRIPTOR.services_by_name.items():
//::         stubName = service[0] + 'Stub'
//::         servicerName = service[0] + 'Servicer'
${fileName} = importlib.import_module('${fileName}')
${fileGrpcName} = importlib.import_module('${fileGrpcName}')
class ${service[0]}(${fileGrpcName}.${servicerName}):
//::         for table in service[1].methods_by_name.items():
    def ${table[0]}(self, request, context):
         stub = ${fileGrpcName}.${stubName}(channel)
         response, err = ${proxy_handler}.CreateConfigFromDol(request, '${table[0]}')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.${table[0]}(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

//::         #endfor
//::         # Add the service to the proxy server.
    pass
//::
//:: service_reg = fileGrpcName + '.add_' + servicerName + '_to_server(' + service[0] + '(), proxyServer)'
//:: grpc_service_reg_str += '    ' + service_reg + '\n'
//::

//::     #endfor
//:: #endfor
//::
def grpc_service_register(proxyServer):
${grpc_service_reg_str}
//::
grpc_service_register(proxyServer)
