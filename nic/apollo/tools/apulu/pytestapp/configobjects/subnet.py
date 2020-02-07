#! /usr/bin/python3
import pdb
import utils
import re

import subnet_pb2 as subnet_pb2
import types_pb2 as types_pb2

class SubnetObject():
    def __init__(self, id, vpcid, v4prefix, v6prefix, hostifindex, v4virtualrouterip, v6virtualrouterip, virtualroutermac, v4routetableid, v6routetableid, ingv4securitypolicyid, egrv4securitypolicyid, ingv6securitypolicyid, egrv6securitypolicyid, fabricencap='VXLAN', fabricencapid=1, node_uuid=None):
        super().__init__()
        self.id    = id
        self.vpcid = vpcid
        self.uuid = utils.PdsUuid(self.id)
        self.v4prefix = v4prefix
        self.v6prefix = v6prefix
        self.hostifindex = int(hostifindex, 16)
        if node_uuid:
            self.hostifuuid = utils.PdsUuid(self.hostifindex, node_uuid)
        else:
            self.hostifuuid = utils.PdsUuid(self.hostifindex)
        self.v4virtualrouterip = v4virtualrouterip
        self.v6virtualrouterip = v6virtualrouterip
        self.virtualroutermac = virtualroutermac
        self.fabricencap = fabricencap
        self.fabricencapid = fabricencapid
        self.v4routetableid = v4routetableid
        self.v6routetableid = v6routetableid
        self.ingv4securitypolicyid = ingv4securitypolicyid
        self.egrv4securitypolicyid = egrv4securitypolicyid
        self.ingv6securitypolicyid = ingv6securitypolicyid
        self.egrv6securitypolicyid = egrv6securitypolicyid

        return

    def GetGrpcCreateMessage(self):
        grpcmsg = subnet_pb2.SubnetRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.VPCId = utils.PdsUuid.GetUUIDfromId(self.vpcid)
        spec.V4Prefix.Len = self.v4prefix.prefixlen
        #print(spec.V4Prefix.Addr)
        spec.V4Prefix.Addr = int( self.v4prefix.network_address)
        #print(spec.V4Prefix.Addr)

        if re.search( 'VXLAN', self.fabricencap, re.I ):
           spec.FabricEncap.type = types_pb2.ENCAP_TYPE_VXLAN
           spec.FabricEncap.value.Vnid = self.fabricencapid
        else:
           print('In else part')
           #spec.FabricEncap.type = types_pb2.ENCAP_TYPE_NONE

        spec.IPv4VirtualRouterIP = int(self.v4virtualrouterip)
        spec.VirtualRouterMac = utils.getmac2num(self.virtualroutermac)
        spec.V4RouteTableId = utils.PdsUuid.GetUUIDfromId(self.v4routetableid)
        #spec.IngV4SecurityPolicyId = self.ingv4securitypolicyid
        #spec.IngV6SecurityPolicyId = self.egrv4securitypolicyid
        #spec.EgV4SecurityPolicyId = self.egrv4securitypolicyid
        #spec.EgV6SecurityPolicyId = self.egrv6securitypolicyid
        spec.HostIf = self.hostifuuid.GetUuid()
        return grpcmsg

