# /usr/bin/python3
import pdb
import ipaddress
import random
from infra.common.logging import logger
import infra.api.api as infra_api
import apollo.config.resmgr as resmgr
import apollo.config.utils as utils
import types_pb2 as types_pb2

IPV4_HOST = ipaddress.IPv4Address(0xbadee1ba)
IPV6_HOST = ipaddress.IPv6Address('e1ba:aced:a11:face:b00c:bade:da75:900d')

def __get_packet_template_impl(obj, args):
    template = 'ETH'
    template += "_%s" % (obj.AddrFamily)

    if args is not None:
        template += "_%s" % (args.proto)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateFromMapping(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.localmapping, args)

def __get_packet_template_from_policy_impl(rule, policy):
    template = 'ETH'
    template += "_%s" % (policy.AddrFamily)

    protocol = utils.GetIPProtoName(rule.L3Match.Proto) if rule else "TCP"
    template += "_%s" % (protocol)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateFromPolicy(testcase, packet, args=None):
    return __get_packet_template_from_policy_impl(testcase.config.tc_rule, testcase.config.policy)

def __get_non_default_random_route(routes):
    numroutes = len(routes)
    if numroutes == 0:
        return None
    elif numroutes == 1:
        route = None
        if not utils.isDefaultRoute(routes[0]):
            route = routes[0]
        return route
    while True:
        route = random.choice(routes)
        if not utils.isDefaultRoute(route):
            break
    return route

def __get_host_from_route_impl(route, af):
    """
        returns a random usable host from non-default route
        if no route / only default route exists, returns IPV4_HOST / IPV6_HOST
    """
    if route is None or utils.isDefaultRoute(route):
        if af == "IPV4":
            return str(IPV4_HOST)
        elif af == "IPV6":
            return str(IPV6_HOST)
        else:
            logger.error("ERROR: invalid AddrFamily ", af)
            sys.exit(1)
            return None
    total_hosts = route.num_addresses
    host = route.network_address
    if total_hosts > 1:
        host += random.randint(0, total_hosts-1)
    return str(host)

def __get_host_from_route(modargs, route, af):
    pval = __get_module_args_value(modargs, 'prefix')
    if pval == 'right':
        #first ip post prefix range
        host = route.network_address + route.num_addresses
    elif pval == 'left':
        #first ip pre prefix range
        host = route.network_address - 1
    elif pval == 'first':
        #first ip in prefix range
        host = route.network_address
    elif pval == 'last':
        #last ip in prefix range
        host = route.network_address + route.num_addresses - 1
    else:
        host = __get_host_from_route_impl(route, af)
    return str(host)

def GetUsableSrcHostFromPolicy(testcase, packet, args=None):
    #TODO: make an API to get based on pfx / pfxrange / tag
    route = testcase.config.tc_rule.L3Match.SrcPrefix if testcase.config.tc_rule else None
    return __get_host_from_route(testcase.module.args, route, testcase.config.policy.AddrFamily)

def GetUsableDstHostFromPolicy(testcase, packet, args=None):
    route = testcase.config.tc_rule.L3Match.DstPrefix if testcase.config.tc_rule else None
    return __get_host_from_route(testcase.module.args, route, testcase.config.policy.AddrFamily)

def __get_module_args_value(modargs, attr):
    if modargs is not None:
        for args in modargs:
            if hasattr(args, attr):
                pval = getattr(args, attr, None)
                return pval
    return None

def GetUsableHostFromPolicy(testcase, packet, args=None):
    policy = testcase.config.policy
    rule = testcase.config.tc_rule
    route = None
    if rule:
        l3match = rule.L3Match
        route = l3match.SrcPrefix if policy.Direction == types_pb2.RULE_DIR_INGRESS else l3match.DstPrefix
    return __get_host_from_route(testcase.module.args, route, policy.AddrFamily)

def __get_random_port_in_range(beg=0, end=65535):
    return random.randint(beg, end)

def __get_port_from_rule(rule, isSource=True):
    if rule is None:
        return __get_random_port_in_range()
    if isSource:
        beg = rule.L4Match.SportLow
        end = rule.L4Match.SportHigh
    else:
        beg = rule.L4Match.DportLow
        end = rule.L4Match.DportHigh
    return __get_random_port_in_range(beg, end)

def GetUsableSPortFromPolicy(testcase, packet, args=None):
    rule = testcase.config.tc_rule
    pval = __get_module_args_value(testcase.module.args, 'sport')
    if pval == 'right':
        return (rule.L4Match.SportHigh + 1)
    if pval == 'left':
        return (rule.L4Match.SportLow - 1)
    if pval == 'first':
        rule.Show()
        return (rule.L4Match.SportLow)
    if pval == 'last':
        rule.Show()
        return (rule.L4Match.SportHigh)
    return __get_port_from_rule(rule)

def GetUsableDPortFromPolicy(testcase, packet, args=None):
    rule = testcase.config.tc_rule
    pval = __get_module_args_value(testcase.module.args, 'dport')
    if pval == 'right':
        return (rule.L4Match.DportHigh + 1)
    if pval == 'left':
        return (rule.L4Match.DportLow - 1)
    if pval == 'first':
        return (rule.L4Match.DportLow)
    if pval == 'last':
        return (rule.L4Match.DportHigh)
    return __get_port_from_rule(rule, False)

def GetInvalidMPLSTag(testcase, packet, args=None):
    return next(resmgr.InvalidMplsSlotIdAllocator)

def GetInvalidVnid(testcase, packet, args=None):
    return next(resmgr.InvalidVxlanIdAllocator)

def __get_packet_encap_impl(obj, rmap, args):
    if obj.IsEncapTypeVXLAN():
        encap = 'ENCAP_VXLAN_IPV6' if rmap.TunFamily == 'IPV6' else 'ENCAP_VXLAN'
    else:
        assert 0
    return infra_api.GetPacketTemplate(encap)

# This can be called for packets to switch or from switch
def GetPacketEncapFromMapping(testcase, packet, args=None):
    encaps = []
    encaps.append(__get_packet_encap_impl(testcase.config.devicecfg, testcase.config.remotemapping, args))
    return encaps

def __get_packet_srcmac_impl(fwdmode, dobj, robj, lobj, args):
    if dobj.IsEncapTypeMPLS():
        if fwdmode == 'L2':
            return robj.MACAddr
        elif fwdmode == 'L2L':
            return robj.VNIC.SUBNET.VirtualRouterMACAddr
        else:
            return lobj.VNIC.SUBNET.VirtualRouterMACAddr
    elif dobj.IsEncapTypeVXLAN():
        if fwdmode == 'L2L':
            return robj.VNIC.SUBNET.VirtualRouterMACAddr
        else:
            return lobj.VNIC.SUBNET.VirtualRouterMACAddr
    else:
        assert 0

# This can be called for packets to host from switch
def GetPacketSrcMacAddrFromMapping(testcase, packet, args=None):
    return __get_packet_srcmac_impl(testcase.config.root.FwdMode,
            testcase.config.devicecfg, testcase.config.remotemapping,
            testcase.config.localmapping, args)

def __get_ip_localmapping_impl(localmapping, tunnel):
    if tunnel is not None:
        if tunnel.Nat is True:
            if hasattr(localmapping, "PublicIP"):
                return localmapping.PublicIP
        else:
            # If TEP does not have nat flag set, then no translation occurs.
            return localmapping.IP

def GetIPFromLocalMapping(testcase, packet, args=None):
    if testcase.config.route is not None and testcase.config.route.Tunnel is not None:
        tunnel = testcase.config.route.Tunnel
    else:
        tunnel = None
    return __get_ip_localmapping_impl(testcase.config.localmapping, tunnel)
