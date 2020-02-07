#! /usr/bin/python3
import pdb

import types_pb2 as types_pb2
import policy_pb2 as policy_pb2
import ipaddress as ipaddress

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

