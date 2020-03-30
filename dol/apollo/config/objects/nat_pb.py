#! /usr/bin/python3

import pdb

from collections import defaultdict

from infra.common.logging import logger

import apollo.config.objects.base as base
from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo

from apollo.config.store import EzAccessStore

import service_pb2 as service_pb2
import types_pb2 as types_pb2
import nat_pb2 as nat_pb2

class NatPbStats:
    def __init__(self):
        self.InUseCount = 0
        self.SessionCount = 0

    def Add(self, stats):
        if not stats:
            return
        self.InUseCount += stats.InUseCount
        self.SessionCount += stats.SessionCount

class NatPbObject(base.ConfigObjectBase):
    def __init__(self, node, parent, prefix, port_lo, port_hi, proto, addr_type):
        super().__init__(api.ObjectTypes.NAT, node)
        self.Id = next(ResmgrClient[node].NatPoolIdAllocator)
        self.GID('NatPortBlock%d'%self.Id)
        self.UUID = utils.PdsUuid(self.Id, self.ObjType)
        self.VPC = parent
        self.Prefix = prefix
        self.PortLo = port_lo
        self.PortHi = port_hi
        self.ProtoName = proto
        self.ProtoNum = utils.GetIPProtoByName(proto)
        self.AddrType = addr_type

    def Show(self):
        logger.info("NAT Port Block object:", self)
        logger.info("- Prefix:%s" % self.Prefix)
        logger.info("- Port Range:%d-%d" % (self.PortLo, self.PortHi))
        logger.info("- Proto:%s" % self.ProtoName)
        logger.info("- AddrType:%d" % self.AddrType)

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.VpcId = self.VPC.GetKey()
        spec.Protocol = self.ProtoNum
        spec.NatAddress.Prefix.IPv4Subnet.Len = self.Prefix.prefixlen
        spec.NatAddress.Prefix.IPv4Subnet.Addr.Af = types_pb2.IP_AF_INET
        spec.NatAddress.Prefix.IPv4Subnet.Addr.V4Addr = int(self.Prefix.network_address)
        spec.Ports.PortLow = self.PortLo
        spec.Ports.PortHigh = self.PortHi
        if self.AddrType == utils.NAT_ADDR_TYPE_PUBLIC:
            spec.AddressType = types_pb2.ADDR_TYPE_PUBLIC
        else:
            spec.AddressType = types_pb2.ADDR_TYPE_SERVICE

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.Protocol != self.ProtoNum:
            return False
        ports = spec.Ports
        if ports.PortLow != self.PortLo:
            return False
        if ports.PortHigh != self.PortHi:
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if utils.GetYamlSpecAttr(spec) != self.GetKey():
            return False
        if spec['protocol'] != self.ProtoNum:
            return False
        ports = spec['ports']
        if ports['portlow'] != self.PortLo:
            return False
        if ports['porthigh'] != self.PortHi:
            return False
        return True

    def GetStats(self):
        if utils.IsDryRun():
            return
        grpcmsg = nat_pb2.NatPortBlockGetRequest()
        grpcmsg.Id.append(self.GetKey())
        resp = api.client[self.Node].Get(api.ObjectTypes.NAT, [ grpcmsg ])
        if resp is None:
            return None

        stats = NatPbStats()
        stats.InUseCount = resp[0].Response[0].Stats.InUseCount
        stats.SessionCount = resp[0].Response[0].Stats.SessionCount
        return stats

class NatPbObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.NAT, Resmgr.MAX_NAT_PB)
        self.AllNatPbs = []
        self.VpcNatPbs = []
        self.VpcNatPbs.append(defaultdict(list))
        self.VpcNatPbs.append(defaultdict(list))

    def __add_to_vpc_db(self, nat_type, vpc_obj, nat_obj):
        self.AllNatPbs.append(nat_obj)
        self.VpcNatPbs[nat_type][vpc_obj.GetKey()].append(nat_obj)

    def GetVpcNatPortBlocks(self, nat_type, vpc_key):
        return self.VpcNatPbs[nat_type][vpc_key]

    def GetAllNatPortBlocks(self):
        return self.AllNatPbs

    def GenerateObjects(self, node, parent, vpc_spec_obj):
        nat_spec = vpc_spec_obj.nat
        for i in range(nat_spec.count):
            if nat_spec.addrtype == 'PUBLIC_AND_SERVICE':
                prefix_internet = parent.AllocNatPrefix(utils.NAT_ADDR_TYPE_PUBLIC)
                prefix_infra = parent.AllocNatPrefix(utils.NAT_ADDR_TYPE_SERVICE)
            elif nat_spec.addrtype == 'PUBLIC':
                prefix_internet = parent.AllocNatPrefix(utils.NAT_ADDR_TYPE_PUBLIC)
                prefix_infra = NULL
            elif nat_spec.addrtype == 'SERVICE':
                prefix_internet = NULL
                prefix_infra = parent.AllocNatPrefix(utils.NAT_ADDR_TYPE_SERVICE)
            else:
                prefix_internet = NULL
                prefix_infra = NULL
            protos = nat_spec.protocol.split(',')
            for proto in protos:
                port_lo, port_hi = ResmgrClient[node].GetNatPoolPortRange(proto)
                if prefix_internet:
                    obj = NatPbObject(node, parent, prefix_internet, port_lo, \
                        port_hi, proto, utils.NAT_ADDR_TYPE_PUBLIC)
                    self.Objs[node].update({obj.Id : obj})
                    self.__add_to_vpc_db(utils.NAT_ADDR_TYPE_PUBLIC, parent, obj)
                if prefix_infra:
                    obj = NatPbObject(node, parent, prefix_infra, port_lo, \
                        port_hi, proto, utils.NAT_ADDR_TYPE_SERVICE)
                    self.Objs[node].update({obj.Id : obj})
                    self.__add_to_vpc_db(utils.NAT_ADDR_TYPE_SERVICE, parent, obj)

client = NatPbObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
