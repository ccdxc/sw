#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.objects.base as base
import iota.test.apulu.config.objects.lmapping as lmapping
import iota.test.apulu.config.objects.mirror as mirror
import iota.test.apulu.config.objects.meter as meter
from iota.test.apulu.config.objects.policy import client as PolicyClient
import iota.test.apulu.config.utils as utils

import vnic_pb2 as vnic_pb2
import types_pb2 as types_pb2

class VnicStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.VNIC)
        return

    def Update(self, status):
        self.HwId = status.HwId
        return

class VnicObject(base.ConfigObjectBase):
    def __init__(self, node, parent, spec, rxmirror, txmirror):
        super().__init__(api.ObjectTypes.VNIC)
        ################# PUBLIC ATTRIBUTES OF VNIC OBJECT #####################
        if (hasattr(spec, 'id')):
            self.VnicId = spec.id
        else:
            self.VnicId = next(resmgr.VnicIdAllocator)
        self.GID('Vnic%d'%self.VnicId)
        self.Node = node
        self.SUBNET = parent
        if hasattr(spec, 'vmac'):
            self.MACAddr =  spec.vmac
        else:
            self.MACAddr =  resmgr.VnicMacAllocator.get()
        #self.VlanId = next(resmgr.VnicVlanIdAllocator)
        self.VlanId = 0
        self.MplsSlot = next(resmgr.VnicMplsSlotIdAllocator)
        #self.Vnid = next(resmgr.VxlanIdAllocator)
        self.Vnid = parent.Vnid
        self.SourceGuard = False
        c = getattr(spec, 'srcguard', None)
        if c != None:
            self.SourceGuard = c
        self.RxMirror = rxmirror
        self.TxMirror = txmirror
        self.V4MeterId = meter.client.GetV4MeterId(node, parent.VPC.VPCId)
        self.V6MeterId = meter.client.GetV6MeterId(node, parent.VPC.VPCId)
        self.HostIf = parent.HostIf
        self.HostIfIdx = parent.HostIfIdx
        self.IngV4SecurityPolicyIds = []
        self.IngV6SecurityPolicyIds = []
        self.EgV4SecurityPolicyIds = []
        self.EgV6SecurityPolicyIds = []
        self.Status = VnicStatus()
        ################# PRIVATE ATTRIBUTES OF VNIC OBJECT #####################
        self.__attachpolicy = getattr(spec, 'policy', False) and utils.IsVnicPolicySupported()
        # get num of policies [0-5] in rrob order if needed
        self.__numpolicy = resmgr.NumVnicPolicyAllocator.rrnext() if self.__attachpolicy else 0
        self.dot1Qenabled = getattr(spec, 'tagged', True)
        self._derive_oper_info(node)
        self.Show()

        ############### CHILDREN OBJECT GENERATION
        # Generate MAPPING configuration
        lmapping.client.GenerateObjects(node, self, spec)

        return

    def _derive_oper_info(self, node):
        self.RxMirrorObjs = dict()
        for rxmirrorid in self.RxMirror:
            rxmirrorobj = mirror.client.GetMirrorObject(node, rxmirrorid)
            self.RxMirrorObjs.update({rxmirrorid: rxmirrorobj})

        self.TxMirrorObjs = dict()
        for txmirrorid in self.TxMirror:
            txmirrorobj = mirror.client.GetMirrorObject(node, txmirrorid)
            self.TxMirrorObjs.update({txmirrorid: txmirrorobj})
        return

    def __repr__(self):
        return "VnicID:%d|SubnetID:%d|VPCId:%d" %\
               (self.VnicId, self.SUBNET.SubnetId, self.SUBNET.VPC.VPCId)

    def Show(self):
        logger.info("VNIC object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Vlan: %s %d|Mpls:%d|Vxlan:%d|MAC:%s|SourceGuard:%s"\
             % (self.dot1Qenabled, self.VlanId, self.MplsSlot, self.Vnid, self.MACAddr.get(), str(self.SourceGuard)))
        logger.info("- RxMirror:", self.RxMirror)
        logger.info("- TxMirror:", self.TxMirror)
        logger.info("- V4MeterId:%d|V6MeterId:%d" %(self.V4MeterId, self.V6MeterId))
        if self.HostIf:
            logger.info("- HostInterface:", self.HostIf.Ifname)
        if self.HostIfIdx:
            logger.info("- HostInterfaceId:", self.HostIfIdx)
        if self.__attachpolicy:
            logger.info("- NumSecurityPolicies:", self.__numpolicy)
            logger.info("- Ing V4 Policies:", self.IngV4SecurityPolicyIds)
            logger.info("- Ing V6 Policies:", self.IngV6SecurityPolicyIds)
            logger.info("- Egr V4 Policies:", self.EgV4SecurityPolicyIds)
            logger.info("- Egr V6 Policies:", self.EgV6SecurityPolicyIds)
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.VnicId.append(self.VnicId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.VnicId = self.VnicId
        spec.SubnetId = self.SUBNET.SubnetId
        spec.VPCId = self.SUBNET.VPC.VPCId
        if self.dot1Qenabled:
            spec.VnicEncap.type = types_pb2.ENCAP_TYPE_DOT1Q
            spec.VnicEncap.value.VlanId = self.VlanId
        else:
            spec.VnicEncap.type = types_pb2.ENCAP_TYPE_NONE
        spec.MACAddress = self.MACAddr.getnum()
        spec.SourceGuardEnable = self.SourceGuard
        utils.GetRpcEncap(self.MplsSlot, self.Vnid, spec.FabricEncap)
        for rxmirror in self.RxMirror:
            spec.RxMirrorSessionId.append(int(rxmirror))
        for txmirror in self.TxMirror:
            spec.TxMirrorSessionId.append(int(txmirror))
        spec.V4MeterId = self.V4MeterId
        spec.V6MeterId = self.V6MeterId
        for policyid in self.IngV4SecurityPolicyIds:
            spec.IngV4SecurityPolicyId.append(policyid)
        for policyid in self.IngV6SecurityPolicyIds:
            spec.IngV6SecurityPolicyId.append(policyid)
        for policyid in self.EgV4SecurityPolicyIds:
            spec.EgV4SecurityPolicyId.append(policyid)
        for policyid in self.EgV6SecurityPolicyIds:
            spec.EgV6SecurityPolicyId.append(policyid)
        if self.HostIfIdx:
            spec.HostIfIndex = self.HostIfIdx
        if self.HostIf:
            spec.HostIfIndex = utils.LifId2LifIfIndex(self.HostIf.lif.id)
        return

    def ValidateSpec(self, spec):
        if spec.VnicId != self.VnicId:
            return False
        # if spec.SubnetId != self.SUBNET.SubnetId:
        #     return False
        if Store.IsDeviceEncapTypeMPLS():
            if utils.ValidateTunnelEncap(self.MplsSlot, spec.FabricEncap) is False:
                return False
        else:
            if utils.ValidateTunnelEncap(self.Vnid, spec.FabricEncap) is False:
                return False
        if utils.IsPipelineApulu():
            if self.HostIf:
                if spec.HostIfIndex != utils.LifId2LifIfIndex(self.HostIf.lif.id):
                    return False
        if spec.VPCId != self.SUBNET.VPC.VPCId:
            return False
        if spec.MACAddress != self.MACAddr.getnum():
            return False
        if spec.SourceGuardEnable != self.SourceGuard:
            return False
        if spec.V4MeterId != self.V4MeterId:
            return False
        if spec.V6MeterId != self.V6MeterId:
            return False
        # TODO: validate policyid, policer
        return True

    def ValidateYamlSpec(self, spec):
        if spec['vnicid'] != self.VnicId:
            return False
        if utils.IsPipelineApulu():
            if self.HostIf:
                if spec['hostifindex'] != utils.LifId2LifIfIndex(self.HostIf.lif.id):
                    return False
        if spec['vpcid'] != self.SUBNET.VPC.VPCId:
            return False
        if spec['macaddress'] != self.MACAddr.getnum():
            return False
        if spec['sourceguardenable'] != self.SourceGuard:
            return False
        if spec['v4meterid'] != self.V4MeterId:
            return False
        if spec['v6meterid'] != self.V6MeterId:
            return False
        return True

    def GetStatus(self):
        return self.Status

    def Generate_vnic_security_policies(self):
        if self.__numpolicy == 0:
            return
        numpolicy = self.__numpolicy
        subnetobj = self.SUBNET
        self.IngV4SecurityPolicyIds = PolicyClient.GenerateVnicPolicies(numpolicy, subnetobj, 'ingress')
        self.EgV4SecurityPolicyIds = PolicyClient.GenerateVnicPolicies(numpolicy, subnetobj, 'egress')
        if self.SUBNET.VPC.IsV6Stack():
            self.IngV6SecurityPolicyIds = PolicyClient.GenerateVnicPolicies(numpolicy, subnetobj, 'ingress', True)
            self.EgV6SecurityPolicyIds = PolicyClient.GenerateVnicPolicies(numpolicy, subnetobj, 'egress', True)
        return

    def IsEncapTypeVLAN(self):
        return self.dot1Qenabled

class VnicObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.VNIC, resmgr.MAX_VNIC)
        return

    def GetVnicObject(self, node, vnicid):
        return self.GetObjectByKey(node, vnicid)

    def GetKeyfromSpec(self, spec, yaml=False):
        if yaml: return spec['vnicid']
        return spec.VnicId

    def AssociateObjects(self, node):
        # generate security policies and associate with vnic
        for vnic in self.Objects(node):
            vnic.Generate_vnic_security_policies()
        return

    def GenerateObjects(self, node, parent, subnet_spec_obj):
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
                obj = VnicObject(node, parent, vnic_spec_obj, rxmirror, txmirror)
                self.Objs[node].update({obj.VnicId: obj})
        return

    def CreateObjects(self, node):
        super().CreateObjects(node)
        # Create Local Mapping Objects
        lmapping.client.CreateObjects(node)
        return

client = VnicObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
