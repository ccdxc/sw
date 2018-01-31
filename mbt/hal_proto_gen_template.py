//:: import os, pdb, sys
//:: import importlib
import importlib
import grpc
import os
import sys
from concurrent import futures
import pdb
import config_mgr
from infra.common.logging import logger

proxyServer = grpc.server(futures.ThreadPoolExecutor(max_workers=1))
if 'HAL_GRPC_PORT' in os.environ:
    port = os.environ['HAL_GRPC_PORT']
else:
    port = '50054'
channel = grpc.insecure_channel('localhost:%s' %(port))

//:: ws_top = os.path.dirname(sys.argv[0]) + '/..'
//:: ws_top = os.path.abspath(ws_top)
//:: os.environ['WS_TOP'] = ws_top
//:: fullpath = ws_top + '/nic/gen/proto/'
//:: sys.path.insert(0, fullpath)
//:: fullpath = ws_top + '/nic/gen/proto/hal/'
//:: sys.path.insert(0, fullpath)
ws_top = os.path.dirname(sys.argv[0]) + '/..'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top
fullpath = ws_top + '/nic/gen/proto/hal/'
sys.path.insert(0, fullpath)

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
         response, err = config_mgr.CreateConfigFromDol(request)
         if err:
            logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.${table[0]}(request)
            logger.info("Received HAL response \n%s\n" %(response))
         return response

//::         #endfor
//::         # Add the service to the proxy server.
    pass
${fileGrpcName}.add_${servicerName}_to_server(${service[0]}(), proxyServer)

//::     #endfor
//:: #endfor


