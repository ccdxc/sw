#! /usr/bin/python3
import pdb

import device_pb2 as device_pb2
import types_pb2 as types_pb2
import ipaddress
import utils

class DeviceObject():
    def __init__(self, ip, gatewayip, mac, deviceopermode=device_pb2.DEVICE_OPER_MODE_HOST ):
        self.ip = ip
        self.gatewayip = gatewayip
        self.mac = mac
        self.deviceopermode = deviceopermode
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = device_pb2.DeviceRequest()
        grpcmsg.Request.IPAddr.Af = types_pb2.IP_AF_INET
        grpcmsg.Request.IPAddr.V4Addr = int(self.ip)
        if self.gatewayip:
            grpcmsg.Request.GatewayIP.Af = types_pb2.IP_AF_INET
            grpcmsg.Request.GatewayIP.V4Addr = int(self.gatewayip)
        grpcmsg.Request.DevOperMode = self.deviceopermode
        if self.mac:
            grpcmsg.Request.MACAddr = utils.getmac2num(self.mac)
        return grpcmsg


