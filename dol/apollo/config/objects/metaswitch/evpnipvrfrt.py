#! /usr/bin/python3
import pdb
import ipaddress

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base

import evpn_pb2 as evpn_pb2

class EvpnIpVrfRtObject(base.ConfigObjectBase):
    def __init__(self, node, evpnipvrfrtspec):
        super().__init__(api.ObjectTypes.BGP_EVPN_IP_VRF_RT, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].EvpnIpVrfIdAllocator)
        self.GID("EvpnIPVrfRt%d"%self.Id)
        self.UUID = utils.PdsUuid(self.Id)
        ########## PUBLIC ATTRIBUTES OF EVPNEVI CONFIG OBJECT ##############
        self.VPCId = getattr(evpnipvrfrtspec, 'vpcid', None)
        self.RT = getattr(evpnipvrfrtspec, 'rt', None)
        self.RTType = getattr(evpnipvrfrtspec, 'rttype', 0)
        self.VRFName = getattr(evpnipvrfrtspec, 'vrfname', None)
        ########## PRIVATE ATTRIBUTES OF EVPEVI CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return f"EvpnIPVrfRt:{self.UUID} VPCId: {self.VPCId} RT:{self.RT} \
                 RTType:{self.RTType} VRFName:{self.VRFName}"

    def Show(self):
        logger.info("EvpnIpVrfRt config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        if self.VPCId:
            spec.VPCId = utils.PdsUuid.GetUUIDfromId(self.VPCId)
        #if self.RT:
            #spec.RT = self.RT
        if self.RTType:
            if self.RTType == 'import':
                spec.RTType = evpn_pb2.EVPN_RT_IMPORT
            elif self.RTType == 'export':
                spec.RTType = evpn_pb2.EVPN_RT_EXPORT
            elif self.RTType == 'import_export':
                spec.RTType = evpn_pb2.EVPN_RT_IMPORT_EXPORT
            elif self.RTType == 'none':
                spec.RTType = evpn_pb2.EVPN_RT_NONE
            else:
                spec.RTType = evpn_pb2.EVPN_RT_INVALID
        if self.VRFName:
            spec.VRFName = self.VRFName
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

    def GetDhcpRelayObject(self, node):
        return self.GetObjectByKey(node, 1)

    def GenerateObjects(self, node, vpcspec):
        def __add_evpn_ip_vrf_rt_config(evpnipvrfrtspec):
            obj = EvpnIpVrfRtObject(node, evpnipvrfrtspec)
            self.Objs[node].update({obj.Id: obj})
            utils.dump(evpnipvrfrtspec)
        evpnIpVrfRtSpec = getattr(vpcspec, 'evpnipvrfrt', None)
        if not evpnIpVrfRtSpec:
            return

        for evpn_ip_vrf_rt_spec_obj in evpnIpVrfRtSpec:
            __add_evpn_ip_vrf_rt_config(evpn_ip_vrf_rt_spec_obj)
        return

client = EvpnIpVrfRtObjectClient()
