# /usr/bin/python3
import pdb
import ipaddress
import random
from scapy.all import *
from infra.common.logging import logger
import infra.api.api as infra_api
import apollo.config.resmgr as resmgr
import apollo.config.utils as utils
import apollo.config.topo as topo
import policy_pb2 as policy_pb2
import tunnel_pb2 as tunnel_pb2
import types_pb2 as types_pb2

IPV4_HOST = ipaddress.IPv4Address(0xbadee1ba)
IPV6_HOST = ipaddress.IPv6Address('e1ba:aced:a11:face:b00c:bade:da75:900d')

IPV4_MHOST = ipaddress.IPv4Address(0xffffffff)
IPV6_MHOST = ipaddress.IPv6Address('ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff')

def __get_packet_template_impl(obj, args):
    template = 'ETH'
    template += "_%s" % (obj.AddrFamily)

    if args is not None:
        template += "_%s" % (args.proto)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateFromMapping(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.localmapping, args)

def __get_proto_from_policy_rule(rule):
    if rule is None:
        return "TCP"
    l3match = rule.L3Match
    if not l3match.valid:
        return "TCP"
    return utils.GetIPProtoName(l3match.Proto)

def __get_packet_template_from_policy_impl(rule, policy):
    protocol = __get_proto_from_policy_rule(rule)
    template = 'ETH_%s_%s' % (policy.AddrFamily, protocol)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateFromPolicy(testcase, packet, args=None):
    return __get_packet_template_from_policy_impl(testcase.config.tc_rule, testcase.config.policy)

def __get_non_default_random_route(routes):
    numroutes = len(routes)
    if numroutes == 0:
        return None
    elif numroutes == 1:
        route = None
        if not utils.isDefaultRoute(list(routes)[0].ipaddr):
            route = list(routes)[0]
        if route:
            return route.ipaddr
        else:
            return None
    while True:
        route = random.choice(list(routes))
        if not utils.isDefaultRoute(route.ipaddr):
            break
    return route.ipaddr

def __get_host_from_pfx_impl(pfx, af):
    """
        if route is None / default route, returns IPV4_HOST / IPV6_HOST
        returns a random usable host otherwise
    """
    if pfx is None or utils.isDefaultRoute(pfx):
        if af == "IPV4":
            return str(IPV4_HOST)
        elif af == "IPV6":
            return str(IPV6_HOST)
        else:
            logger.error("ERROR: invalid AddrFamily ", af)
            sys.exit(1)
            return None
    total_hosts = pfx.num_addresses
    host = pfx.network_address
    if total_hosts > 1:
        host += random.randint(0, total_hosts-1)
    return str(host)

def __get_host_from_pfx(pfx, af, pos=None):
    if pfx is None or utils.isDefaultRoute(pfx) or pos is None:
        host = __get_host_from_pfx_impl(pfx, af)
    elif pos == 'right':
        #first ip post prefix range
        host = pfx.network_address + pfx.num_addresses
    elif pos == 'left':
        #first ip pre prefix range
        host = pfx.network_address - 1
    elif pos == 'first':
        #first ip in prefix range
        host = pfx.network_address
    elif pos == 'last':
        #last ip in prefix range
        host = pfx.network_address + pfx.num_addresses - 1
    return str(host)

def __get_module_args_value(modargs, attr):
    if modargs is not None:
        for args in modargs:
            if hasattr(args, attr):
                pval = getattr(args, attr, None)
                return pval
    return None

def __get_pfx_position_selector(modargs):
    return __get_module_args_value(modargs, 'prefix')

def __get_usable_pfx_from_range(ipaddrLow, ipaddrHigh, pos):
    if not all([ipaddrLow, ipaddrHigh]):
        return None
    elif pos == 'right':
        #first ip beyond range
        host = ipaddrHigh + 1
    elif pos == 'left':
        #first ip before range
        host = ipaddrLow - 1
    elif pos == 'first':
        #first ip in range
        host = ipaddrLow
    elif pos == 'last':
        #last ip in range
        host = ipaddrHigh
    else:
        # random pfx between ipaddrLow and ipaddrHigh
        pfxlist = [ipaddr for ipaddr in ipaddress.summarize_address_range(ipaddrLow, ipaddrHigh)]
        return random.choice(pfxlist)

    return ipaddress.ip_network(host)

def __get_usable_pfx_from_tag(tag, pfxpos):
    pfxlist = tag.Prefixes if tag else None
    if pfxlist is None:
        pfx = None
    else:
        pfx = random.choice(pfxlist)
    return pfx

def __get_usable_pfx_from_rule_impl(matchtype, ippfx=None, ipaddrLow=None, ipaddrHigh=None, tag=None, pfxpos=None):
    if matchtype == topo.L3MatchType.PFX:
        return ippfx
    elif matchtype == topo.L3MatchType.PFXRANGE:
        return __get_usable_pfx_from_range(ipaddrLow, ipaddrHigh, pfxpos)
    elif matchtype == topo.L3MatchType.TAG:
        return __get_usable_pfx_from_tag(tag, pfxpos)
    return None

def __get_usable_host_from_rule(rule, policy, pfxpos):
    af = policy.AddrFamily
    if rule is None:
        return __get_host_from_pfx(None, af, pfxpos)
    direction = policy.Direction
    l3match = rule.L3Match
    if not l3match.valid:
        pfx = None
    elif direction == types_pb2.RULE_DIR_INGRESS:
        pfx = __get_usable_pfx_from_rule_impl(l3match.SrcType, l3match.SrcPrefix, l3match.SrcIPLow, l3match.SrcIPHigh, l3match.SrcTag, pfxpos)
    else:
        pfx = __get_usable_pfx_from_rule_impl(l3match.DstType, l3match.DstPrefix, l3match.DstIPLow, l3match.DstIPHigh, l3match.DstTag, pfxpos)
    return __get_host_from_pfx(pfx, af, pfxpos)

def GetUsableHostFromPolicy(testcase, packet, args=None):
    policy = testcase.config.policy
    rule = testcase.config.tc_rule
    pfxpos = __get_pfx_position_selector(testcase.module.args)
    return __get_usable_host_from_rule(rule, policy, pfxpos)

def __get_random_port_in_range(beg=utils.L4PORT_MIN, end=utils.L4PORT_MAX):
    return random.randint(beg, end)

def __get_valid_port(port):
    if port < utils.L4PORT_MIN:
        return utils.L4PORT_MIN
    elif port > utils.L4PORT_MAX:
        return utils.L4PORT_MAX
    else:
        return port

def __get_port_from_rule(rule, pos=None, isSource=True):
    l4matchobj = rule.L4Match if rule else None
    if l4matchobj is None or l4matchobj.valid == False:
        return __get_random_port_in_range()
    if isSource:
        beg = l4matchobj.SportLow
        end = l4matchobj.SportHigh
    else:
        beg = l4matchobj.DportLow
        end = l4matchobj.DportHigh
    if pos == 'right':
        port = end + 1
    elif pos == 'left':
        port = beg - 1
    elif pos == 'first':
        port = beg
    elif pos == 'last':
        port = end
    else:
        return __get_random_port_in_range(beg, end)
    return __get_valid_port(port)

def GetUsableSPortFromPolicy(testcase, packet, args=None):
    rule = testcase.config.tc_rule
    pos = __get_module_args_value(testcase.module.args, 'sport')
    return __get_port_from_rule(rule, pos)

def GetUsableDPortFromPolicy(testcase, packet, args=None):
    rule = testcase.config.tc_rule
    pos = __get_module_args_value(testcase.module.args, 'dport')
    return __get_port_from_rule(rule, pos, False)

def __get_random_icmptype_in_range(beg=utils.ICMPTYPE_MIN, end=utils.ICMPTYPE_MAX):
    return random.randint(beg, end)

def __get_valid_icmptype(icmptype):
    if icmptype < utils.ICMPTYPE_MIN:
        return utils.ICMPTYPE_MIN
    elif icmptype > utils.ICMPTYPE_MAX:
        return utils.ICMPTYPE_MAX
    else:
        return icmptype

def __get_icmp_values_from_rule(rule, pos=None, isIcmpCode=False):
    l4matchobj = rule.L4Match if rule else None
    if l4matchobj is None or l4matchobj.valid == False:
        return __get_random_icmptype_in_range()
    if isIcmpCode:
        val = l4matchobj.IcmpCode
    else:
        val = l4matchobj.IcmpType
    if pos == 'right':
        val = val + 1
    elif pos == 'left':
        val = val - 1
    return __get_valid_icmptype(val)

def GetUsableICMPTypeFromPolicy(testcase, packet, args=None):
    rule = testcase.config.tc_rule
    pos = __get_module_args_value(testcase.module.args, 'icmptype')
    return __get_icmp_values_from_rule(rule, pos)

def GetUsableICMPCodeFromPolicy(testcase, packet, args=None):
    rule = testcase.config.tc_rule
    pos = __get_module_args_value(testcase.module.args, 'icmpcode')
    return __get_icmp_values_from_rule(rule, pos, True)

def __is_matching_ip(matchtype, ipaddr, ippfx=None, ipaddrLow=None, ipaddrHigh=None, tag=None):
    ipaddr = ipaddress.ip_address(ipaddr)
    if matchtype == topo.L3MatchType.PFX:
        if ippfx is None:
            return True
        return ipaddr in ippfx
    elif matchtype == topo.L3MatchType.PFXRANGE:
        if not all([ipaddrLow, ipaddrHigh]):
            return True
        return ((ipaddr >= ipaddrLow) and (ipaddr <= ipaddrHigh))
    elif matchtype == topo.L3MatchType.TAG:
        tagpfxlist = tag.Prefixes if tag else None
        if tagpfxlist is None:
            return False
        for tagpfx in tagpfxlist:
            if ipaddr in tagpfx:
                return True
    return False

def __is_matching_src_ip(src_ip, l3matchobj):
    return __is_matching_ip(l3matchobj.SrcType, src_ip, l3matchobj.SrcPrefix, l3matchobj.SrcIPLow, l3matchobj.SrcIPHigh, l3matchobj.SrcTag)

def __is_matching_dst_ip(dst_ip, l3matchobj):
    return __is_matching_ip(l3matchobj.DstType, dst_ip, l3matchobj.DstPrefix, l3matchobj.DstIPLow, l3matchobj.DstIPHigh, l3matchobj.DstTag)

def __is_matching_proto(proto1, proto2):
    if proto2 == 0:
        # wildcard
        return True
    return proto1 == proto2

def __is_matching_L4port(port, portLow=utils.L4PORT_MIN, portHigh=utils.L4PORT_MAX):
    return ((port >= portLow) and (port <= portHigh))

def __is_matching_sport(sport, l4matchobj):
    return __is_matching_L4port(sport, l4matchobj.SportLow, l4matchobj.SportHigh)

def __is_matching_dport(dport, l4matchobj):
    return __is_matching_L4port(dport, l4matchobj.DportLow, l4matchobj.DportHigh)

def __is_matching_icmptype(icmptype, l4matchobj):
    return icmptype == l4matchobj.IcmpType

def __is_matching_icmpcode(icmpcode, l4matchobj):
    return icmpcode == l4matchobj.IcmpCode

def __is_l3_match(packet_tuples, l3matchobj):
    if not l3matchobj.valid:
        return True
    if not __is_matching_src_ip(packet_tuples[0], l3matchobj):
        logger.verbose("l3match sip fail")
        return False
    if not __is_matching_dst_ip(packet_tuples[1], l3matchobj):
        logger.verbose("l3match dip fail")
        return False
    if not __is_matching_proto(packet_tuples[2], l3matchobj.Proto):
        logger.verbose("l3match proto fail")
        return False
    return True

def __is_l4_match(packet_tuples, l4matchobj):
    if not l4matchobj.valid:
        return True
    if utils.IsICMPProtocol(packet_tuples[2]):
        if not __is_matching_icmptype(packet_tuples[5], l4matchobj):
            logger.verbose("l4match icmp type fail")
            return False
        if not __is_matching_icmpcode(packet_tuples[6], l4matchobj):
            logger.verbose("l4match icmp code fail")
            return False
    else:
        if not __is_matching_sport(packet_tuples[3], l4matchobj):
            logger.verbose("l4match sport fail")
            return False
        if not __is_matching_dport(packet_tuples[4], l4matchobj):
            logger.verbose("l4match dport fail")
            return False
    return True

def __is_matching_rule(packet_tuples, rule):
    if not __is_l3_match(packet_tuples, rule.L3Match):
        logger.verbose("l3match fail")
        return False
    if not __is_l4_match(packet_tuples, rule.L4Match):
        logger.verbose("l4match fail")
        return False
    return True

def __get_packet_tuples(pkt):
    payload = None
    sip = None
    dip = None
    proto = 0
    sport = 0
    dport = 0
    icmptype = 0
    icmpcode = 0
    logger.info("Actual Packet ", pkt.summary())
    if IP in pkt:
        sip = pkt[IP].src
        dip = pkt[IP].dst
        proto = pkt[IP].proto
    elif IPv6 in pkt:
        sip = pkt[IPv6].src
        dip = pkt[IPv6].dst
        proto = pkt[IPv6].nh
        payload = pkt[IPv6].payload
    if TCP in pkt:
        sport = pkt[TCP].sport
        dport = pkt[TCP].dport
    elif UDP in pkt:
        sport = pkt[UDP].sport
        dport = pkt[UDP].dport
    elif ICMP in pkt:
        icmptype = pkt[ICMP].type
        icmpcode = pkt[ICMP].code
    elif proto == 58: #ICMPv6
        icmptype = payload.type
        icmpcode = payload.code
    #TODO: make packet_tuples a class
    packet_tuples = [sip, dip, proto, sport, dport, icmptype, icmpcode]
    logger.info("Retrieved Packet Tuples ", packet_tuples)
    return packet_tuples

def __get_final_result(tc_rule, match_rule):
    final_result = match_rule.Action if match_rule else policy_pb2.SECURITY_RULE_ACTION_DENY
    logger.info("TestCase rule for packet ")
    tc_rule.Show()
    if match_rule:
        logger.info("Final matching rule for packet")
        match_rule.Show()
    else:
        logger.info("No rule matching the packet - so DENY")
    return final_result

def __get_matching_rule(policy, pkt, tc_rule):
    packet_tuples = __get_packet_tuples(pkt)
    action = tc_rule.Action
    prio = tc_rule.Priority
    # Get a new copy of stable sorted (based on priority) rules
    # Note: lower the value of rule.Priority higher the Priority
    rules = sorted(policy.rules, key=lambda x: x.Priority)
    match_rule = None
    for rule in rules:
        if __is_matching_rule(packet_tuples, rule):
            match_rule = rule
            break
    return match_rule

def GetExpectedPacket(testcase, args):
    tc_rule = testcase.config.tc_rule
    if tc_rule is None:
        # no rule in policy - so implicit deny
        return testcase.packets.Get(args.epkt_fail)
    policy = testcase.config.policy
    pkt = testcase.packets.Get(args.ipkt).GetScapyPacket()
    match_rule = __get_matching_rule(policy, pkt, tc_rule)
    final_result = __get_final_result(tc_rule, match_rule)
    if final_result == policy_pb2.SECURITY_RULE_ACTION_DENY:
        return testcase.packets.Get(args.epkt_fail)
    return testcase.packets.Get(args.epkt_pass)

def GetInvalidMPLSTag(testcase, packet, args=None):
    return next(resmgr.InvalidMplsSlotIdAllocator)

def GetInvalidVnid(testcase, packet, args=None):
    return next(resmgr.InvalidVxlanIdAllocator)

def __get_mapping_packet_encap_impl(dev, lmap, args):
    if dev.IsEncapTypeVXLAN():
        encap = 'ENCAP_VXLAN_IPV6' if lmap.TunFamily == 'IPV6' else 'ENCAP_VXLAN'
    else:
        assert 0
    return infra_api.GetPacketTemplate(encap)

# This can be called for packets to switch or from switch
def GetPacketEncapFromMapping(testcase, packet, args=None):
    encaps = []
    encaps.append(__get_mapping_packet_encap_impl(testcase.config.devicecfg, testcase.config.localmapping, args))
    return encaps

def __get_host_packet_encap_impl(vnic):
    encaps = []
    if vnic.IsEncapTypeVLAN():
        vlan_encap = infra_api.GetPacketTemplate('ENCAP_QTAG')
        encaps.append(vlan_encap)
    return encaps

 # Encap for to/from host packet
def GetHostPacketEncapFromVnic(testcase, packet, args=None):
    if args is None:
        vnic = testcase.config.localmapping.VNIC
    else:
        vnic = testcase.config.remotemapping.VNIC
    return __get_host_packet_encap_impl(vnic)

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
    if testcase.config.route is not None and testcase.config.route.TUNNEL is not None:
        tunnel = testcase.config.route.TUNNEL
    else:
        tunnel = None
    return __get_ip_localmapping_impl(testcase.config.localmapping, tunnel)

def __get_packet_encap_impl_from_route(obj, tunnel, args):
    if obj.IsEncapTypeVXLAN():
        direction = __get_module_args_value(args, 'direction')
        if tunnel.Type is tunnel_pb2.TUNNEL_TYPE_SERVICE and \
            tunnel.Remote is True and direction == "remote2local":
            encap = 'ENCAP_VXLAN2'
        else:
            encap = 'ENCAP_VXLAN'
    else:
        assert 0
    return infra_api.GetPacketTemplate(encap)

# This can be called for packets to switch or from switch
def GetPacketEncapFromRoute(testcase, packet, args=None):
    encaps = []
    encaps.append(__get_packet_encap_impl_from_route(testcase.config.devicecfg, testcase.config.route.TUNNEL, testcase.module.args))
    return encaps

def GetCPSPacketIFlowFlagsFromRoute(testcase, packet, args=None):
    if testcase.config.route.AddrFamily == 'IPV6':
        return 128
    return 0

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
    if route is None or pval is None:
        host = __get_host_from_route_impl(route, af)
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
    return str(host)

def GetUsableHostFromRoute(testcase, packet, args=None):
    route = __get_non_default_random_route(testcase.config.route.routes.values())
    if args != None and args.addr == 'last':
        if route != None:
            return str(route.network_address + route.num_addresses - 1)
        else:
            return str(IPV4_MHOST) if testcase.config.route.AddrFamily == "IPV4" else str(IPV6_MHOST)
    return __get_host_from_route(testcase.module.args, route, testcase.config.route.AddrFamily)

def __get_sipo_from_svctun(lmapping, tunnel):
    if tunnel.Type is tunnel_pb2.TUNNEL_TYPE_SERVICE and tunnel.Remote is True:
        return str(tunnel.RemoteServicePublicIP)
    else:
        return str(lmapping.ProviderIP)

def GetSIPoFromSvcTun(testcase, packet, args=None):
    return __get_sipo_from_svctun( \
            testcase.config.localmapping, \
            testcase.config.route.TUNNEL)

def __get_ipv6_from_nat46(lmapping):
    ipv6_pfx = lmapping.VNIC.SUBNET.VPC.Nat46_pfx
    ipv6_pfx = ipv6_pfx.exploded.split("/")
    ipv6_pfx = ipv6_pfx[0]
    prefix6to4 = int(ipaddress.IPv6Address(ipv6_pfx))
    ip4 = ipaddress.IPv4Address(lmapping.IP)
    ip6 = ipaddress.IPv6Address(prefix6to4 | (int(ip4)))
    return str(ip6)

def GetIPV6FromNat46(testcase, packet, args=None):
    if testcase.config.localmapping is not None:
        return __get_ipv6_from_nat46(testcase.config.localmapping)

def __get_ipv4_from_ipv6(addr):
    if addr.version is 4:
        assert 0
    if addr.version is 6:
        v4addr = addr.exploded.split(":")
        v4addr = v4addr[6] + v4addr[7]
        v4addr = v4addr[:2] + '.' + v4addr[2:4] + '.' + v4addr[4:6] +'.'+ v4addr[6:]
        return v4addr
    return None

def __get_packet_v4_dipo_remote46(tunnel):
    if tunnel is not None:
        return __get_ipv4_from_ipv6(tunnel.RemoteIPAddr)
    return None

def GetPacketV4DIPoRemote46(testcase, packet, args=None):
    if testcase.config.route is not None and testcase.config.route.TUNNEL is not None:
        return __get_packet_v4_dipo_remote46(testcase.config.route.TUNNEL)

def __get_packet_v6_dipo_remote46(tunnel):
    if tunnel is not None:
        return tunnel.RemoteIPAddr
    return None

def GetPacketV6DIPoRemote46(testcase, packet, args=None):
    if testcase.config.route is not None and testcase.config.route.TUNNEL is not None:
        return __get_packet_v6_dipo_remote46(testcase.config.route.TUNNEL)
