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

class EvpnEviRtObject(base.ConfigObjectBase):
    def __init__(self, node, evpnevirtspec):
        super().__init__(api.ObjectTypes.BGP_EVPN_EVI_RT, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].EvpnEviRtIdAllocator)
        self.GID("EvpnEvi%d"%self.Id)
        self.UUID = utils.PdsUuid(self.Id)
        ########## PUBLIC ATTRIBUTES OF EVPNEVI CONFIG OBJECT ##############
        self.SubnetId = getattr(evpnevirtspec, 'subnetid', None)
        self.RT = getattr(evpnevirtspec, 'rt', None)
        self.RTType = getattr(evpnevirtspec, 'rttype', None)
        self.EVIId = getattr(evpnevirtspec, 'eviid', None)
        ########## PRIVATE ATTRIBUTES OF EVPEVI CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return f"EvpnEviRt:{self.UUID} SubnetId: {self.SubnetId} RT:\
                {self.RT} RTType:{self.RTType} EVIId:{self.EVIId}"

    def Show(self):
        logger.info("EvpnEviRt config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        if self.SubnetId:
            spec.subnetId = utils.PdsUuid.GetUUIDfromId(self.SubnetId)
        if self.RT:
            spec.RT = bytes(self.RT, 'utf-8')
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
        if self.EVIId:
            spec.EVIId = self.EVIId
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if spec['id'] != self.GetKey():
            return False
        return True

class EvpnEviRtObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP_EVPN_EVI_RT, Resmgr.MAX_BGP_EVPN_EVI_RT)
        return

    def GenerateObjects(self, node, subnetspec):
        def __add_evpn_evi_rt_config(evpnevirtspec):
            obj = EvpnEviRtObject(node, evpnevirtspec)
            self.Objs[node].update({obj.Id: obj})
        evpnEviRtSpec = getattr(subnetspec, 'evpnevirt', None)
        if not evpnEviRtSpec:
            return

        for evpn_evi_rt_spec_obj in evpnEviRtSpec:
            __add_evpn_evi_rt_config(evpn_evi_rt_spec_obj)
        return

client = EvpnEviRtObjectClient()
