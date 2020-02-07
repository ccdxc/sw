#! /usr/bin/python3
import pdb

import mirror_pb2 as mirror_pb2
import types_pb2 as types_pb2
import ipaddress
import utils

class MirrorObject():
    def __init__(self, id, len = 64, type = 0, frontportid = None, vlanid = None,  vpcid = None, dstip = None, srcip = None, dscp = None, spanid = None):
        self.id          = id
        self.type        = type
        self.len         = len
        self.frontportid = frontportid
        self.vlanid      = vlanid
        self.vpcid       = vpcid
        self.dstip       = dstip
        self.srcip       = srcip
        self.dscp        = dscp
        self.spanid      = spanid
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = mirror_pb2.MirrorSessionRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.id
        spec.SnapLen = self.len
        spec.RspanSpec.InterfaceId = self.frontportid
        spec.RspanSpec.Encap.type = types_pb2.ENCAP_TYPE_DOT1Q
        spec.RspanSpec.Encap.value.VlanId = self.vlanid
        return grpcmsg
