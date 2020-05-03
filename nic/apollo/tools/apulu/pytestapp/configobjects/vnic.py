#! /usr/bin/python3
import pdb

import vnic_pb2 as vnic_pb2
import types_pb2 as types_pb2
import ipaddress
import utils
import re

class VnicObject():
    def __init__(self, id, subnetid, macaddr, hostifindex, sourceguard=False, fabricencap='VXLAN', fabricencapid=1, rxmirrorid = [], txmirrorid = [], node_uuid=None, primary=False, vlan=None, flow_learn_en=True):
        self.id       = id
        self.uuid     = utils.PdsUuid(self.id)
        self.primary  = primary
        self.macaddr  = macaddr
        self.subnetid = subnetid
        self.fabricencap = fabricencap
        self.fabricencapid = fabricencapid
        self.hostifindex = int(hostifindex, 16)
        if node_uuid:
            self.hostifuuid = utils.PdsUuid(self.hostifindex, node_uuid)
        else:
            self.hostifuuid = utils.PdsUuid(self.hostifindex)
        self.sourceguard = sourceguard
        self.rxmirrorid = rxmirrorid
        self.txmirrorid = txmirrorid
        self.vlan = vlan
        self.flow_learn_en = flow_learn_en
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = vnic_pb2.VnicRequest()
        spec = grpcmsg.Request.add()
        spec.Id = utils.PdsUuid.GetUUIDfromId(self.id)
        spec.Primary = self.primary
        spec.SubnetId = utils.PdsUuid.GetUUIDfromId(self.subnetid)
        spec.MACAddress = utils.getmac2num(self.macaddr)
        spec.SourceGuardEnable = self.sourceguard
        if re.search( 'VXLAN', self.fabricencap, re.I ):
           spec.FabricEncap.type = types_pb2.ENCAP_TYPE_VXLAN
           spec.FabricEncap.value.Vnid = self.fabricencapid
        spec.HostIf = self.hostifuuid.GetUuid()
        if self.rxmirrorid:
            spec.RxMirrorSessionId.extend(self.rxmirrorid)
        if self.txmirrorid:
            spec.TxMirrorSessionId.extend(self.txmirrorid)
        if self.vlan:
            spec.VnicEncap.type = types_pb2.ENCAP_TYPE_DOT1Q
            spec.VnicEncap.value.VlanId = self.vlan
        spec.FlowLearnEn = self.flow_learn_en
        return grpcmsg
