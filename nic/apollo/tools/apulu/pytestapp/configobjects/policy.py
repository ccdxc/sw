#! /usr/bin/python3
import pdb
import ipaddress as ipaddress

import types_pb2 as types_pb2
import policy_pb2 as policy_pb2
import utils

PROTO_TCP = 6
PROTO_UDP = 17

protos = {PROTO_TCP, PROTO_UDP}

class PolicyObject():
    def __init__(self, id, af, direction, rules):
        self.id = id
        self.af       = af
        self.direction = direction
        self.rules = rules
        return

    def FillRule(self, spec, rule):
        specrule = spec.Rules.add()
        specrule.Stateful = rule.Stateful
        specrule.Action = rule.Action
        if rule.L4Match:
            specrule.Match.L4Match.Ports.SrcPortRange.PortLow = rule.L4SportLow
            specrule.Match.L4Match.Ports.SrcPortRange.PortHigh = rule.L4SportHigh
            specrule.Match.L4Match.Ports.DstPortRange.PortLow = rule.L4DportLow
            specrule.Match.L4Match.Ports.DstPortRange.PortHigh = rule.L4DportHigh
        if rule.L3Match:
            specrule.Match.L3Match.Protocol = rule.Proto
            if self.direction == types_pb2.RULE_DIR_EGRESS:
                specrule.Match.L3Match.DstPrefix.Len = rule.Prefix.prefixlen
                specrule.Match.L3Match.DstPrefix.Addr.Af = rule.af
                if rule.af == types_pb2.IP_AF_INET:
                    specrule.Match.L3Match.DstPrefix.Addr.V4Addr = int(rule.Prefix.network_address)
                elif rule.af == types_pb2.IP_AF_INET6:
                    specrule.Match.L3Match.DstPrefix.Addr.V6Addr = rule.Prefix.network_address.packed
            elif self.direction == types_pb2.RULE_DIR_INGRESS:
                specrule.Match.L3Match.SrcPrefix.Len = rule.Prefix.prefixlen
                specrule.Match.L3Match.SrcPrefix.Addr.Af = rule.af
                if rule.af == types_pb2.IP_AF_INET:
                    specrule.Match.L3Match.SrcPrefix.Addr.V4Addr = int(rule.Prefix.network_address)
                elif rule.af == types_pb2.IP_AF_INET6:
                    specrule.Match.L3Match.SrcPrefix.Addr.V6Addr = rule.Prefix.network_address.packed

    def GetGrpcCreateMessage(self):
        grpcmsg = policy_pb2.SecurityPolicyRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.id
        spec.Direction = self.direction
        spec.AddrFamily = self.af
        for rule in self.rules:
            self.FillRule(spec, rule)
        return grpcmsg

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
