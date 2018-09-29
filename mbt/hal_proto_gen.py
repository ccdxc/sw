import importlib
import grpc
import os
import sys
from concurrent import futures
from infra.common.logging import logger
import config_mgr

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

if 'WS_TOP' in os.environ:
    ws_top = os.environ['WS_TOP']
else:
    ws_top = os.path.dirname(sys.argv[0]) + '/..'
    ws_top = os.path.abspath(ws_top)
    os.environ['WS_TOP'] = ws_top

fullpath = ws_top + '/nic/build/x86_64/iris/gen/proto/'
sys.path.insert(0, fullpath)

cpucb_pb2 = importlib.import_module('cpucb_pb2')
cpucb_pb2_grpc = importlib.import_module('cpucb_pb2_grpc')
class CpuCb(cpucb_pb2_grpc.CpuCbServicer):
    def CpuCbCreate(self, request, context):
         stub = cpucb_pb2_grpc.CpuCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CpuCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CpuCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CpuCbUpdate(self, request, context):
         stub = cpucb_pb2_grpc.CpuCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CpuCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CpuCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CpuCbDelete(self, request, context):
         stub = cpucb_pb2_grpc.CpuCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CpuCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CpuCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CpuCbGet(self, request, context):
         stub = cpucb_pb2_grpc.CpuCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CpuCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CpuCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

crypto_keys_pb2 = importlib.import_module('crypto_keys_pb2')
crypto_keys_pb2_grpc = importlib.import_module('crypto_keys_pb2_grpc')
class CryptoKey(crypto_keys_pb2_grpc.CryptoKeyServicer):
    def CryptoKeyCreate(self, request, context):
         stub = crypto_keys_pb2_grpc.CryptoKeyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CryptoKeyCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CryptoKeyCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CryptoKeyRead(self, request, context):
         stub = crypto_keys_pb2_grpc.CryptoKeyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CryptoKeyRead')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CryptoKeyRead(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CryptoKeyUpdate(self, request, context):
         stub = crypto_keys_pb2_grpc.CryptoKeyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CryptoKeyUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CryptoKeyUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CryptoKeyDelete(self, request, context):
         stub = crypto_keys_pb2_grpc.CryptoKeyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CryptoKeyDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CryptoKeyDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

acl_pb2 = importlib.import_module('acl_pb2')
acl_pb2_grpc = importlib.import_module('acl_pb2_grpc')
class Acl(acl_pb2_grpc.AclServicer):
    def AclCreate(self, request, context):
         stub = acl_pb2_grpc.AclStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'AclCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.AclCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def AclUpdate(self, request, context):
         stub = acl_pb2_grpc.AclStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'AclUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.AclUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def AclDelete(self, request, context):
         stub = acl_pb2_grpc.AclStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'AclDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.AclDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def AclGet(self, request, context):
         stub = acl_pb2_grpc.AclStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'AclGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.AclGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

barco_rings_pb2 = importlib.import_module('barco_rings_pb2')
barco_rings_pb2_grpc = importlib.import_module('barco_rings_pb2_grpc')
class BarcoRings(barco_rings_pb2_grpc.BarcoRingsServicer):
    def GetOpaqueTagAddr(self, request, context):
         stub = barco_rings_pb2_grpc.BarcoRingsStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'GetOpaqueTagAddr')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.GetOpaqueTagAddr(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def BarcoGetReqDescrEntry(self, request, context):
         stub = barco_rings_pb2_grpc.BarcoRingsStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'BarcoGetReqDescrEntry')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.BarcoGetReqDescrEntry(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def BarcoGetRingMeta(self, request, context):
         stub = barco_rings_pb2_grpc.BarcoRingsStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'BarcoGetRingMeta')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.BarcoGetRingMeta(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

dos_pb2 = importlib.import_module('dos_pb2')
dos_pb2_grpc = importlib.import_module('dos_pb2_grpc')
class Dos(dos_pb2_grpc.DosServicer):
    def DoSPolicyCreate(self, request, context):
         stub = dos_pb2_grpc.DosStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DoSPolicyCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DoSPolicyCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DoSPolicyUpdate(self, request, context):
         stub = dos_pb2_grpc.DosStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DoSPolicyUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DoSPolicyUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DoSPolicyDelete(self, request, context):
         stub = dos_pb2_grpc.DosStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DoSPolicyDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DoSPolicyDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DoSPolicyGet(self, request, context):
         stub = dos_pb2_grpc.DosStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DoSPolicyGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DoSPolicyGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

crypto_apis_pb2 = importlib.import_module('crypto_apis_pb2')
crypto_apis_pb2_grpc = importlib.import_module('crypto_apis_pb2_grpc')
class CryptoApis(crypto_apis_pb2_grpc.CryptoApisServicer):
    def CryptoApiInvoke(self, request, context):
         stub = crypto_apis_pb2_grpc.CryptoApisStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CryptoApiInvoke')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CryptoApiInvoke(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

descriptor_aol_pb2 = importlib.import_module('descriptor_aol_pb2')
descriptor_aol_pb2_grpc = importlib.import_module('descriptor_aol_pb2_grpc')
class DescrAol(descriptor_aol_pb2_grpc.DescrAolServicer):
    def DescrAolGet(self, request, context):
         stub = descriptor_aol_pb2_grpc.DescrAolStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DescrAolGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DescrAolGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

debug_pb2 = importlib.import_module('debug_pb2')
debug_pb2_grpc = importlib.import_module('debug_pb2_grpc')
class Debug(debug_pb2_grpc.DebugServicer):
    def RegisterGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RegisterGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RegisterGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RegisterUpdate(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RegisterUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RegisterUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MemoryGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MemoryGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MemoryGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MemoryUpdate(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MemoryUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MemoryUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MemoryRawGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MemoryRawGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MemoryRawGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MemoryRawUpdate(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MemoryRawUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MemoryRawUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MpuTraceUpdate(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MpuTraceUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MpuTraceUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MemTrackGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MemTrackGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MemTrackGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SlabGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SlabGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SlabGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def HashTableGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'HashTableGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.HashTableGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TraceUpdate(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TraceUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TraceUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TraceGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TraceGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TraceGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ThreadGet(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ThreadGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ThreadGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FlushLogs(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FlushLogs')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FlushLogs(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def GenericOpn(self, request, context):
         stub = debug_pb2_grpc.DebugStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'GenericOpn')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.GenericOpn(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

endpoint_pb2 = importlib.import_module('endpoint_pb2')
endpoint_pb2_grpc = importlib.import_module('endpoint_pb2_grpc')
class Endpoint(endpoint_pb2_grpc.EndpointServicer):
    def EndpointCreate(self, request, context):
         stub = endpoint_pb2_grpc.EndpointStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'EndpointCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.EndpointCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def EndpointUpdate(self, request, context):
         stub = endpoint_pb2_grpc.EndpointStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'EndpointUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.EndpointUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def EndpointDelete(self, request, context):
         stub = endpoint_pb2_grpc.EndpointStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'EndpointDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.EndpointDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def EndpointGet(self, request, context):
         stub = endpoint_pb2_grpc.EndpointStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'EndpointGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.EndpointGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FilterCreate(self, request, context):
         stub = endpoint_pb2_grpc.EndpointStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FilterCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FilterCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FilterDelete(self, request, context):
         stub = endpoint_pb2_grpc.EndpointStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FilterDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FilterDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FilterGet(self, request, context):
         stub = endpoint_pb2_grpc.EndpointStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FilterGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FilterGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

event_pb2 = importlib.import_module('event_pb2')
event_pb2_grpc = importlib.import_module('event_pb2_grpc')
class Event(event_pb2_grpc.EventServicer):
    def EventListen(self, request, context):
         stub = event_pb2_grpc.EventStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'EventListen')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.EventListen(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

ipseccb_pb2 = importlib.import_module('ipseccb_pb2')
ipseccb_pb2_grpc = importlib.import_module('ipseccb_pb2_grpc')
class IpsecCb(ipseccb_pb2_grpc.IpsecCbServicer):
    def IpsecCbCreate(self, request, context):
         stub = ipseccb_pb2_grpc.IpsecCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecCbUpdate(self, request, context):
         stub = ipseccb_pb2_grpc.IpsecCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecCbDelete(self, request, context):
         stub = ipseccb_pb2_grpc.IpsecCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecCbGet(self, request, context):
         stub = ipseccb_pb2_grpc.IpsecCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

gft_pb2 = importlib.import_module('gft_pb2')
gft_pb2_grpc = importlib.import_module('gft_pb2_grpc')
class Gft(gft_pb2_grpc.GftServicer):
    def GftExactMatchProfileCreate(self, request, context):
         stub = gft_pb2_grpc.GftStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'GftExactMatchProfileCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.GftExactMatchProfileCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def GftHeaderTranspositionProfileCreate(self, request, context):
         stub = gft_pb2_grpc.GftStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'GftHeaderTranspositionProfileCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.GftHeaderTranspositionProfileCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def GftExactMatchFlowEntryCreate(self, request, context):
         stub = gft_pb2_grpc.GftStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'GftExactMatchFlowEntryCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.GftExactMatchFlowEntryCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

l4lb_pb2 = importlib.import_module('l4lb_pb2')
l4lb_pb2_grpc = importlib.import_module('l4lb_pb2_grpc')
class L4Lb(l4lb_pb2_grpc.L4LbServicer):
    def L4LbServiceCreate(self, request, context):
         stub = l4lb_pb2_grpc.L4LbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'L4LbServiceCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.L4LbServiceCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

multicast_pb2 = importlib.import_module('multicast_pb2')
multicast_pb2_grpc = importlib.import_module('multicast_pb2_grpc')
class Multicast(multicast_pb2_grpc.MulticastServicer):
    def MulticastEntryCreate(self, request, context):
         stub = multicast_pb2_grpc.MulticastStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MulticastEntryCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MulticastEntryCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MulticastEntryUpdate(self, request, context):
         stub = multicast_pb2_grpc.MulticastStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MulticastEntryUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MulticastEntryUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MulticastEntryDelete(self, request, context):
         stub = multicast_pb2_grpc.MulticastStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MulticastEntryDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MulticastEntryDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MulticastEntryGet(self, request, context):
         stub = multicast_pb2_grpc.MulticastStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MulticastEntryGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MulticastEntryGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

l2segment_pb2 = importlib.import_module('l2segment_pb2')
l2segment_pb2_grpc = importlib.import_module('l2segment_pb2_grpc')
class L2Segment(l2segment_pb2_grpc.L2SegmentServicer):
    def L2SegmentCreate(self, request, context):
         stub = l2segment_pb2_grpc.L2SegmentStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'L2SegmentCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.L2SegmentCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def L2SegmentUpdate(self, request, context):
         stub = l2segment_pb2_grpc.L2SegmentStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'L2SegmentUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.L2SegmentUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def L2SegmentDelete(self, request, context):
         stub = l2segment_pb2_grpc.L2SegmentStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'L2SegmentDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.L2SegmentDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def L2SegmentGet(self, request, context):
         stub = l2segment_pb2_grpc.L2SegmentStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'L2SegmentGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.L2SegmentGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

nat_pb2 = importlib.import_module('nat_pb2')
nat_pb2_grpc = importlib.import_module('nat_pb2_grpc')
class Nat(nat_pb2_grpc.NatServicer):
    def NatPoolCreate(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPoolCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPoolCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatPoolUpdate(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPoolUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPoolUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatPoolDelete(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPoolDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPoolDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatPoolGet(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPoolGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPoolGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatPolicyCreate(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPolicyCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPolicyCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatPolicyUpdate(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPolicyUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPolicyUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatPolicyDelete(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPolicyDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPolicyDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatPolicyGet(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatPolicyGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatPolicyGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatMappingCreate(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatMappingCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatMappingCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatMappingDelete(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatMappingDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatMappingDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NatMappingGet(self, request, context):
         stub = nat_pb2_grpc.NatStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NatMappingGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NatMappingGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

nic_pb2 = importlib.import_module('nic_pb2')
nic_pb2_grpc = importlib.import_module('nic_pb2_grpc')
class Nic(nic_pb2_grpc.NicServicer):
    def DeviceCreate(self, request, context):
         stub = nic_pb2_grpc.NicStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DeviceCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DeviceCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DeviceUpdate(self, request, context):
         stub = nic_pb2_grpc.NicStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DeviceUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DeviceUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DeviceGet(self, request, context):
         stub = nic_pb2_grpc.NicStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DeviceGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DeviceGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

ipsec_pb2 = importlib.import_module('ipsec_pb2')
ipsec_pb2_grpc = importlib.import_module('ipsec_pb2_grpc')
class Ipsec(ipsec_pb2_grpc.IpsecServicer):
    def IpsecRuleCreate(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecRuleCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecRuleCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecRuleUpdate(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecRuleUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecRuleUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecRuleDelete(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecRuleDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecRuleDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecRuleGet(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecRuleGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecRuleGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSAEncryptCreate(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSAEncryptCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSAEncryptCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSAEncryptUpdate(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSAEncryptUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSAEncryptUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSAEncryptDelete(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSAEncryptDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSAEncryptDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSAEncryptGet(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSAEncryptGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSAEncryptGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSADecryptCreate(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSADecryptCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSADecryptCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSADecryptUpdate(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSADecryptUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSADecryptUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSADecryptDelete(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSADecryptDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSADecryptDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def IpsecSADecryptGet(self, request, context):
         stub = ipsec_pb2_grpc.IpsecStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'IpsecSADecryptGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.IpsecSADecryptGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

internal_pb2 = importlib.import_module('internal_pb2')
internal_pb2_grpc = importlib.import_module('internal_pb2_grpc')
class Internal(internal_pb2_grpc.InternalServicer):
    def GetProgramAddress(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'GetProgramAddress')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.GetProgramAddress(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def AllocHbmAddress(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'AllocHbmAddress')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.AllocHbmAddress(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ConfigureLifBdf(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ConfigureLifBdf')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ConfigureLifBdf(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SoftwarePhvInject(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SoftwarePhvInject')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SoftwarePhvInject(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SoftwarePhvGet(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SoftwarePhvGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SoftwarePhvGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def LogFlow(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'LogFlow')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.LogFlow(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def QuiesceMsgSnd(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'QuiesceMsgSnd')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.QuiesceMsgSnd(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def QuiesceStart(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'QuiesceStart')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.QuiesceStart(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def QuiesceStop(self, request, context):
         stub = internal_pb2_grpc.InternalStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'QuiesceStop')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.QuiesceStop(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

internal_pb2 = importlib.import_module('internal_pb2')
internal_pb2_grpc = importlib.import_module('internal_pb2_grpc')
class SoftwarePhv(internal_pb2_grpc.SoftwarePhvServicer):
    pass

interface_pb2 = importlib.import_module('interface_pb2')
interface_pb2_grpc = importlib.import_module('interface_pb2_grpc')
class Interface(interface_pb2_grpc.InterfaceServicer):
    def LifCreate(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'LifCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.LifCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def LifUpdate(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'LifUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.LifUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def LifDelete(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'LifDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.LifDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def LifGet(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'LifGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.LifGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def LifGetQState(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'LifGetQState')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.LifGetQState(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def LifSetQState(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'LifSetQState')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.LifSetQState(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def InterfaceCreate(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'InterfaceCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.InterfaceCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def InterfaceUpdate(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'InterfaceUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.InterfaceUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def InterfaceDelete(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'InterfaceDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.InterfaceDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def InterfaceGet(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'InterfaceGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.InterfaceGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def AddL2SegmentOnUplink(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'AddL2SegmentOnUplink')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.AddL2SegmentOnUplink(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DelL2SegmentOnUplink(self, request, context):
         stub = interface_pb2_grpc.InterfaceStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DelL2SegmentOnUplink')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DelL2SegmentOnUplink(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

rawrcb_pb2 = importlib.import_module('rawrcb_pb2')
rawrcb_pb2_grpc = importlib.import_module('rawrcb_pb2_grpc')
class RawrCb(rawrcb_pb2_grpc.RawrCbServicer):
    def RawrCbCreate(self, request, context):
         stub = rawrcb_pb2_grpc.RawrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawrCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawrCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RawrCbUpdate(self, request, context):
         stub = rawrcb_pb2_grpc.RawrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawrCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawrCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RawrCbDelete(self, request, context):
         stub = rawrcb_pb2_grpc.RawrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawrCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawrCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RawrCbGet(self, request, context):
         stub = rawrcb_pb2_grpc.RawrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawrCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawrCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

nw_pb2 = importlib.import_module('nw_pb2')
nw_pb2_grpc = importlib.import_module('nw_pb2_grpc')
class Network(nw_pb2_grpc.NetworkServicer):
    def NetworkCreate(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NetworkCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NetworkCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NetworkUpdate(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NetworkUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NetworkUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NetworkDelete(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NetworkDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NetworkDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NetworkGet(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NetworkGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NetworkGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NexthopCreate(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NexthopCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NexthopCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NexthopUpdate(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NexthopUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NexthopUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NexthopDelete(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NexthopDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NexthopDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def NexthopGet(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'NexthopGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.NexthopGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RouteCreate(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RouteCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RouteCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RouteUpdate(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RouteUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RouteUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RouteDelete(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RouteDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RouteDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RouteGet(self, request, context):
         stub = nw_pb2_grpc.NetworkStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RouteGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RouteGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

rawccb_pb2 = importlib.import_module('rawccb_pb2')
rawccb_pb2_grpc = importlib.import_module('rawccb_pb2_grpc')
class RawcCb(rawccb_pb2_grpc.RawcCbServicer):
    def RawcCbCreate(self, request, context):
         stub = rawccb_pb2_grpc.RawcCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawcCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawcCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RawcCbUpdate(self, request, context):
         stub = rawccb_pb2_grpc.RawcCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawcCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawcCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RawcCbDelete(self, request, context):
         stub = rawccb_pb2_grpc.RawcCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawcCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawcCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RawcCbGet(self, request, context):
         stub = rawccb_pb2_grpc.RawcCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RawcCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RawcCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

port_pb2 = importlib.import_module('port_pb2')
port_pb2_grpc = importlib.import_module('port_pb2_grpc')
class Port(port_pb2_grpc.PortServicer):
    def PortCreate(self, request, context):
         stub = port_pb2_grpc.PortStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'PortCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.PortCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def PortUpdate(self, request, context):
         stub = port_pb2_grpc.PortStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'PortUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.PortUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def PortDelete(self, request, context):
         stub = port_pb2_grpc.PortStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'PortDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.PortDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def PortGet(self, request, context):
         stub = port_pb2_grpc.PortStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'PortGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.PortGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def PortInfoGet(self, request, context):
         stub = port_pb2_grpc.PortStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'PortInfoGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.PortInfoGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

proxyrcb_pb2 = importlib.import_module('proxyrcb_pb2')
proxyrcb_pb2_grpc = importlib.import_module('proxyrcb_pb2_grpc')
class ProxyrCb(proxyrcb_pb2_grpc.ProxyrCbServicer):
    def ProxyrCbCreate(self, request, context):
         stub = proxyrcb_pb2_grpc.ProxyrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyrCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyrCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyrCbUpdate(self, request, context):
         stub = proxyrcb_pb2_grpc.ProxyrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyrCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyrCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyrCbDelete(self, request, context):
         stub = proxyrcb_pb2_grpc.ProxyrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyrCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyrCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyrCbGet(self, request, context):
         stub = proxyrcb_pb2_grpc.ProxyrCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyrCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyrCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

rdma_pb2 = importlib.import_module('rdma_pb2')
rdma_pb2_grpc = importlib.import_module('rdma_pb2_grpc')
class Rdma(rdma_pb2_grpc.RdmaServicer):
    def RdmaQpCreate(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaQpCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaQpCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaQpUpdate(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaQpUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaQpUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaAhCreate(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaAhCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaAhCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaCqCreate(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaCqCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaCqCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaEqCreate(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaEqCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaEqCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaMemReg(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaMemReg')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaMemReg(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaAllocLkey(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaAllocLkey')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaAllocLkey(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaMemWindow(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaMemWindow')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaMemWindow(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def RdmaAqCreate(self, request, context):
         stub = rdma_pb2_grpc.RdmaStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'RdmaAqCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.RdmaAqCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

proxyccb_pb2 = importlib.import_module('proxyccb_pb2')
proxyccb_pb2_grpc = importlib.import_module('proxyccb_pb2_grpc')
class ProxycCb(proxyccb_pb2_grpc.ProxycCbServicer):
    def ProxycCbCreate(self, request, context):
         stub = proxyccb_pb2_grpc.ProxycCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxycCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxycCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxycCbUpdate(self, request, context):
         stub = proxyccb_pb2_grpc.ProxycCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxycCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxycCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxycCbDelete(self, request, context):
         stub = proxyccb_pb2_grpc.ProxycCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxycCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxycCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxycCbGet(self, request, context):
         stub = proxyccb_pb2_grpc.ProxycCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxycCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxycCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

table_pb2 = importlib.import_module('table_pb2')
table_pb2_grpc = importlib.import_module('table_pb2_grpc')
class Table(table_pb2_grpc.TableServicer):
    def TableMetadataGet(self, request, context):
         stub = table_pb2_grpc.TableStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TableMetadataGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TableMetadataGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TableGet(self, request, context):
         stub = table_pb2_grpc.TableStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TableGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TableGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

proxy_pb2 = importlib.import_module('proxy_pb2')
proxy_pb2_grpc = importlib.import_module('proxy_pb2_grpc')
class Proxy(proxy_pb2_grpc.ProxyServicer):
    def ProxyEnable(self, request, context):
         stub = proxy_pb2_grpc.ProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyEnable')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyEnable(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyUpdate(self, request, context):
         stub = proxy_pb2_grpc.ProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyDisable(self, request, context):
         stub = proxy_pb2_grpc.ProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyDisable')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyDisable(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyGet(self, request, context):
         stub = proxy_pb2_grpc.ProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyFlowConfig(self, request, context):
         stub = proxy_pb2_grpc.ProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyFlowConfig')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyFlowConfig(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyGetFlowInfo(self, request, context):
         stub = proxy_pb2_grpc.ProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyGetFlowInfo')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyGetFlowInfo(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def ProxyGlobalCfg(self, request, context):
         stub = proxy_pb2_grpc.ProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ProxyGlobalCfg')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ProxyGlobalCfg(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

tls_proxy_cb_pb2 = importlib.import_module('tls_proxy_cb_pb2')
tls_proxy_cb_pb2_grpc = importlib.import_module('tls_proxy_cb_pb2_grpc')
class TlsCb(tls_proxy_cb_pb2_grpc.TlsCbServicer):
    def TlsCbCreate(self, request, context):
         stub = tls_proxy_cb_pb2_grpc.TlsCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TlsCbUpdate(self, request, context):
         stub = tls_proxy_cb_pb2_grpc.TlsCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TlsCbDelete(self, request, context):
         stub = tls_proxy_cb_pb2_grpc.TlsCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TlsCbGet(self, request, context):
         stub = tls_proxy_cb_pb2_grpc.TlsCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

system_pb2 = importlib.import_module('system_pb2')
system_pb2_grpc = importlib.import_module('system_pb2_grpc')
class System(system_pb2_grpc.SystemServicer):
    def ApiStatsGet(self, request, context):
         stub = system_pb2_grpc.SystemStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'ApiStatsGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.ApiStatsGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SystemGet(self, request, context):
         stub = system_pb2_grpc.SystemStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SystemGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SystemGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

qos_pb2 = importlib.import_module('qos_pb2')
qos_pb2_grpc = importlib.import_module('qos_pb2_grpc')
class QOS(qos_pb2_grpc.QOSServicer):
    def QosClassCreate(self, request, context):
         stub = qos_pb2_grpc.QOSStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'QosClassCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.QosClassCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def QosClassUpdate(self, request, context):
         stub = qos_pb2_grpc.QOSStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'QosClassUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.QosClassUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def QosClassDelete(self, request, context):
         stub = qos_pb2_grpc.QOSStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'QosClassDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.QosClassDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def QosClassGet(self, request, context):
         stub = qos_pb2_grpc.QOSStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'QosClassGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.QosClassGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CoppUpdate(self, request, context):
         stub = qos_pb2_grpc.QOSStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CoppUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CoppUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CoppGet(self, request, context):
         stub = qos_pb2_grpc.QOSStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CoppGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CoppGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

tcp_proxy_cb_pb2 = importlib.import_module('tcp_proxy_cb_pb2')
tcp_proxy_cb_pb2_grpc = importlib.import_module('tcp_proxy_cb_pb2_grpc')
class TcpCb(tcp_proxy_cb_pb2_grpc.TcpCbServicer):
    def TcpCbCreate(self, request, context):
         stub = tcp_proxy_cb_pb2_grpc.TcpCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpCbUpdate(self, request, context):
         stub = tcp_proxy_cb_pb2_grpc.TcpCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpCbDelete(self, request, context):
         stub = tcp_proxy_cb_pb2_grpc.TcpCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpCbGet(self, request, context):
         stub = tcp_proxy_cb_pb2_grpc.TcpCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

nwsec_pb2 = importlib.import_module('nwsec_pb2')
nwsec_pb2_grpc = importlib.import_module('nwsec_pb2_grpc')
class NwSecurity(nwsec_pb2_grpc.NwSecurityServicer):
    def SecurityProfileCreate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityProfileCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityProfileCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityProfileUpdate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityProfileUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityProfileUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityProfileDelete(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityProfileDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityProfileDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityProfileGet(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityProfileGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityProfileGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupPolicyCreate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupPolicyCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupPolicyCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupPolicyUpdate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupPolicyUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupPolicyUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupPolicyDelete(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupPolicyDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupPolicyDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupPolicyGet(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupPolicyGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupPolicyGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupCreate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupUpdate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupDelete(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityGroupGet(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityGroupGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityGroupGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityPolicyCreate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityPolicyCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityPolicyCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityPolicyUpdate(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityPolicyUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityPolicyUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityPolicyDelete(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityPolicyDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityPolicyDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SecurityPolicyGet(self, request, context):
         stub = nwsec_pb2_grpc.NwSecurityStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SecurityPolicyGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SecurityPolicyGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

tls_proxy_cb2_pb2 = importlib.import_module('tls_proxy_cb2_pb2')
tls_proxy_cb2_pb2_grpc = importlib.import_module('tls_proxy_cb2_pb2_grpc')
class TlsProxyCb(tls_proxy_cb2_pb2_grpc.TlsProxyCbServicer):
    def TlsProxyCbCreate(self, request, context):
         stub = tls_proxy_cb2_pb2_grpc.TlsProxyCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsProxyCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsProxyCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TlsProxyCbUpdate(self, request, context):
         stub = tls_proxy_cb2_pb2_grpc.TlsProxyCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsProxyCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsProxyCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TlsProxyCbDelete(self, request, context):
         stub = tls_proxy_cb2_pb2_grpc.TlsProxyCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsProxyCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsProxyCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TlsProxyCbGet(self, request, context):
         stub = tls_proxy_cb2_pb2_grpc.TlsProxyCbStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TlsProxyCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TlsProxyCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

wring_pb2 = importlib.import_module('wring_pb2')
wring_pb2_grpc = importlib.import_module('wring_pb2_grpc')
class WRing(wring_pb2_grpc.WRingServicer):
    def WRingCreate(self, request, context):
         stub = wring_pb2_grpc.WRingStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'WRingCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.WRingCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def WRingUpdate(self, request, context):
         stub = wring_pb2_grpc.WRingStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'WRingUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.WRingUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def WRingDelete(self, request, context):
         stub = wring_pb2_grpc.WRingStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'WRingDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.WRingDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def WRingGetEntries(self, request, context):
         stub = wring_pb2_grpc.WRingStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'WRingGetEntries')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.WRingGetEntries(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def WRingGetMeta(self, request, context):
         stub = wring_pb2_grpc.WRingStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'WRingGetMeta')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.WRingGetMeta(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def WRingSetMeta(self, request, context):
         stub = wring_pb2_grpc.WRingStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'WRingSetMeta')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.WRingSetMeta(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

session_pb2 = importlib.import_module('session_pb2')
session_pb2_grpc = importlib.import_module('session_pb2_grpc')
class Session(session_pb2_grpc.SessionServicer):
    def SessionCreate(self, request, context):
         stub = session_pb2_grpc.SessionStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SessionCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SessionCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SessionDelete(self, request, context):
         stub = session_pb2_grpc.SessionStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SessionDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SessionDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def SessionGet(self, request, context):
         stub = session_pb2_grpc.SessionStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'SessionGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.SessionGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

telemetry_pb2 = importlib.import_module('telemetry_pb2')
telemetry_pb2_grpc = importlib.import_module('telemetry_pb2_grpc')
class Telemetry(telemetry_pb2_grpc.TelemetryServicer):
    def CollectorCreate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CollectorCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CollectorCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CollectorUpdate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CollectorUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CollectorUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CollectorDelete(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CollectorDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CollectorDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def CollectorGet(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'CollectorGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.CollectorGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FlowMonitorRuleCreate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FlowMonitorRuleCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FlowMonitorRuleCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FlowMonitorRuleUpdate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FlowMonitorRuleUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FlowMonitorRuleUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FlowMonitorRuleDelete(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FlowMonitorRuleDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FlowMonitorRuleDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def FlowMonitorRuleGet(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'FlowMonitorRuleGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.FlowMonitorRuleGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DropMonitorRuleCreate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DropMonitorRuleCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DropMonitorRuleCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DropMonitorRuleUpdate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DropMonitorRuleUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DropMonitorRuleUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DropMonitorRuleDelete(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DropMonitorRuleDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DropMonitorRuleDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def DropMonitorRuleGet(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'DropMonitorRuleGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.DropMonitorRuleGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MirrorSessionCreate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MirrorSessionCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MirrorSessionCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MirrorSessionUpdate(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MirrorSessionUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MirrorSessionUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MirrorSessionDelete(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MirrorSessionDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MirrorSessionDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def MirrorSessionGet(self, request, context):
         stub = telemetry_pb2_grpc.TelemetryStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'MirrorSessionGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.MirrorSessionGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

vrf_pb2 = importlib.import_module('vrf_pb2')
vrf_pb2_grpc = importlib.import_module('vrf_pb2_grpc')
class Vrf(vrf_pb2_grpc.VrfServicer):
    def VrfCreate(self, request, context):
         stub = vrf_pb2_grpc.VrfStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'VrfCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.VrfCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def VrfUpdate(self, request, context):
         stub = vrf_pb2_grpc.VrfStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'VrfUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.VrfUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def VrfDelete(self, request, context):
         stub = vrf_pb2_grpc.VrfStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'VrfDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.VrfDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def VrfGet(self, request, context):
         stub = vrf_pb2_grpc.VrfStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'VrfGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.VrfGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

tcp_proxy_pb2 = importlib.import_module('tcp_proxy_pb2')
tcp_proxy_pb2_grpc = importlib.import_module('tcp_proxy_pb2_grpc')
class TcpProxy(tcp_proxy_pb2_grpc.TcpProxyServicer):
    def TcpProxyRuleCreate(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyRuleCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyRuleCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpProxyRuleUpdate(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyRuleUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyRuleUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpProxyRuleDelete(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyRuleDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyRuleDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpProxyRuleGet(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyRuleGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyRuleGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpProxyCbCreate(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyCbCreate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyCbCreate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpProxyCbUpdate(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyCbUpdate')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyCbUpdate(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpProxyCbDelete(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyCbDelete')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyCbDelete(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    def TcpProxyCbGet(self, request, context):
         stub = tcp_proxy_pb2_grpc.TcpProxyStub(channel)
         response, err = config_mgr.CreateConfigFromDol(request, 'TcpProxyCbGet')
         if err:
            #logger.info("Sending DOL message for message type %s\n%s\n"%(type(request), request))
            response = stub.TcpProxyCbGet(request)
            #logger.info("Received HAL response \n%s\n" %(response))
         return response

    pass

def grpc_service_register(proxyServer):
    cpucb_pb2_grpc.add_CpuCbServicer_to_server(CpuCb(), proxyServer)
    crypto_keys_pb2_grpc.add_CryptoKeyServicer_to_server(CryptoKey(), proxyServer)
    acl_pb2_grpc.add_AclServicer_to_server(Acl(), proxyServer)
    barco_rings_pb2_grpc.add_BarcoRingsServicer_to_server(BarcoRings(), proxyServer)
    dos_pb2_grpc.add_DosServicer_to_server(Dos(), proxyServer)
    crypto_apis_pb2_grpc.add_CryptoApisServicer_to_server(CryptoApis(), proxyServer)
    descriptor_aol_pb2_grpc.add_DescrAolServicer_to_server(DescrAol(), proxyServer)
    debug_pb2_grpc.add_DebugServicer_to_server(Debug(), proxyServer)
    endpoint_pb2_grpc.add_EndpointServicer_to_server(Endpoint(), proxyServer)
    event_pb2_grpc.add_EventServicer_to_server(Event(), proxyServer)
    ipseccb_pb2_grpc.add_IpsecCbServicer_to_server(IpsecCb(), proxyServer)
    gft_pb2_grpc.add_GftServicer_to_server(Gft(), proxyServer)
    l4lb_pb2_grpc.add_L4LbServicer_to_server(L4Lb(), proxyServer)
    multicast_pb2_grpc.add_MulticastServicer_to_server(Multicast(), proxyServer)
    l2segment_pb2_grpc.add_L2SegmentServicer_to_server(L2Segment(), proxyServer)
    nat_pb2_grpc.add_NatServicer_to_server(Nat(), proxyServer)
    nic_pb2_grpc.add_NicServicer_to_server(Nic(), proxyServer)
    ipsec_pb2_grpc.add_IpsecServicer_to_server(Ipsec(), proxyServer)
    internal_pb2_grpc.add_InternalServicer_to_server(Internal(), proxyServer)
    internal_pb2_grpc.add_SoftwarePhvServicer_to_server(SoftwarePhv(), proxyServer)
    interface_pb2_grpc.add_InterfaceServicer_to_server(Interface(), proxyServer)
    rawrcb_pb2_grpc.add_RawrCbServicer_to_server(RawrCb(), proxyServer)
    nw_pb2_grpc.add_NetworkServicer_to_server(Network(), proxyServer)
    rawccb_pb2_grpc.add_RawcCbServicer_to_server(RawcCb(), proxyServer)
    port_pb2_grpc.add_PortServicer_to_server(Port(), proxyServer)
    proxyrcb_pb2_grpc.add_ProxyrCbServicer_to_server(ProxyrCb(), proxyServer)
    rdma_pb2_grpc.add_RdmaServicer_to_server(Rdma(), proxyServer)
    proxyccb_pb2_grpc.add_ProxycCbServicer_to_server(ProxycCb(), proxyServer)
    table_pb2_grpc.add_TableServicer_to_server(Table(), proxyServer)
    proxy_pb2_grpc.add_ProxyServicer_to_server(Proxy(), proxyServer)
    tls_proxy_cb_pb2_grpc.add_TlsCbServicer_to_server(TlsCb(), proxyServer)
    system_pb2_grpc.add_SystemServicer_to_server(System(), proxyServer)
    qos_pb2_grpc.add_QOSServicer_to_server(QOS(), proxyServer)
    tcp_proxy_cb_pb2_grpc.add_TcpCbServicer_to_server(TcpCb(), proxyServer)
    nwsec_pb2_grpc.add_NwSecurityServicer_to_server(NwSecurity(), proxyServer)
    tls_proxy_cb2_pb2_grpc.add_TlsProxyCbServicer_to_server(TlsProxyCb(), proxyServer)
    wring_pb2_grpc.add_WRingServicer_to_server(WRing(), proxyServer)
    session_pb2_grpc.add_SessionServicer_to_server(Session(), proxyServer)
    telemetry_pb2_grpc.add_TelemetryServicer_to_server(Telemetry(), proxyServer)
    vrf_pb2_grpc.add_VrfServicer_to_server(Vrf(), proxyServer)
    tcp_proxy_pb2_grpc.add_TcpProxyServicer_to_server(TcpProxy(), proxyServer)

grpc_service_register(proxyServer)
