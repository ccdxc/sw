#! /usr/bin/python3
import pdb
import utils
import re

import subnet_pb2 as subnet_pb2
import types_pb2 as types_pb2

class SubnetObject():
    def __init__(self, id, vpcid, v4prefix, hostifindex, v4virtualrouterip, virtualroutermac, v4routetableid, fabricencap='VXLAN', fabricencapid=1, node_uuid=None, dhcp_policy_id=None):
        super().__init__()
        self.id    = id
        self.vpcid = vpcid
        self.dhcp_policy_id = dhcp_policy_id
        self.uuid = utils.PdsUuid(self.id)
        self.v4prefix = v4prefix
        self.hostifindex = int(hostifindex, 16)
        if node_uuid:
            self.hostifuuid = utils.PdsUuid(self.hostifindex, node_uuid)
        else:
            self.hostifuuid = utils.PdsUuid(self.hostifindex)
        self.v4virtualrouterip = v4virtualrouterip
        self.virtualroutermac = virtualroutermac
        self.fabricencap = fabricencap
        self.fabricencapid = fabricencapid
        self.v4routetableid = v4routetableid
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = subnet_pb2.SubnetRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.VPCId = utils.PdsUuid.GetUUIDfromId(self.vpcid)
        if self.dhcp_policy_id:
            spec.DHCPPolicyId.append(utils.PdsUuid.GetUUIDfromId(self.dhcp_policy_id))
        spec.V4Prefix.Len = self.v4prefix.prefixlen
        spec.V4Prefix.Addr = int( self.v4prefix.network_address)

        if re.search( 'VXLAN', self.fabricencap, re.I ):
           spec.FabricEncap.type = types_pb2.ENCAP_TYPE_VXLAN
           spec.FabricEncap.value.Vnid = self.fabricencapid

        spec.IPv4VirtualRouterIP = int(self.v4virtualrouterip)
        spec.VirtualRouterMac = utils.getmac2num(self.virtualroutermac)
        if self.v4routetableid:
            spec.V4RouteTableId = utils.PdsUuid.GetUUIDfromId(self.v4routetableid)
        spec.HostIf = self.hostifuuid.GetUuid()
        return grpcmsg

