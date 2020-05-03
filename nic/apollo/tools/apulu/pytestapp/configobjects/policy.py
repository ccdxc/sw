#! /usr/bin/python3
import pdb

import types_pb2 as types_pb2
import policy_pb2 as policy_pb2
import ipaddress as ipaddress
import utils

PROTO_TCP = 6
PROTO_UDP = 17

protos = {PROTO_TCP, PROTO_UDP}

class PolicyObject():
    def __init__(self, id, af, direction, rules, default_action=types_pb2.SECURITY_RULE_ACTION_ALLOW):
        self.id = id
        self.uuid     = utils.PdsUuid(self.id)
        self.af       = af
        self.direction = direction
        self.rules = rules
        self.default_action = default_action
        return

    def FillRule(self, spec, rule):
        specrule = spec.Rules.add()
        specrule.Attrs.Stateful = rule.Stateful
        specrule.Attrs.Action = rule.Action
        if rule.L4Match:
            specrule.Attrs.Match.L4Match.Ports.SrcPortRange.PortLow = rule.L4SportLow
            specrule.Attrs.Match.L4Match.Ports.SrcPortRange.PortHigh = rule.L4SportHigh
            specrule.Attrs.Match.L4Match.Ports.DstPortRange.PortLow = rule.L4DportLow
            specrule.Attrs.Match.L4Match.Ports.DstPortRange.PortHigh = rule.L4DportHigh
        if rule.L3Match:
            specrule.Attrs.Match.L3Match.ProtoNum = rule.Proto
            if self.direction == types_pb2.RULE_DIR_EGRESS:
                specrule.Attrs.Match.L3Match.DstPrefix.Len = rule.Prefix.prefixlen
                specrule.Attrs.Match.L3Match.DstPrefix.Addr.Af = rule.af
                if rule.af == types_pb2.IP_AF_INET:
                    specrule.Attrs.Match.L3Match.DstPrefix.Addr.V4Addr = int(rule.Prefix.network_address)
                elif rule.af == types_pb2.IP_AF_INET6:
                    specrule.Attrs.Match.L3Match.DstPrefix.Addr.V6Addr = rule.Prefix.network_address.packed
            elif self.direction == types_pb2.RULE_DIR_INGRESS:
                specrule.Attrs.Match.L3Match.SrcPrefix.Len = rule.Prefix.prefixlen
                specrule.Attrs.Match.L3Match.SrcPrefix.Addr.Af = rule.af
                if rule.af == types_pb2.IP_AF_INET:
                    specrule.Attrs.Match.L3Match.SrcPrefix.Addr.V4Addr = int(rule.Prefix.network_address)
                elif rule.af == types_pb2.IP_AF_INET6:
                    specrule.Attrs.Match.L3Match.SrcPrefix.Addr.V6Addr = rule.Prefix.network_address.packed

    def GetGrpcCreateMessage(self):
        grpcmsg = policy_pb2.SecurityPolicyRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        spec.DefaultFWAction = self.default_action
        spec.AddrFamily = self.af
        for rule in self.rules:
            self.FillRule(spec, rule)
        return grpcmsg
    
class rule_obj:
    def __init__(self, af, stateful, action, l3match, prefix, proto, l4match, l4sportlow = 0, l4sporthigh = 65535, l4dportlow = 0, l4dporthigh = 65535):
        self.af = af
        self.Stateful = stateful
        self.Action = action
        self.L3Match = l3match
        self.Prefix = prefix
        self.Proto = proto
        self.L4Match = l4match
        self.L4SportLow = l4sportlow
        self.L4SportHigh = l4sporthigh
        self.L4DportLow = l4dportlow
        self.L4DportHigh = l4dporthigh

class SecurityProfileObject():
    def __init__(self, id, tcp_idle_timeout, udp_idle_timeout, icmp_idle_timeout):
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.tcp_idle_timeout = tcp_idle_timeout
        self.udp_idle_timeout = udp_idle_timeout
        self.icmp_idle_timeout = icmp_idle_timeout
        return

    def GetGrpcCreateMessage(self):
        grpcmsg = policy_pb2.SecurityProfileRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()

        spec.ConnTrackEn = False
        spec.DefaultFWAction = types_pb2.SECURITY_RULE_ACTION_ALLOW
        spec.TCPIdleTimeout = self.tcp_idle_timeout
        spec.UDPIdleTimeout = self.udp_idle_timeout
        spec.ICMPIdleTimeout = self.icmp_idle_timeout
        spec.OtherIdleTimeout = 100
        spec.TCPCnxnSetupTimeout = 100
        spec.TCPHalfCloseTimeout = 100
        spec.TCPCloseTimeout = 100
        spec.TCPDropTimeout = 100
        spec.UDPDropTimeout = 100
        spec.ICMPDropTimeout = 100
        spec.OtherDropTimeout = 100
        return grpcmsg
