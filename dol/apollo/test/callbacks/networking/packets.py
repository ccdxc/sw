# /usr/bin/python3
import ipaddress
import random
from scapy.all import *

from infra.common.logging import logger
import infra.api.api as infra_api
from apollo.config.resmgr import Resmgr
from apollo.config.store import EzAccessStore
import apollo.config.utils as utils
import apollo.config.topo as topo
from apollo.config.agent.api import ObjectTypes as ObjectTypes
import types_pb2 as types_pb2

IPV4_HOST = ipaddress.IPv4Address(0xbadee1ba)
IPV6_HOST = ipaddress.IPv6Address('e1ba:aced:a11:face:b00c:bade:da75:900d')

def __get_packet_template_impl(obj, tsargs=None, modargs=None):
    af = obj.AddrFamily
    template = 'ETH'
    template += "_%s" % (af)
    proto = None

    # testspec args takes precedence over module args
    if tsargs is not None:
        proto = tsargs.proto
    elif modargs is not None:
        proto = __get_module_args_value(modargs, 'proto')
    else:
        assert(0)

    if af == "IPV6" and proto == "icmp":
        proto = "icmpv6"
    template += "_%s" % (proto)
    return infra_api.GetPacketTemplate(template)

def GetPacketTemplateFromMapping(testcase, packet, args=None):
    return __get_packet_template_impl(testcase.config.localmapping, args, testcase.module.args)

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

def __get_non_default_random_route(routeTable):
    routes = list(routeTable.routes.values())
    numroutes = len(routes)
    route = None
    if numroutes == 0:
        pass
    elif numroutes == 1:
        if not utils.isDefaultRoute(routes[0].ipaddr):
            route = routes[0]
    else:
        while True:
            route = random.choice(routes)
            if hasattr(route, "DstNatIp") and route.DstNatIp:
                continue
            if not utils.isDefaultRoute(route.ipaddr):
                break
    return route

def __get_dnat_route(route_tbl):
    return str(route_tbl.ServiceNatPrefix.network_address)

def __get_dnat_route_dest(route_tbl):
    return str(route_tbl.DstNatIp)

def GetUsableHostFromRoute(testcase, packet, args=None):
    route = __get_non_default_random_route(testcase.config.route)
    routepfx = route.ipaddr if route else None
    pfxpos = __get_pfx_position_selector(testcase.module.args)
    addr = __get_host_from_pfx(routepfx, testcase.config.route.AddrFamily, pfxpos)
    return addr

def GetDnatRoute(testcase, packet, args=None):
    addr = __get_dnat_route(testcase.config.route)
    logger.info("returning ", addr)
    return addr

def GetDnatRouteDest(testcase, packet, args=None):
    addr = __get_dnat_route_dest(testcase.config.route)
    logger.info("returning ", addr)
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
    if tc_rule == None:
        logger.info("No Rule ")
    else:
        tc_rule.Show()
    if match_rule:
        logger.info("Final matching rule for packet")
        match_rule.Show()
    else:
        logger.info("No rule matching the packet - so DENY")
    return final_result

def __get_matching_rule(policies, pkt):
    packet_tuples = __get_packet_tuples(pkt)
    rules = []
    for policyid in policies:
        policyobj = __get_config_object(ObjectTypes.POLICY, policyid)
        rules.extend(policyobj.rules)
    # Get a new copy of stable sorted (based on priority) rules
    # Note: lower the value of rule.Priority higher the Priority
    rules = sorted(rules, key=lambda x: x.Priority)
    match_rule = None
    for rule in rules:
        if __is_matching_rule(packet_tuples, rule):
            # if priorities are same for multiple rules, choose the first
            match_rule = rule
            break
    return match_rule

def __get_security_policies_from_lmapping(lmapping, direction):
    attr = ""
    attr += "Ing" if direction == "ingress" else "Eg"
    attr += "V4" if lmapping.IsV4() else "V6"
    attr += "SecurityPolicyIds"
    vnic = lmapping.VNIC
    subnet = vnic.SUBNET
    policies = []
    # TODO: order in which they applied
    policies.extend(getattr(vnic, attr))
    policies.extend(getattr(subnet, attr))
    return policies

def GetExpectedCPSPacket(testcase, args):
    tc_rule = testcase.config.tc_rule
    # get nacl which we selected for testing
    policy = testcase.config.policy
    # get all security policies which needs to be applied
    policies = __get_security_policies_from_lmapping(testcase.config.localmapping, policy.Direction)
    pkt = testcase.packets.Get(args.ipkt).GetScapyPacket()
    match_rule = __get_matching_rule(policies, pkt)
    final_result = __get_final_result(tc_rule, match_rule)
    if final_result == types_pb2.SECURITY_RULE_ACTION_DENY:
        logger.info("GetExpectedCPSPacket: packet denied")
        return None
    logger.info("GetExpectedCPSPacket: packet allowed")
    return testcase.packets.Get(args.epkt_pass)

def IsCPSPacketExpected(testcase, args):
    pkt = GetExpectedCPSPacket(testcase, args)
    return True if pkt is None else False

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

def __get_packet_srcmac(fwdmode, robj, lobj):
    local_subnet = lobj.VNIC.SUBNET
    if fwdmode == 'L2L':
        remote_subnet = robj.VNIC.SUBNET
        if local_subnet.SubnetId != remote_subnet.SubnetId:
            return robj.VNIC.SUBNET.VirtualRouterMACAddr
        else:
            return lobj.VNIC.MACAddr
    elif fwdmode == 'L2R':
        remote_subnet = robj.SUBNET
        if local_subnet.SubnetId != remote_subnet.SubnetId:
            return robj.SUBNET.VirtualRouterMACAddr
        else:
            return lobj.VNIC.MACAddr
    elif fwdmode == 'R2L':
        return lobj.SUBNET.VirtualRouterMACAddr
    elif fwdmode == 'L2N':
        return lobj.VNIC.SUBNET.VPC.VirtualRouterMACAddr
    elif fwdmode == 'N2L':
        return lobj.VNIC.SUBNET.VirtualRouterMACAddr

def GetPacketSrcMacFromMapping(tc, packet, args=None):
    return __get_packet_srcmac(args.Fwdmode,
            tc.config.remotemapping,
            tc.config.localmapping)

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

def __get_packet_ttl(fwdmode, robj, lobj, ttl):
    local_subnet = lobj.VNIC.SUBNET
    if fwdmode == 'L2L':
        remote_subnet = robj.VNIC.SUBNET
        if local_subnet.SubnetId != remote_subnet.SubnetId:
            return ttl - 1
        else:
            return ttl
    elif fwdmode == 'L2R':
        remote_subnet = robj.SUBNET
        if local_subnet.SubnetId != remote_subnet.SubnetId:
            return ttl - 1
        else:
            return ttl
    elif fwdmode == 'R2L':
        return ttl
    elif fwdmode == 'L2N' or fwdmode == 'N2L':
        return ttl - 1

def GetPacketTtl(tc, packet, args=None):
    pkt = tc.packets.Get(args.ipkt).GetScapyPacket()
    if IP in pkt:
        ttl = pkt[IP].ttl
    else:
        assert 0
    return __get_packet_ttl(args.Fwdmode,
            tc.config.remotemapping,
            tc.config.localmapping, ttl)

def GetUsableICMPId(tc, packet, args=None):
    if GetUsableICMPId.count < 65535 and GetUsableICMPId.count >= 10000:
        GetUsableICMPId.count += 1
    else:
        GetUsableICMPId.count = 10000
    return GetUsableICMPId.count
GetUsableICMPId.count = 10000

def GetTunnelIPFromMapping(testcase, packet, args=None):
    return str(testcase.config.remotemapping.TUNNEL.RemoteIPAddr)

def __get_config_object(objtype, id):
    dutNode = EzAccessStore.GetDUTNode()
    objClient = EzAccessStore.GetConfigClient(objtype)
    return objClient.GetObjectByKey(dutNode, id)

def __getTunObject(nh_type, id):
    if nh_type == "tep":
        objtype = ObjectTypes.TUNNEL
    else:
        #TODO: Handle nh, nhgroup, vpc_peer
        assert(0)
    return __get_config_object(objtype, id)

def __get_matching_route(routetbl, addr):
    addr = ipaddress.ip_address(addr)
    matching_route = None
    matching_masklen = 0
    if routetbl.PriorityType:
        min_priority = topo.MIN_ROUTE_PRIORITY + 1
        routes = routetbl.routes
        for route in routes.values():
            if route.Priority == 0: assert 0
            if addr in route.ipaddr:
                if route.Priority  < min_priority or \
                  (route.Priority == min_priority and \
                   route.ipaddr.prefixlen > matching_masklen):
                    matching_masklen = route.ipaddr.prefixlen
                    matching_route = route
                    min_priority = route.Priority
    else:
        #TODO: move everything to above model
        assert(0)
    return matching_route

def __get_matching_route_tep(routetbl, addr):
    matching_route = __get_matching_route(routetbl, addr)
    logger.info(f"Testcase Matching route - {matching_route}")
    nh_type = matching_route.NextHopType
    if nh_type == "tep":
        nh_objid = matching_route.TunnelId
    elif nh_type == "nexthop":
        nh_objid = matching_route.NexthopId
    elif nh_type == "nhg":
        nh_objid = matching_route.NexthopGroupId
    elif nh_type == "vpcpeer":
        nh_objid = matching_route.PeerVPCId
    return __getTunObject(nh_type, nh_objid)


def __get_tunnel_from_route(tc, args):
    routetbl = tc.config.route
    pktid = getattr(args, 'ipkt', None)
    if pktid and routetbl.PriorityType:
        ipkt = tc.packets.Get(pktid).GetScapyPacket()
        packet_tuples = __get_packet_tuples(ipkt)
        addr = packet_tuples[1] if args.direction == 'TX' else packet_tuples[0]
        tep = __get_matching_route_tep(routetbl, addr)
    else:
        tep = routetbl.TUNNEL
    return tep

def GetTunnelIPFromRoute(tc, pkt, args=None):
    tep = __get_tunnel_from_route(tc, args)
    tc.config.tunnel = tep
    return str(tep.RemoteIPAddr)

def GetTunnelMacFromRoute(tc, pkt, args=None):
    tep = __get_tunnel_from_route(tc, args)
    return tep.MACAddr

def GetUplinkPortMacFromRoute(tc, pkt, args=None):
    tep = __get_tunnel_from_route(tc, args)
    if tep.IsUnderlay():
        nh = tep.NEXTHOP
    l3if = nh.L3Interface
    mac = l3if.IfInfo.macaddr.get()
    return mac

def GetUnderlayRemoteMacFromRoute(tc, pkt, args=None):
    tep = __get_tunnel_from_route(tc, args)
    if tep.IsUnderlay():
        nh = tep.NEXTHOP
    else:
        assert(0)
    return nh.underlayMACAddr.get()

def GetVirtualRouterIP(testcase, pkt, args=None):
    if testcase.config.root.FwdMode == 'L3':
        if testcase.config.remotemapping.IsV6():
            return str(testcase.config.remotemapping.SUBNET.VirtualRouterIPAddr[0].packed)
        else:
            return str(testcase.config.remotemapping.SUBNET.VirtualRouterIPAddr[1])
    else:
        if testcase.config.localmapping.IsV6():
            return str(testcase.config.localmapping.VNIC.SUBNET.VirtualRouterIPAddr[0].packed)
        else:
            return str(testcase.config.localmapping.VNIC.SUBNET.VirtualRouterIPAddr[1])

def GetVirtualRouterMAC(testcase, pkt, args=None):
    if testcase.config.root.FwdMode == 'L3':
            return str(testcase.config.remotemapping.SUBNET.VirtualRouterMACAddr)
    return str(testcase.config.localmapping.VNIC.SUBNET.VirtualRouterMACAddr)

def __is_any_cfg_deleted(tc):
    device = tc.config.devicecfg
    lmapping = tc.config.localmapping
    rmapping = tc.config.remotemapping
    vnic = lmapping.VNIC
    subnet = vnic.SUBNET
    vpc = subnet.VPC
    routetable = tc.config.route
    if device.IsOverlayRoutingEnabled():
        objs = [device, vpc, subnet]
    else:
        objs = [device, vpc, routetable, subnet, vnic, lmapping, rmapping]
    values = list(map(lambda x: not(x.IsHwHabitant()) or (x.IsDirty()), list(filter(None, objs))))
    if any(values):
        return True
    return False

def GetExpectedPacket(testcase, args):
    #add all the cases for checking packet and return expected packet on demand
    if __is_any_cfg_deleted(testcase):
        logger.info("one or more cfgs deleted - no packet expected")
        return None
    return testcase.packets.Get(args.pkt)

def IsARPProxyNegativeTestCase(testcase):
    if "IPV4_ARP_PROXY_OUTSIDE_SUBNET" == testcase.module.name or \
        "IPV4_ARP_PROXY_NO_MAPPING" == testcase.module.name:
        logger.info("Negative test cases for ARP proxy")
        return True
    return False

def IsVRIPNegativeTestCase(tc):
    proto = __get_module_args_value(tc.module.args, 'proto')
    if proto == 'TCP' or proto == 'UDP':
        return True
    device = tc.config.devicecfg
    lmapping = tc.config.localmapping
    vnic = lmapping.VNIC
    subnet = vnic.SUBNET
    vpc = subnet.VPC
    if device.IsOverlayRoutingEnabled():
        objs = [device, vpc, subnet]
    else:
        objs = [device, vpc, subnet, vnic, lmapping ]
    values = list(map(lambda x: not(x.IsHwHabitant()) or (x.IsDirty()), list(filter(None, objs))))
    if any(values):
        return True
    return False

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

def GetRingFromMapping(testcase, args=None):
    if args.type == "remote":
        hostIf = testcase.config.remotemapping.VNIC.SUBNET.HostIf
    else:
        hostIf = testcase.config.localmapping.VNIC.SUBNET.HostIf
    return hostIf.lif.GetQt(args.qid)

def GetSrcMacInArpReply(testcase, packet, args):
    if args.type == "subnet":
        return testcase.config.localmapping.VNIC.SUBNET.VirtualRouterMACAddr
    else:
        return testcase.config.remotemapping.MACAddr

def GetEncapForARP(testcase, packet):
    vlan_encap = None
    if testcase.config.localmapping.VNIC.dot1Qenabled:
        vlan_encap = infra_api.GetPacketTemplate('ENCAP_QTAG')
    elif testcase.config.localmapping.VNIC.QinQenabled:
        vlan_encap = infra_api.GetPacketTemplate('ENCAP_QINQ')
    return [vlan_encap] if vlan_encap else []

def __get_type_val(modargs):
    return __get_module_args_value(modargs, "type")

def GetDstIpForARP(testcase, packet, args=None):
    if "IPV4_ARP_PROXY_NO_MAPPING" == testcase.module.name:
        # select a dst ip which is not in mapping table
        return testcase.config.devicecfg.IP
    elif "IPV4_ARP_PROXY_OUTSIDE_SUBNET" == testcase.module.name:
        pfx = testcase.config.localmapping.VNIC.SUBNET.IPPrefix[1]
        nextpfx = str(pfx.broadcast_address + 2)
        return nextpfx
    else:
        return testcase.config.remotemapping.IP

def GetDstMacForARP(testcase, packet, args=None):
    if "IPV4_ARP_PROXY_NO_MAPPING" == testcase.module.name:
        return testcase.config.devicecfg.MACAddr
    elif "IPV4_ARP_PROXY_OUTSIDE_SUBNET" == testcase.module.name:
        return "00:00:00:00:00:00"
    else:
        return testcase.config.remotemapping.MACAddr

def GetDstMac(testcase, packet, args=None):
    lobj = testcase.config.localmapping
    robj = testcase.config.remotemapping
    fwdmode = testcase.config.root.FwdMode
    if testcase.config.devicecfg.BridgingEnabled:
        if fwdmode == 'L2L':
            if lobj.VNIC.SUBNET.SubnetId != robj.VNIC.SUBNET.SubnetId:
                if args.direction == 'TX':
                    return robj.VNIC.SUBNET.VirtualRouterMACAddr
            return robj.VNIC.MACAddr
        else:
            if lobj.VNIC.SUBNET.SubnetId != robj.SUBNET.SubnetId:
                if args.direction == 'TX':
                    return robj.SUBNET.VirtualRouterMACAddr
                elif args.direction == 'RX':
                    return lobj.VNIC.SUBNET.VirtualRouterMACAddr
                else:
                    logger.error("Direction not provided")
                    assert(0)
            return robj.MACAddr
    else:
        if fwdmode == 'L2L':
            return robj.VNIC.MACAddr
        else:
            return robj.MACAddr

def GetSrcMac(testcase, packet, args=None):
    lobj = testcase.config.localmapping
    robj = testcase.config.remotemapping
    fwdmode = testcase.config.root.FwdMode
    if testcase.config.devicecfg.BridgingEnabled:
        if lobj.VNIC.SUBNET.SubnetId != robj.SUBNET.SubnetId:
            return lobj.VNIC.SUBNET.VirtualRouterMACAddr
    return lobj.VNIC.MACAddr

def GetVNId(testcase, args=None):
    if utils.IsBridgingEnabled(testcase.config.localmapping.Node):
        return testcase.config.localmapping.VNIC.SUBNET.Vnid
    else:
        return testcase.config.localmapping.VNIC.SUBNET.VPC.Vnid
