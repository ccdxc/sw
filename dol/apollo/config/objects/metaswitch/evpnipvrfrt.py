#! /usr/bin/python3
import pdb
import ipaddress

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base

import apollo.config.objects.metaswitch.cp_utils as cp_utils
import evpn_pb2 as evpn_pb2

class EvpnIpVrfRtObject(base.ConfigObjectBase):
    def __init__(self, node, parent, evpnipvrfrtspec):
        super().__init__(api.ObjectTypes.BGP_EVPN_IP_VRF_RT, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].EvpnIpVrfIdAllocator)
        self.GID("EvpnIPVrfRt%d"%self.Id)
        self.UUID = parent.UUID
        parent.AddChild(self)
        ########## PUBLIC ATTRIBUTES OF EVPNEVI CONFIG OBJECT ##############
        self.VPCId = parent.UUID
        self.RT = getattr(evpnipvrfrtspec, 'rt', None)
        self.RTType = getattr(evpnipvrfrtspec, 'rttype', 0)
        ########## PRIVATE ATTRIBUTES OF EVPEVI CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return f"EvpnIPVrfRt:{self.UUID} VPCId: {self.VPCId} RT:{self.RT} \
                 RTType:{self.RTType}"

    def Show(self):
        logger.info("EvpnIpVrfRt config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Key.VPCId = self.VPCId.GetUuid()
        spec.Key.RT = cp_utils.GetRT(self.RT)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.VPCId = spec.Id
        spec.RT = cp_utils.GetRT(self.RT)
        spec.RTType = cp_utils.GetRTType(self.RTType)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if spec['id'] != self.GetKey():
            return False
        return True

class EvpnIpVrfRtObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP_EVPN_IP_VRF_RT,\
                         Resmgr.MAX_BGP_EVPN_IP_VRF_RT)
        return

    def GetEvpnIpVrfRtObject(self, node, id):
        return self.GetObjectByKey(node, id)

    def GenerateObjects(self, node, vpc, vpcspec):
        def __add_evpn_ip_vrf_rt_config(evpnipvrfrtspec):
            parentid = getattr(evpnipvrfrtspec, "parent-id", 1)
            if vpc.VPCId == parentid:
                obj = EvpnIpVrfRtObject(node, vpc, evpnipvrfrtspec)
                self.Objs[node].update({obj.Id: obj})

        evpnIpVrfRtSpec = getattr(vpcspec, 'evpnipvrfrt', None)
        if not evpnIpVrfRtSpec:
            return
        for evpn_ip_vrf_rt_spec_obj in evpnIpVrfRtSpec:
            __add_evpn_ip_vrf_rt_config(evpn_ip_vrf_rt_spec_obj)
        return

client = EvpnIpVrfRtObjectClient()
