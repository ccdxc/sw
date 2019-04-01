#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.lmapping as lmapping
import apollo.config.utils as utils
import vnic_pb2 as vnic_pb2
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class VnicObject(base.ConfigObjectBase):
    def __init__(self, parent, spec):
        super().__init__()
        ################# PUBLIC ATTRIBUTES OF VNIC OBJECT #####################
        self.VnicId = next(resmgr.VnicIdAllocator)
        self.GID('Vnic%d'%self.VnicId)
        self.SUBNET = parent
        self.MACAddr =  resmgr.VnicMacAllocator.get()
        self.VlanId = next(resmgr.VnicVlanIdAllocator)
        self.MplsSlot = next(resmgr.VnicMplsSlotIdAllocator)
        self.Vnid = next(resmgr.VxlanIdAllocator)
        self.SourceGuard = False
        c = getattr(spec, 'srcguard', None)
        if c != None:
            self.SourceGuard = c

        ################# PRIVATE ATTRIBUTES OF VNIC OBJECT #####################
        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate MAPPING configuration
        lmapping.client.GenerateObjects(self, spec)

        return

    def __repr__(self):
        return "VnicID:%d|SubnetID:%d|VPCId:%d" %\
               (self.VnicId, self.SUBNET.SubnetId, self.SUBNET.VPC.VPCId)

    def GetGrpcCreateMessage(self):
        grpcmsg = vnic_pb2.VnicRequest()
        spec = grpcmsg.Request.add()
        spec.VnicId = self.VnicId
        spec.SubnetId = self.SUBNET.SubnetId
        spec.VPCId = self.SUBNET.VPC.VPCId
        spec.WireVLAN = self.VlanId
        spec.MACAddress = self.MACAddr.getnum()
        spec.ResourcePoolId = 0 # TODO, Need to allocate and use
        spec.SourceGuardEnable = self.SourceGuard
        utils.GetRpcEncap(self.MplsSlot, self.Vnid, spec.Encap)
        return grpcmsg

    def Show(self):
        logger.info("VNIC object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Vlan:%d|Mpls:%d|Vxlan:%d|MAC:%s|SourceGuard:%s"\
             % (self.VlanId, self.MplsSlot, self.Vnid, self.MACAddr, str(self.SourceGuard)))
        return

    def SetupTestcaseConfig(self, obj):
        return


class VnicObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def GenerateObjects(self, parent, subnet_spec_obj):
        if getattr(subnet_spec_obj, 'vnic', None) == None:
            return
        for vnic_spec_obj in subnet_spec_obj.vnic:
            for c in range(vnic_spec_obj.count):
                # Alternate src dst validations
                obj = VnicObject(parent, vnic_spec_obj)
                self.__objs.append(obj)
        return

    def CreateObjects(self):
        if len(self.__objs) == 0:
            return
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs))
        api.client.Create(api.ObjectTypes.VNIC, msgs)
        # Create Local Mapping Objects
        lmapping.client.CreateObjects()
        return

client = VnicObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
