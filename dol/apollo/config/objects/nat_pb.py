#! /usr/bin/python3

import pdb

from infra.common.logging import logger

import apollo.config.objects.base as base
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo

import service_pb2 as service_pb2
import types_pb2 as types_pb2
import nat_pb2 as nat_pb2

class NatPbObject(base.ConfigObjectBase):
    def __init__(self, parent, addr, port_lo, port_hi, proto, addr_type):
        super().__init__(api.ObjectTypes.NAT_PB)
        self.Id = next(resmgr.NatPoolIdAllocator)
        self.GID('NatPortBlock%d'%self.Id)
        self.VPC = parent
        self.Addr = addr
        self.PortLo = port_lo
        self.PortHi = port_hi
        self.ProtoName = proto
        self.ProtoNum = utils.GetIPProtoByName(proto)
        self.AddrType = addr_type

    def Show(self):
        logger.info("NAT Port Block object:", self)
        logger.info("- Addr:%s" % self.Addr)
        logger.info("- Port Range:%d-%d" % (self.PortLo, self.PortHi))
        logger.info("- Proto:%s" % self.ProtoName)
        logger.info("- AddrType:%d" % self.AddrType)

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.Id)

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.Id
        spec.VpcId = self.VPC.VPCId
        spec.Protocol = self.ProtoNum
        spec.NatAddress.Prefix.IPv4Subnet.Addr.Af = types_pb2.IP_AF_INET
        spec.NatAddress.Prefix.IPv4Subnet.Addr.V4Addr = int(self.Addr)
        spec.NatAddress.Prefix.IPv4Subnet.Len = 32
        spec.Ports.PortLow = self.PortLo
        spec.Ports.PortHigh = self.PortHi
        if self.AddrType == utils.NAT_ADDR_TYPE_PUBLIC:
            spec.AddressType = types_pb2.ADDR_TYPE_PUBLIC
        else:
            spec.AddressType = types_pb2.ADDR_TYPE_SERVICE

    def ValidateSpec(self, spec):
        if spec.Id != self.Id:
            return False
        return True

class NatPbObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.NAT_PB, resmgr.MAX_NAT_PB)

    def GenerateObjects(self, parent, vpc_spec_obj):
        nat_spec = vpc_spec_obj.nat
        for i in range(nat_spec.count):
            if nat_spec.addrtype == 'PUBLIC_AND_SERVICE':
                addr_internet = parent.AllocNatAddr(utils.NAT_ADDR_TYPE_PUBLIC)
                addr_infra = parent.AllocNatAddr(utils.NAT_ADDR_TYPE_SERVICE)
            elif nat_spec.addrtype == 'PUBLIC':
                addr_internet = parent.AllocNatAddr(utils.NAT_ADDR_TYPE_PUBLIC)
                addr_infra = NULL
            elif nat_spec.addrtype == 'SERVICE':
                addr_internet = NULL
                addr_infra = parent.AllocNatAddr(utils.NAT_ADDR_TYPE_SERVICE)
            else:
                addr_internet = NULL
                addr_infra = NULL
            protos = nat_spec.protocol.split(',')
            port_lo, port_hi = resmgr.GetNatPoolPortRange()
            for proto in protos:
                if addr_internet:
                    obj = NatPbObject(parent, addr_internet, port_lo, \
                        port_hi, proto, utils.NAT_ADDR_TYPE_PUBLIC)
                    self.Objs.update({obj.Id : obj})
                if addr_infra:
                    obj = NatPbObject(parent, addr_internet, port_lo, \
                        port_hi, proto, utils.NAT_ADDR_TYPE_SERVICE)
                    self.Objs.update({obj.Id : obj})

    def PdsctlRead(self):
        # pdsctl show not supported for nat port blocks
        return

    def GrpcRead(self):
        # grpc read not supported for nat port blocks
        return

    def ValidateGrpcRead(self, getResp):
        # read not supported, return true for now
        return True

client = NatPbObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
