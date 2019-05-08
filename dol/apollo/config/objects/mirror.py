#! /usr/bin/python3
import pdb
import ipaddress

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import mirror_pb2 as mirror_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class MirrorSessionObject(base.ConfigObjectBase):
    def __init__(self, span_type, snap_len, interface, vlan_id, vpcid, dscp, srcip, dstip):
        super().__init__()
        self.Id = next(resmgr.MirrorSessionIdAllocator)
        self.GID("MirrorSession%d"%self.Id)

        ################# PUBLIC ATTRIBUTES OF MIRROR OBJECT #####################
        self.SnapLen = snap_len
        self.SpanType = span_type
        if span_type == 'RSPAN':
            self.Interface = interface
            self.VlanId = vlan_id
        elif span_type == 'ERSPAN':
            self.VPCId = vpcid
            self.SpanID = self.Id
            self.Dscp = dscp
            self.SrcIP = srcip
            self.DstIP = dstip
        else:
            assert(0)
        ################# PRIVATE ATTRIBUTES OF MIRROR OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "MirrorSession%d|SnapLen:%s|SpanType:%s" %\
               (self.Id, self.SnapLen, self.SpanType)

    def GetGrpcCreateMessage(self):
        grpcmsg = mirror_pb2.MirrorSessionRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.Id
        spec.SnapLen = self.SnapLen
        if self.SpanType == 'RSPAN':
            spec.RspanSpec.InterfaceId = self.Interface
            spec.RspanSpec.Encap.type = types_pb2.ENCAP_TYPE_DOT1Q
            spec.RspanSpec.Encap.value.VlanId = self.VlanId
        elif self.SpanType == 'ERSPAN':
            utils.GetRpcIPAddr(self.DstIP, spec.ErspanSpec.DstIP)
            utils.GetRpcIPAddr(self.SrcIP, spec.ErspanSpec.SrcIP)
            spec.ErspanSpec.Dscp = self.Dscp
            spec.ErspanSpec.SpanId = self.SpanID
            spec.ErspanSpec.VPCId = self.VPCId
        else:
            assert(0)
        return grpcmsg

    def Show(self):
        logger.info("Mirror session Object: %s" % self)
        logger.info("- %s" % repr(self))
        if self.SpanType == 'RSPAN':
            logger.info("- InterfaceId:0x%x|vlanId:%d" %\
                        (self.Interface, self.VlanId))
        elif self.SpanType == 'ERSPAN':
            logger.info("- VPCId:%d|DstIP:%s|SrcIP:%s|Dscp:%d|SpanID:%d" %\
                        (self.VPCId, self.DstIP, self.SrcIP, self.Dscp, self.SpanID))
        else:
            assert(0)
        return

class MirrorSessionObjectClient:
    def __init__(self):
        self.__objs = dict()
        return

    def Objects(self):
        return self.__objs.values()

    def GetMirrorObject(self, mirrorid):
        return self.__objs.get(mirrorid, None)

    def GenerateObjects(self, mirrorsessionspec):
        def __add_mirror_session(msspec):
            spantype = msspec.spantype
            snaplen = msspec.snaplen
            if msspec.spantype == "RSPAN":
                interface = msspec.interface
                vlanid = msspec.vlanid
                obj = MirrorSessionObject(spantype, snaplen, interface, vlanid, 0, 0, 0, 0)
            elif msspec.spantype == "ERSPAN":
                vpcid = msspec.vpcid
                dscp = msspec.dscp
                srcip = ipaddress.ip_address(msspec.srcip)
                dstip = ipaddress.ip_address(msspec.dstip)
                obj = MirrorSessionObject(spantype, snaplen, 0, 0, vpcid, dscp, srcip, dstip)
            else:
                assert(0)
            self.__objs.update({obj.Id: obj})

        if not hasattr(mirrorsessionspec, 'mirror'):
            return

        for mirror_session_spec_obj in mirrorsessionspec.mirror:
            __add_mirror_session(mirror_session_spec_obj)
        return

    def CreateObjects(self):
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs.values()))
        api.client.Create(api.ObjectTypes.MIRROR, msgs)
        return

client = MirrorSessionObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
