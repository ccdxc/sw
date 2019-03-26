# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
from infra.common.objects import IpAddress
from infra.common.objects import Ipv6Address
from infra.common.objects import MacAddressBase
import infra.api.api as infra_api
import tunnel_pb2 as tunnel_pb2

def __get_packet_template_impl(obj, args):
    template = 'ETH'
    template += "_%s" % (obj.AddrFamily)

    if args is not None:
        template += "_%s" % (args.proto)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateFromMapping(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.localmapping, args)

def __get_host_from_route_impl(obj):
    return str(next(obj.Prefix.hosts()))

def GetUsableHostFromRoute(testcase, packet, args=None):
    return __get_host_from_route_impl(testcase.config.route)

def __get_packet_encap_impl(obj, args):
    if obj.Encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_2:
        encap = 'ENCAP_MPLS2'
    elif obj.Encap == tunnel_pb2.TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
        encap = 'ENCAP_MPLS'
    else:
        assert 0
    return infra_api.GetPacketTemplate(encap)

# This can be called for packets to switch or from switch
def GetPacketEncapFromMapping(testcase, packet, args=None):
    encaps = []
    encaps.append(__get_packet_encap_impl(testcase.config.remotemapping, args))
    return encaps


def __get_packet_srcmac_impl(fwdmode, robj, lobj, args):
    if fwdmode == 'L2':
        return robj.MACAddr
    else:
        return lobj.VNIC.SUBNET.VirtualRouterMACAddr

# This can be called for packets to host from switch
def GetPacketSrcMacAddrFromMapping(testcase, packet, args=None):
    return __get_packet_srcmac_impl(testcase.config.root.FwdMode,
            testcase.config.remotemapping, testcase.config.localmapping, args)
