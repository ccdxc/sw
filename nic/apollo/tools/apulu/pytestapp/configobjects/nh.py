#! /usr/bin/python3
import pdb

import nh_pb2 as nh_pb2
import types_pb2 as types_pb2
import ipaddress
import utils
import re


class UnderlayNhObject():
    def __init__(self, l3intfid, underlaymac ):
        self.l3intfid = l3intfid
        self.underlaymac = underlaymac
        return


class IPNhObject():
    def __init__(self, vpcid, nhip, vlanid, macaddr ):
        self.vpcid     = vpcid
        self.nhip      = nhip
        self.vlanid    = vlanid
        self.macaddr   = macaddr
        return


class NexthopObject():
    def __init__(self, id, type, l3intfid, underlaymac, vpcid=None, nhip=None, vlanid=None, macaddr=None ):
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.type = type
        self.l3intfid = l3intfid
        self.underlaymac = underlaymac
        if re.search( 'underlay', type, re.I ):
            self.nhinfo = UnderlayNhObject( self.l3intfid, self.underlaymac )
        elif re.search( 'ip', type, re.I ):
            self.nhinfo = IPNhObject( vpcid, nhip, vlanid)
        return

    def GetGrpcCreateMessage(self,):
        grpcmsg = nh_pb2.NexthopRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        if re.search( 'underlay', self.type, re.I ):
            underlaynhobj = UnderlayNhObject( self.l3intfid, self.underlaymac )
            #spec.UnderlayNhInfo.L3InterfaceId = underlaynhobj.l3intfid.id 
            spec.UnderlayNhInfo.L3Interface = utils.PdsUuid.GetUUIDfromId(underlaynhobj.l3intfid)
            spec.UnderlayNhInfo.UnderlayMAC = utils.getmac2num(underlaynhobj.underlaymac,reorder=False )
        elif re.search( 'ip', self.type, re.I):
            ipnhobj =  IPNhObject( self.vpcid, self.nhip, self.vlanid, self.macaddr )
            spec.IPNhInfo.VPCId = utils.PdsUuid.GetUUIDfromId(self.ipnhobj.vpcid)
            spec.IPNhInfo.IP = self.ipnhobj.nhip
            spec.IPNhInfo.Vlan = self.vlanid
            spec.IPNhInfo.Mac = utils.getmac2num(self.ipnhobj.macaddr, reorder=False )
        return grpcmsg
