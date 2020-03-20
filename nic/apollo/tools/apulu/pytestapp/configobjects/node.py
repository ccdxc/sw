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

        # save node uuid
        macaddress = utils.PdsUuid.GetUuidMacString((objs[0].Response[0].Spec.Id))
        self.MacAddress = macaddress

        # save dscv0 mac
        for resp in objs[0].Response:
            print(resp.Spec.Type)
            if resp.Spec.Type == types_pb2.LIF_TYPE_VENDOR_INBAND:
                self.VcnIntfMacAddress = utils.getnum2mac(resp.Spec.MacAddress)
        return

    def GetNodeMac(self):
        return self.MacAddress

    def GetVcnIntfMac(self):
        return self.VcnIntfMacAddress
