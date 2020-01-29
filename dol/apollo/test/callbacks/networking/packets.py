# /usr/bin/python3
import pdb
import ipaddress
import random
from scapy.all import *

from infra.common.logging import logger
import infra.api.api as infra_api
from apollo.config.resmgr import Resmgr
import apollo.config.utils as utils
import apollo.config.topo as topo

import policy_pb2 as policy_pb2
import types_pb2 as types_pb2

IPV4_HOST = ipaddress.IPv4Address(0xbadee1ba)
IPV6_HOST = ipaddress.IPv6Address('e1ba:aced:a11:face:b00c:bade:da75:900d')

def __get_packet_template_impl(obj, args):
    af = obj.AddrFamily
    template = 'ETH'
    template += "_%s" % (af)

    if args is not None:
        proto = args.proto
        if af == "IPV6" and proto == "icmp":
            proto = "icmpv6"
        template += "_%s" % (proto)
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

def __get_non_default_random_route(route):
    routes = list(route.routes.values())
    numroutes = len(routes)
    if numroutes == 0:
        return None
    elif numroutes == 1:
        route = None
        if not utils.isDefaultRoute(routes[0].ipaddr):
            route = routes[0]
        if route:
            return route.ipaddr
        else:
            return None
    while True:
        route = random.choice(routes)
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

def __reset_tunnel(routetbl, addr):
    if routetbl.PriorityType is True:
        routes = routetbl.routes
        min_priority = 65535
        nh_type_res = None
        result = None
        for route in routes.values():
            if route.Priority == 0: assert 0
            if addr in route.ipaddr:
                if route.Priority < min_priority:
                    min_priority = route.Priority
                    nh_type_res = route.NextHopType
                    if nh_type_res == "tep":
                        result = route.TunnelId
                    elif nh_type_res == "nexthop":
                        result = route.NexthopId
                    elif nh_type_res == "nhg":
                        result = route.NexthopGroupId
                    elif nh_type_res == "vpcpeer":
                        result = route.PeerVPCId
        # use this for getting the nexthop info
        [routetbl.TunEncap, routetbl.TunIP] = utils.getTunInfo(nh_type_res, result)

def GetUsableHostFromRoute(testcase, packet, args=None):
    route = __get_non_default_random_route(testcase.config.route)
    pfxpos = __get_pfx_position_selector(testcase.module.args)
    addr = __get_host_from_pfx(route, testcase.config.route.AddrFamily, pfxpos)
    #reset the nexthop/tunnel values based on priority for this destination
    __reset_tunnel(testcase.config.route, addr)
    return addr

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
    final_result = match_rule.Action if match_rule else types_pb2.SECURITY_RULE_ACTION_DENY
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

def GetExpectedCPSPacket(testcase, args):
    tc_rule = testcase.config.tc_rule
    if tc_rule is None:
        # no rule in policy - so implicit deny
        logger.info("GetExpectedCPSPacket: packet denied, empty rules")
        return None
    # TODO: handle vnic security policies
    policy = testcase.config.policy
    pkt = testcase.packets.Get(args.ipkt).GetScapyPacket()
    match_rule = __get_matching_rule(policy, pkt, tc_rule)
    final_result = __get_final_result(tc_rule, match_rule)
    if final_result == types_pb2.SECURITY_RULE_ACTION_DENY:
        logger.info("GetExpectedCPSPacket: packet denied")
        return None
    logger.info("GetExpectedCPSPacket: packet allowed")
    return testcase.packets.Get(args.epkt_pass)

def GetInvalidMPLSTag(testcase, packet, args=None):
    return next(Resmgr.InvalidMplsSlotIdAllocator)

def GetInvalidVnid(testcase, packet, args=None):
    return next(Resmgr.InvalidVxlanIdAllocator)

def __get_packet_encap_type_impl(obj, args):
    if obj.IsWorkload():
        return 'ENCAP_MPLS2'
    elif obj.IsIgw():
        return 'ENCAP_MPLS'
    else:
        return None

def __get_packet_encap_impl(obj, tunnel, args):
    if obj.IsEncapTypeVXLAN():
        encap = 'ENCAP_VXLAN'
    elif obj.IsEncapTypeMPLS():
        encap = __get_packet_encap_type_impl(tunnel, args)
    else:
        assert 0
    return infra_api.GetPacketTemplate(encap)

# This can be called for packets to switch or from switch
def GetPacketEncapFromMapping(testcase, packet, args=None):
    encaps = []
    encaps.append(__get_packet_encap_impl(testcase.config.devicecfg, testcase.config.tunnel, args))
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

def GetTunnelIPFromMapping(testcase, packet, args=None):
    return str(testcase.config.remotemapping.TUNNEL.RemoteIPAddr)

def GetTunnelIPFromRoute(testcase, packet, args=None):
    return str(testcase.config.route.TUNNEL.RemoteIPAddr)

def __is_any_cfg_deleted(tc):
    nexthop = None
    nexthopgroup = None
    interface = None
    device = tc.config.devicecfg
    vpc = tc.config.localmapping.VNIC.SUBNET.VPC
    routetable = tc.config.route
    subnet = tc.config.localmapping.VNIC.SUBNET
    vnic = tc.config.localmapping.VNIC
    lmapping = tc.config.localmapping
    rmapping = tc.config.remotemapping
    tunnel = tc.config.tunnel
    if tunnel is None:
        pass
    elif tunnel.IsUnderlayEcmp():
        nexthopgroup = tunnel.NEXTHOPGROUP
    elif tunnel.IsUnderlay():
        nexthop = tunnel.NEXTHOP
        interface = nexthop.L3Interface
    objs = [device, vpc, routetable, subnet, vnic, lmapping]
    objs.extend([rmapping, tunnel, nexthop, nexthopgroup, interface])
    values = list(map(lambda x: not(x.IsHwHabitant()), list(filter(None, objs))))
    if any(values):
        return True
    return False

def GetExpectedPacket(testcase, args):
    #add all the cases for checking packet and return expected packet on demand
    if __is_any_cfg_deleted(testcase):
        logger.info("one or more cfgs deleted - no packet expected")
        return None
    return testcase.packets.Get(args.pkt)

# used to check if packets are expected on the lif queue
# True  - no packets expected
# False - descriptor on expect is valid
def IsNegativeTestCase(testcase):
    # if no packet is expected, return True otherwise False
    if __is_any_cfg_deleted(testcase):
        logger.info("one or more cfgs deleted - negative testcase")
        return True
    return False

def __is_nat_enabled(routeTable):
    return routeTable.IsNatEnabled()

def __get_ip_localmapping_impl(localmapping, natEnabled):
    if natEnabled:
        publicIP = getattr(localmapping, "PublicIP", None)
        if publicIP:
            return localmapping.PublicIP
    return localmapping.IP

def GetIPFromLocalMapping(testcase, packet, args=None):
    natEnabled = __is_nat_enabled(testcase.config.route)
    return __get_ip_localmapping_impl(testcase.config.localmapping, natEnabled)

def __get_expected_nexthop(cfg):
    tunnel = cfg.tunnel
    nh = None
    if tunnel is None:
        pass
    elif tunnel.IsUnderlay():
        nh = tunnel.NEXTHOP
    return nh

def GetExpectedEgressUplinkPort(testcase, args=None):
    nh = __get_expected_nexthop(testcase.config)
    if nh is None:
        return None
    l3if = nh.L3Interface
    port = topo.EthIfIdx2Port(l3if.IfInfo.ethifidx)
    return port

def GetUplinkPortMac(testcase, args=None):
    nh = __get_expected_nexthop(testcase.config)
    if nh is None:
        return None
    l3if = nh.L3Interface
    mac = l3if.IfInfo.macaddr.get()
    return mac

def GetUnderlayRemoteMac(testcase, args=None):
    nh = __get_expected_nexthop(testcase.config)
    if nh is None:
        return None
    mac = nh.underlayMACAddr.get()
    return mac

def GetRingFromMapping(testcase, args):
    if args.type == "remote":
        hostIf = testcase.config.remotemapping.VNIC.SUBNET.HostIf
    else:
        hostIf = testcase.config.localmapping.VNIC.SUBNET.HostIf
    return hostIf.lif.GetQt(args.qid)

