#! /usr/bin/python3
import utils
import re

import dhcp_pb2 as dhcp_pb2
import types_pb2 as types_pb2

class DhcpPolicyObject():
    def __init__(self, id, server_ip=None, mtu=None, gateway_ip=None, dns_server=None, ntp_server=None, domain_name=None, lease_timeout=None, boot_filename=None):
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
        self.boot_filename = boot_filename

        return

    def GetGrpcCreateMessage(self):
        grpcmsg = dhcp_pb2.DHCPPolicyRequest()
        spec = grpcmsg.Request.add()
        spec.Id = self.uuid.GetUuid()
        if self.server_ip:
            spec.ProxySpec.ServerIP.Af = types_pb2.IP_AF_INET
            spec.ProxySpec.ServerIP.V4Addr = int(self.server_ip)
        if self.mtu:
            spec.ProxySpec.MTU = self.mtu
        if self.gateway_ip:
            spec.ProxySpec.GatewayIP.Af = types_pb2.IP_AF_INET
            spec.ProxySpec.GatewayIP.V4Addr = int(self.gateway_ip)
        if self.dns_server:
            spec.ProxySpec.DNSServerIP.Af = types_pb2.IP_AF_INET
            spec.ProxySpec.DNSServerIP.V4Addr = int(self.dns_server)
        if self.ntp_server:
            spec.ProxySpec.NTPServerIP.Af = types_pb2.IP_AF_INET
            spec.ProxySpec.NTPServerIP.V4Addr = int(self.ntp_server)
        if self.domain_name:
            spec.ProxySpec.DomainName = self.domain_name
        if self.lease_timeout:
            spec.ProxySpec.LeaseTimeout = self.lease_timeout
        if self.boot_filename:
            spec.ProxySpec.BootFileName = self.boot_filename

        return grpcmsg

