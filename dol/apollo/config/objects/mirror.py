#! /usr/bin/python3
import enum
import pdb
import ipaddress
import sys

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import mirror_pb2 as mirror_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class MirrorSessionObject(base.ConfigObjectBase):
    def __init__(self, span_type, snap_len, interface, vlan_id):
        super().__init__()
        self.Id = next(resmgr.MirrorSessionIdAllocator)
        self.GID("MirrorSession%d"%self.Id)

        ################# PUBLIC ATTRIBUTES OF MIRROR OBJECT #####################
        self.SnapLen = snap_len
        self.SpanType = span_type
        if span_type == 'rspan':
            self.Interface = interface
            self.VlanId = vlan_id

        ################# PRIVATE ATTRIBUTES OF MIRROR OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "MirrorSession%d|SnapLen:%s|RspanSpecEnabled:%s|InterfaceId:0x%x|vlanId:%d" %\
               (self.Id,self.SnapLen, self.SpanType, self.Interface, self.VlanId)

    def GetGrpcCreateMessage(self):
        grpcmsg = mirror_pb2.MirrorSessionRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.Id
        spec.SnapLen = self.SnapLen
        if self.SpanType == 'rspan':
            spec.RspanSpec.InterfaceId = self.Interface
            spec.RspanSpec.Encap.type = types_pb2.ENCAP_TYPE_DOT1Q
            spec.RspanSpec.Encap.value.VlanId = self.VlanId
        else:
            assert(0)
        return grpcmsg

    def Show(self):
        logger.info("Mirror session Object: %s" % self)
        logger.info("- %s" % repr(self))
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
            if msspec.spantype == "rspan":
                interface = msspec.interface
                vlanid = msspec.vlanid
            obj = MirrorSessionObject(spantype, snaplen, interface, vlanid)
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
