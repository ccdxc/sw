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

class EvpnEviObject(base.ConfigObjectBase):
    def __init__(self, node, evpnevispec):
        super().__init__(api.ObjectTypes.BGP_EVPN_EVI, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].EvpnEviIdAllocator)
        self.GID("EvpnEvi%d"%self.Id)
        self.UUID = utils.PdsUuid(self.Id)
        ########## PUBLIC ATTRIBUTES OF EVPNEVI CONFIG OBJECT ##############
        self.SubnetId = getattr(evpnevispec, 'subnetid', None)
        self.AutoRD = getattr(evpnevispec, 'autord', None)
        self.RD = getattr(evpnevispec, 'rd', None)
        self.AutoRT = getattr(evpnevispec, 'autort', None)
        self.RTType = getattr(evpnevispec, 'rttype', None)
        self.Encap = getattr(evpnevispec, 'encap', None)
        self.EVIId = getattr(evpnevispec, 'eviid', None)
        ########## PRIVATE ATTRIBUTES OF EVPEVI CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return f"EvpnEvi:{self.UUID} SubnetId: {self.SubnetId} AutoRd:\
                {self.AutoRD} RD:{self.RD} AutoRt:{self.AutoRT} RTType:\
                {self.RTType} Encap:{self.Encap} EVIId:{self.EVIId}"

    def Show(self):
        logger.info("EvpnEvi config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        if self.SubnetId:
            spec.SubnetId = utils.PdsUuid.GetUUIDfromId(self.SubnetId)
        if self.AutoRD:
            if self.AutoRD == 'auto':
                spec.AutoRD == evpn_pb2.EVPN_CFG_AUTO
            elif self.AutoRD == 'manual':
                spec.AutoRD == evpn_pb2.EVPN_CFG_MANUAL
            else:
                spec.AutoRD == evpn_pb2.EVPN_CFG_INVALID
        if spec.AutoRD != evpn_pb2.EVPN_CFG_AUTO:
            if self.RD:
                spec.RD = self.RD
        if self.AutoRT:
            if self.AutoRT == 'auto':
                spec.AutoRT == evpn_pb2.EVPN_CFG_AUTO
            elif self.AutoRT == 'manual':
                spec.AutoRT == evpn_pb2.EVPN_CFG_MANUAL
            else:
                spec.AutoRT == evpn_pb2.EVPN_CFG_INVALID
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
        if self.Encap:
            if self.Encap == 'mpls':
                spec.Encap = evpn_pb2.EVPN_ENCAP_MPLS
            elif self.Encap == 'vxlan':
                spec.Encap = evpn_pb2.EVPN_ENCAP_VXLAN
            else:
                spec.Encap = evpn_pb2.EVPN_ENCAP_INVALID
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

class EvpnEviObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP_EVPN_EVI, Resmgr.MAX_BGP_EVPN_EVI)
        return

    def GenerateObjects(self, node, subnetspec):
        def __add_evpn_evi_config(evpnevispec):
            obj = EvpnEviObject(node, evpnevispec)
            self.Objs[node].update({obj.Id: obj})
        evpneviSpec = getattr(subnetspec, 'evpnevi', None)
        if not evpneviSpec:
            return

        for evpn_evi_spec_obj in evpneviSpec:
            __add_evpn_evi_config(evpn_evi_spec_obj)
        return

client = EvpnEviObjectClient()
