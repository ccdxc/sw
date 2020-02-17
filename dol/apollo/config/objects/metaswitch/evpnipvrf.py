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

class EvpnIpVrfObject(base.ConfigObjectBase):
    def __init__(self, node, evpnipvrfspec):
        super().__init__(api.ObjectTypes.BGP_EVPN_IP_VRF, node)
        self.BatchUnaware = True
        self.Id = next(ResmgrClient[node].EvpnIpVrfIdAllocator)
        self.GID("EvpnIPVrf%d"%self.Id)
        self.UUID = utils.PdsUuid(self.Id)
        ########## PUBLIC ATTRIBUTES OF EVPNEVI CONFIG OBJECT ##############
        self.VPCId = getattr(evpnipvrfspec, 'vpcid', None)
        self.VNI = getattr(evpnipvrfspec, 'vni', 0)
        self.AutoRD = getattr(evpnipvrfspec, 'autord', 0)
        self.RD = getattr(evpnipvrfspec, 'rd', None)
        self.VRFName = getattr(evpnipvrfspec, 'vrfname', None)
        ########## PRIVATE ATTRIBUTES OF EVPEVI CONFIG OBJECT #############
        self.Show()
        return

    def __repr__(self):
        return f"EvpnIpVrf:{self.UUID} VPCId: {self.VPCId} VNI:{self.VNI} \
                AutoRd:{self.AutoRD} RD:{self.RD} VRFName:{self.VRFName}"

    def Show(self):
        logger.info("EvpnIpVrf config Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        if spec.VPCId:
            spec.VPCId = utils.PdsUuid.GetUUIDfromId(self.VPCId)
        if spec.VNI:
            spec.VNI = self.VNI
        if self.AutoRD:
            if self.AutoRD == 'auto':
                spec.AutoRD == evpn_pb2.EVPN_CFG_AUTO
            elif self.AutoRD == 'manual':
                spec.AutoRD == evpn_pb2.EVPN_CFG_MANUAL
            else:
                spec.AutoRD == evpn_pb2.EVPN_CFG_INVALID
        if self.RD:
            spec.RD = self.RD
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

class EvpnIpVrfObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.BGP_EVPN_IP_VRF, Resmgr.MAX_BGP_EVPN_IP_VRF)
        return

    def GenerateObjects(self, node, vpcspec):
        def __add_evpn_ip_vrf_config(evpnipvrfspec):
            obj = EvpnIpVrfObject(node, evpnipvrfspec)
            self.Objs[node].update({obj.Id: obj})
            utils.dump(evpnipvrfspec)
        evpnIpVrfSpec = getattr(vpcspec, 'evpnipvrf', None)
        if not evpnIpVrfSpec:
            return

        for evpn_ip_vrf_spec_obj in evpnIpVrfSpec:
            __add_evpn_ip_vrf_config(evpn_ip_vrf_spec_obj)
        return

client = EvpnIpVrfObjectClient()
