#! /usr/bin/python3
import pdb

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.mirror as mirror
import apollo.config.utils as utils
import vnic_pb2 as vnic_pb2
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class VnicObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, rxmirror, txmirror):
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
        self.RxMirror = rxmirror
        self.TxMirror = txmirror
        self._derive_oper_info()

        ################# PRIVATE ATTRIBUTES OF VNIC OBJECT #####################
        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate MAPPING configuration
        lmapping.client.GenerateObjects(self, spec)

        return

    def _derive_oper_info(self):
        self.RxMirrorObjs = dict()
        for rxmirrorid in self.RxMirror:
            rxmirrorobj = mirror.client.GetMirrorObject(rxmirrorid)
            self.RxMirrorObjs.update({rxmirrorid: rxmirrorobj})

        self.TxMirrorObjs = dict()
        for txmirrorid in self.TxMirror:
            txmirrorobj = mirror.client.GetMirrorObject(txmirrorid)
            self.TxMirrorObjs.update({txmirrorid: txmirrorobj})
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
        spec.VnicEncap.type = types_pb2.ENCAP_TYPE_DOT1Q
        spec.VnicEncap.value.VlanId = self.VlanId
        spec.MACAddress = self.MACAddr.getnum()
        spec.ResourcePoolId = 0 # TODO, Need to allocate and use
        spec.SourceGuardEnable = self.SourceGuard
        utils.GetRpcEncap(self.MplsSlot, self.Vnid, spec.FabricEncap)
        for rxmirror in self.RxMirror:
            spec.RxMirrorSessionId.append(int(rxmirror))
        for txmirror in self.TxMirror:
            spec.TxMirrorSessionId.append(int(txmirror))
        return grpcmsg

    def Show(self):
        logger.info("VNIC object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Vlan:%d|Mpls:%d|Vxlan:%d|MAC:%s|SourceGuard:%s"\
             % (self.VlanId, self.MplsSlot, self.Vnid, self.MACAddr, str(self.SourceGuard)))
        logger.info("- RxMirror:", self.RxMirror)
        logger.info("- TxMirror:", self.TxMirror)
        return

    def SetupTestcaseConfig(self, obj):
        return


class VnicObjectClient:
    def __init__(self):
        self.__objs = []
        return

    def Objects(self):
        return self.__objs

    def IsValidConfig(self):
        count = len(self.__objs)
        if  count > resmgr.MAX_VNIC:
            return False, "VNIC count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_VNIC)
        return True, ""

    def GenerateObjects(self, parent, subnet_spec_obj):
        if getattr(subnet_spec_obj, 'vnic', None) == None:
            return
        def __get_rxmirror(vnicspec):
            ms = []
            if hasattr(vnicspec, 'rxmirror'):
                if vnicspec.rxmirror is None:
                    return ms;
                for rxmirror in vnicspec.rxmirror:
                    ms.append(rxmirror.msid) 
            return ms

        def __get_txmirror(vnicspec):
            ms = []
            if hasattr(vnicspec, 'txmirror'):
                if vnicspec.txmirror is None:
                    return ms;
                for txmirror in vnicspec.txmirror:
                    ms.append(txmirror.msid) 
            return ms

        for vnic_spec_obj in subnet_spec_obj.vnic:
            for c in range(vnic_spec_obj.count):
                # Alternate src dst validations
                rxmirror = __get_rxmirror(vnic_spec_obj)
                txmirror = __get_txmirror(vnic_spec_obj)
                obj = VnicObject(parent, vnic_spec_obj, rxmirror, txmirror)
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
