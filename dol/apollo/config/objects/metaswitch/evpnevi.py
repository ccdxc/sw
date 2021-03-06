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

class EvpnEviObject(base.ConfigObjectBase):
    def __init__(self, node, parent, evpnevispec):
        super().__init__(api.ObjectTypes.BGP_EVPN_EVI, node)
        if hasattr(evpnevispec, 'origin'):
            self.SetOrigin(evpnevispec.origin)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].EvpnEviIdAllocator)
        self.GID("EvpnEvi%d"%self.Id)
        #self.UUID = utils.PdsUuid(self.Id)
        self.UUID = parent.UUID
        parent.AddChild(self)
        ########## PUBLIC ATTRIBUTES OF EVPNEVI CONFIG OBJECT ##############
        self.SubnetId = parent.UUID
        self.AutoRD = getattr(evpnevispec, 'autord', None)
        self.RD = getattr(evpnevispec, 'rd', None)
        self.RTType = getattr(evpnevispec, 'rttype', None)
        self.AutoRT = getattr(evpnevispec, 'autort', None)
        self.RTType = getattr(evpnevispec, 'rttype', None)
        ########## PRIVATE ATTRIBUTES OF EVPEVI CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return f"EvpnEvi:{self.UUID} SubnetId: {self.SubnetId} AutoRd:\
                {self.AutoRD} RD:{self.RD} AutoRt:{self.AutoRT} RTType:\
                {self.RTType}"

    def Show(self):
        logger.info("EvpnEvi config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Key.SubnetId = self.SubnetId.GetUuid()
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.SubnetId = spec.Id
        spec.AutoRD = cp_utils.GetEVPNCfg(self.AutoRD)
        if spec.AutoRD != evpn_pb2.EVPN_CFG_AUTO:
            if self.RD:
                spec.RD = self.RD
        spec.RTType = cp_utils.GetRTType(self.RTType)
        spec.AutoRT = cp_utils.GetEVPNCfg(self.AutoRT)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if spec['id'] != self.GetKey():
            return False
        return True

class EvpnEviObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP_EVPN_EVI, Resmgr.MAX_BGP_EVPN_EVI)
        return

    def IsReadSupported(self):
        return False

    def GenerateObjects(self, node, subnet, subnetspec):
        def __add_evpn_evi_config(evpnevispec):
            parentid = getattr(evpnevispec, "parent-id", 1)
            if subnet.SubnetId == parentid:
                obj = EvpnEviObject(node, subnet, evpnevispec)
                self.Objs[node].update({obj.Id: obj})

        evpneviSpec = getattr(subnetspec, 'evpnevi', None)
        if not evpneviSpec:
            return
        for evpn_evi_spec_obj in evpneviSpec:
            __add_evpn_evi_config(evpn_evi_spec_obj)
        return

client = EvpnEviObjectClient()
