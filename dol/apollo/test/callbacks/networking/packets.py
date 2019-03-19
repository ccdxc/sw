# /usr/bin/python3
import pdb
from infra.api.objects import PacketHeader
from infra.common.objects import IpAddress
from infra.common.objects import Ipv6Address
from infra.common.objects import MacAddressBase
import infra.api.api as infra_api

def __get_packet_template_impl(obj, args):
    template = 'ETH'
    template += "_%s" % (obj.AF)

    if args is not None:
        template += "_%s" % (args.proto)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateByMapping(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.root, args)


def __get_packet_encap_impl(obj, args):
    if obj.FwType == 'VNIC':
        encap = 'ENCAP_MPLS2'
    else:
        encap = 'ENCAP_MPLS'
    return infra_api.GetPacketTemplate(encap)

def GetPacketEncapByMapping(testcase, packet, args=None):
    encaps = []
    encaps.append(__get_packet_encap_impl(testcase.config.root, args))
    return encaps
