#! /usr/bin/python3
import pdb
import utils
import re

import dhcp_pb2 as dhcp_pb2
import types_pb2 as types_pb2

class DhcpPolicyObject():
    def __init__(self, id, server_ip=None, mtu=None, gateway_ip=None, dns_server=None, ntp_server=None, domain_name=None, lease_timeout=None):
        super().__init__()
        self.id = id
        self.uuid = utils.PdsUuid(self.id)
        self.server_ip = server_ip
        self.mtu = mtu
        self.gateway_ip = gateway_ip
        self.dns_server = dns_server
        self.ntp_server = ntp_server
        self.domain_name = domain_name
        self.lease_timeout = lease_timeout

        return

    def GetGrpcCreateMessage(self):
        grpcmsg = dhcp_pb2.DHCPPolicyRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        if self.server_ip:
            spec.ServerIP.Af = types_pb2.IP_AF_INET
            spec.ServerIP.V4Addr = int(self.server_ip)
        if self.mtu:
            spec.MTU = self.mtu
        if self.gateway_ip:
            spec.GatewayIP.Af = types_pb2.IP_AF_INET
            spec.GatewayIP.V4Addr = int(self.gateway_ip)
        if self.dns_server:
            spec.DNSServerIP.Af = types_pb2.IP_AF_INET
            spec.DNSServerIP.V4Addr = int(self.dns_server)
        if self.ntp_server:
            spec.NTPServerIP.Af = types_pb2.IP_AF_INET
            spec.NTPServerIP.V4Addr = int(self.ntp_server)
        if self.domain_name:
            spec.DomainName = self.domain_name
        if self.lease_timeout:
            spec.LeaseTimeout = self.lease_timeout

        return grpcmsg

