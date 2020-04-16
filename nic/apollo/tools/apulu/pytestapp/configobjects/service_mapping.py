#! /usr/bin/python3
import utils
import re

import service_pb2 as service_pb2
import types_pb2 as types_pb2

class SvcMappingObject():
    def __init__(self, id, vpcid, backend_ip, backend_port, vip, service_port):
        super().__init__()
        self.id    = id
        self.uuid = utils.PdsUuid(self.id)
        self.vpcid = vpcid
        self.backend_ip = backend_ip
        self.backend_port = backend_port
        self.vip = vip
        self.service_port = service_port
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = service_pb2.SvcMappingRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.Key.VPCId = utils.PdsUuid.GetUUIDfromId(self.vpcid)
        spec.Key.BackendIP.Af = types_pb2.IP_AF_INET
        spec.Key.BackendIP.V4Addr = int(self.backend_ip)
        spec.Key.BackendPort = int(self.backend_port)
        spec.IPAddr.Af = types_pb2.IP_AF_INET
        spec.IPAddr.V4Addr = int(self.vip)
        spec.SvcPort = int(self.service_port)
        return grpcmsg

