#! /usr/bin/python3
import pdb

import api
import interface_pb2 as interface_pb2
import types_pb2 as types_pb2
import utils

class NodeObject():
    def __init__(self):
        api.Init()
        grpcmsg = interface_pb2.LifGetRequest()
        objs = api.client.Retrieve(api.ObjectTypes.LIF, [grpcmsg])
        macaddress = utils.PdsUuid.GetUuidMacString((objs[0].Response[0].Spec.Id))
        self.MacAddress = macaddress
        return

    def GetNodeMac(self):
        return self.MacAddress
