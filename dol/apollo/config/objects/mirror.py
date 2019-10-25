#! /usr/bin/python3
import pdb
import ipaddress

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base

import mirror_pb2 as mirror_pb2
import types_pb2 as types_pb2


class MirrorSessionObject(base.ConfigObjectBase):
    def __init__(self, span_type, snap_len, interface, vlan_id, vpcid, dscp, srcip, dstip, tunnel_id):
        super().__init__()
        self.SetBaseClassAttr()
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
            self.TunnelId = tunnel_id
        else:
            assert(0)
        ################# PRIVATE ATTRIBUTES OF MIRROR OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "MirrorSession%d|SnapLen:%s|SpanType:%s" %\
               (self.Id, self.SnapLen, self.SpanType)

    def Show(self):
        logger.info("Mirror session Object: %s" % self)
        logger.info("- %s" % repr(self))
        if self.SpanType == 'RSPAN':
            logger.info("- InterfaceId:0x%x|vlanId:%d" %\
                        (self.Interface, self.VlanId))
        elif self.SpanType == 'ERSPAN':
            logger.info("- VPCId:%d|TunnelId:%d|DstIP:%s|SrcIP:%s|Dscp:%d|SpanID:%d" %\
                        (self.VPCId, self.TunnelId, self.DstIP, self.SrcIP, self.Dscp, self.SpanID))
        else:
            assert(0)
        return

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.MIRROR
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.Id)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.Id
        spec.SnapLen = self.SnapLen
        if self.SpanType == 'RSPAN':
            spec.RspanSpec.InterfaceId = self.Interface
            spec.RspanSpec.Encap.type = types_pb2.ENCAP_TYPE_DOT1Q
            spec.RspanSpec.Encap.value.VlanId = self.VlanId
        elif self.SpanType == 'ERSPAN':
            spec.ErspanSpec.TunnelId = self.TunnelId
            utils.GetRpcIPAddr(self.SrcIP, spec.ErspanSpec.SrcIP)
            spec.ErspanSpec.Dscp = self.Dscp
            spec.ErspanSpec.SpanId = self.SpanID
            spec.ErspanSpec.VPCId = self.VPCId
        else:
            assert(0)
        return


class MirrorSessionObjectClient:
    def __init__(self):
        self.__objs = dict()
        return

    def Objects(self):
        return self.__objs.values()

    def IsValidConfig(self):
        count = len(self.__objs.values())
        if  count > resmgr.MAX_MIRROR:
            return False, "Mirror count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_MIRROR)
        return True, ""

    def GetMirrorObject(self, mirrorid):
        return self.__objs.get(mirrorid, None)

    def GenerateObjects(self, mirrorsessionspec):
        def __add_mirror_session(msspec):
            spantype = msspec.spantype
            snaplen = msspec.snaplen
            if msspec.spantype == "RSPAN":
                interface = msspec.interface
                vlanid = msspec.vlanid
                obj = MirrorSessionObject(spantype, snaplen, interface, vlanid, 0, 0, 0, 0, 0)
            elif msspec.spantype == "ERSPAN":
                vpcid = msspec.vpcid
                dscp = msspec.dscp
                tunnel_id = msspec.tunnelid
                srcip = ipaddress.ip_address(msspec.srcip)
                dstip = ipaddress.ip_address(msspec.dstip)
                obj = MirrorSessionObject(spantype, snaplen, 0, 0, vpcid, dscp, srcip, dstip, tunnel_id)
            else:
                assert(0)
            self.__objs.update({obj.Id: obj})

        if not hasattr(mirrorsessionspec, 'mirror'):
            return

        for mirror_session_spec_obj in mirrorsessionspec.mirror:
            __add_mirror_session(mirror_session_spec_obj)
        return

    def CreateObjects(self):
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs.values()))
        api.client.Create(api.ObjectTypes.MIRROR, msgs)
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = mirror_pb2.MirrorSessionGetRequest()
        return grpcmsg

    def ReadObjects(self):
        msg = self.GetGrpcReadAllMessage()
        api.client.Get(api.ObjectTypes.MIRROR, [msg])
        return

client = MirrorSessionObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
