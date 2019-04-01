# /usr/bin/python3
import pdb
from random import choice
from infra.api.objects import PacketHeader
from infra.common.objects import IpAddress
from infra.common.objects import Ipv6Address
from infra.common.objects import MacAddressBase
import infra.api.api as infra_api
import types_pb2 as types_pb2
import apollo.config.resmgr as resmgr

def __get_packet_template_impl(obj, args):
    template = 'ETH'
    template += "_%s" % (obj.AddrFamily)

    if args is not None:
        template += "_%s" % (args.proto)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateFromMapping(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.localmapping, args)

def __get_host_from_route_impl(obj):
    prefix = choice(obj.routes)
    return str(next(prefix.hosts()))

def GetUsableHostFromRoute(testcase, packet, args=None):
    return __get_host_from_route_impl(testcase.config.route)

def GetInvalidMPLSTag(testcase, packet, args=None):
    return next(resmgr.InvalidMplsSlotIdAllocator)

def GetInvalidVnid(testcase, packet, args=None):
    return next(resmgr.InvalidVxlanIdAllocator)

def __get_packet_encap_impl(obj, args):
    if obj.IsEncapTypeMPLS():
        encap = 'ENCAP_MPLS2'
    elif obj.IsEncapTypeVXLAN():
        encap = 'ENCAP_VXLAN'
    else:
        assert 0
    return infra_api.GetPacketTemplate(encap)

# This can be called for packets to switch or from switch
def GetPacketEncapFromMapping(testcase, packet, args=None):
    encaps = []
    encaps.append(__get_packet_encap_impl(testcase.config.devicecfg, args))
    return encaps


def __get_packet_srcmac_impl(fwdmode, dobj, robj, lobj, args):
    if dobj.IsEncapTypeMPLS():
        if fwdmode == 'L2':
            return robj.MACAddr
        else:
            return lobj.VNIC.SUBNET.VirtualRouterMACAddr
    elif dobj.IsEncapTypeVXLAN():
        return lobj.VNIC.SUBNET.VirtualRouterMACAddr
    else:
        assert 0

# This can be called for packets to host from switch
def GetPacketSrcMacAddrFromMapping(testcase, packet, args=None):
    return __get_packet_srcmac_impl(testcase.config.root.FwdMode,
            testcase.config.devicecfg, testcase.config.remotemapping,
            testcase.config.localmapping, args)

def __get_ip_localmapping_impl(localmapping):
    if hasattr(localmapping, "PublicIP"):
        return localmapping.PublicIP
    else:
        return localmapping.IP

def GetIPFromLocalMapping(testcase, packet, args=None):
    return __get_ip_localmapping_impl(testcase.config.localmapping)
