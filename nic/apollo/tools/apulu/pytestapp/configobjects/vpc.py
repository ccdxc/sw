#! /usr/bin/python3
import pdb

import vpc_pb2 as vpc_pb2
import types_pb2 as types_pb2
import ipaddress
import utils
import re

class VpcObject():
    def __init__(self, id, type = vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=None, rmac=None, v4routetableid=None):
        #print(encapvalue)
        self.id       = id
        self.uuid     = utils.PdsUuid(self.id)
        self.type     = type
        self.encaptype = encaptype
        self.encapvalue = encapvalue
        self.v4routetableid = v4routetableid
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = vpc_pb2.VPCRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.Type = self.type
        spec.FabricEncap.type = self.encaptype
        if self.encaptype == types_pb2.ENCAP_TYPE_VXLAN:
           spec.FabricEncap.value.Vnid = self.encapvalue
        if self.v4routetableid:
            spec.V4RouteTableId = utils.PdsUuid.GetUUIDfromId(self.v4routetableid)
        return grpcmsg
