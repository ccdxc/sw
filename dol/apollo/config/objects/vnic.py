#! /usr/bin/python3
import pdb
import sys

from infra.common.logging import logger

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.mirror as mirror
import apollo.config.objects.meter as meter
from apollo.config.objects.policy import client as PolicyClient
import apollo.config.utils as utils

import vnic_pb2 as vnic_pb2
import types_pb2 as types_pb2

class VnicStatusObject(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.VNIC)
        return

    def Update(self, status):
        self.HwId = status.HwId
        return

class VnicObject(base.ConfigObjectBase):
    def __init__(self, parent, spec, rxmirror, txmirror):
        super().__init__()
        self.SetBaseClassAttr()
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
        self.V4MeterId = meter.client.GetV4MeterId(parent.VPC.VPCId)
        self.V6MeterId = meter.client.GetV6MeterId(parent.VPC.VPCId)
        self.HostIf = parent.HostIf
        self.IngV4SecurityPolicyIds = []
        self.IngV6SecurityPolicyIds = []
        self.EgV4SecurityPolicyIds = []
        self.EgV6SecurityPolicyIds = []
        self.Status = VnicStatusObject()
        ################# PRIVATE ATTRIBUTES OF VNIC OBJECT #####################
        self.__attachpolicy = getattr(spec, 'policy', False) and utils.IsVnicPolicySupported()
        # get num of policies [0-5] in rrob order if needed
        self.__numpolicy = resmgr.NumVnicPolicyAllocator.rrnext() if self.__attachpolicy else 0
        self.dot1Qenabled = getattr(spec, 'tagged', True)
        self._derive_oper_info()
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

    def Show(self):
        logger.info("VNIC object:", self)
        logger.info("- %s" % repr(self))
        logger.info("- Vlan: %s %d|Mpls:%d|Vxlan:%d|MAC:%s|SourceGuard:%s"\
             % (self.dot1Qenabled, self.VlanId, self.MplsSlot, self.Vnid, self.MACAddr, str(self.SourceGuard)))
        logger.info("- RxMirror:", self.RxMirror)
        logger.info("- TxMirror:", self.TxMirror)
        logger.info("- V4MeterId:%d|V6MeterId:%d" %(self.V4MeterId, self.V6MeterId))
        if self.HostIf:
            logger.info("- HostInterface:", self.HostIf.Ifname)
        if self.__attachpolicy:
            logger.info("- NumSecurityPolicies:", self.__numpolicy)
            logger.info("- Ing V4 Policies:", self.IngV4SecurityPolicyIds)
            logger.info("- Ing V6 Policies:", self.IngV6SecurityPolicyIds)
            logger.info("- Egr V4 Policies:", self.EgV4SecurityPolicyIds)
            logger.info("- Egr V6 Policies:", self.EgV6SecurityPolicyIds)
        return

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.VNIC
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
        if self.HostIf:
            spec.HostIfIndex = utils.LifId2LifIfIndex(self.HostIf.lif.id)
        return

    def ValidateSpec(self, spec):
        if spec.VnicId != self.VnicId:
            return False
        # if spec.SubnetId != self.SUBNET.SubnetId:
        #     return False
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

class VnicObjectClient:
    def __init__(self):
        self.__objs = dict()
        return

    def Objects(self):
        return self.__objs.values()

    def IsValidConfig(self):
        count = len(self.__objs.values())
        if  count > resmgr.MAX_VNIC:
            return False, "VNIC count %d exceeds allowed limit of %d" %\
                          (count, resmgr.MAX_VNIC)
        return True, ""

    def GetVnicObject(self, vnicid):
        return self.__objs.get(vnicid, None)

    def GetGrpcDeleteMessage(self):
        grpcmsg = vnic_pb2.VnicDeleteRequest()
        grpcmsg.VnicId.append(self.VnicId)
        return grpcmsg

    def AssociateObjects(self):
        # generate security policies and associate with vnic
        for vnic in self.Objects():
            vnic.Generate_vnic_security_policies()
        return

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
                self.__objs.update({obj.VnicId: obj})
        return

    def ShowObjects(self):
        for obj in self.Objects():
            obj.Show()
        return

    def CreateObjects(self):
        if len(self.__objs.values()) == 0:
            return
        logger.info("Creating Vnic Objects in agent")
        self.ShowObjects()
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__objs.values()))
        api.client.Create(api.ObjectTypes.VNIC, msgs)
        # Create Local Mapping Objects
        lmapping.client.CreateObjects()
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = vnic_pb2.VnicGetRequest()
        return grpcmsg

    def ReadObjects(self):
        if len(self.__objs.values()) == 0:
            return
        msg = self.GetGrpcReadAllMessage()
        resp = api.client.Get(api.ObjectTypes.VNIC, [msg])
        result = self.ValidateObjects(resp)
        if result is False:
            logger.critical("VNIC object validation failed!!!")
            sys.exit(1)
        return

    def ValidateObjects(self, getResp):
        if utils.IsDryRun(): return True
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("VNIC get request failed for ", obj)
                return False
            for resp in obj.Response:
                spec = resp.Spec
                vnic = self.GetVnicObject(spec.VnicId)
                if not utils.ValidateObject(vnic, resp):
                    logger.error("VNIC validation failed for ", obj)
                    vnic.Show()
                    return False
                # update hw id for this vnic object
                vnic.Status.Update(resp.Status)
        return True

client = VnicObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
