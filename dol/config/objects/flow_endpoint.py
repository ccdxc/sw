#! /usr/bin/python3

import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils
import infra.config.base        as base
import config.resmgr            as resmgr

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api            as halapi
import config.hal.defs           as haldefs

class FlowEndpointL4LbObject:
    def __init__(self, service):
        self.service = service
        if service is None:
            return
        self.__select_backend()
        self.__select_service_snat()
        return

    def GID(self):
        if self.service is not None:
            return self.service.GID()
        return None

    def __select_backend(self):
        self.backend = self.service.SelectBackend()
        if self.backend is None:
            assert(0)
        return defs.status.SUCCESS

    def IsTwiceNAT(self):
        if self.service is None:
            return False
        return self.service.IsTwiceNAT()

    def __select_service_snat(self):
        if self.IsTwiceNAT():
            self.service_snat_ip, self.service_snat_ipv6, self.service_snat_port = \
                self.service.SelectServiceSNat()
            print("flowepl4obj: snat_ip:%s, snat_ipv6:%s, snat_port:%d" %\
                  (self.service_snat_ip, self.service_snat_ipv6, self.service_snat_port))
            #pdb.set_trace()

    def GetServiceSNatIpAddress(self):
        return self.service_snat_ip

    def GetServiceSNatIpv6Address(self):
        return self.service_snat_ipv6

    def GetServiceSNatPort(self):
        return self.service_snat_port

    def IsServiceSNatPortValid(self):
        return self.service_snat_port != 0

    def IsEnabled(self):
        return self.service is not None

    def GetServicePort(self):
        self.port = self.service.port.get()
        return self.port

    def GetBackendPort(self):
        bport = self.backend.port.get()
        if bport == 0:
            return self.GetServicePort()
        return bport

    def IsBackendPortValid(self):
        bport = self.backend.port.get()
        return bport != 0

    def GetIpAddress(self):
        return self.backend.GetIpAddress()

    def GetIpv6Address(self):
        return self.backend.GetIpv6Address()

    def GetEp(self):
        return self.backend.ep

    def GetProto(self):
        return self.service.proto

    def GetServiceIpAddress(self):
        return self.service.vip

    def GetServiceIpv6Address(self):
        return self.service.vip6

    def GetBackendGID(self):
        return self.backend.GID()

class FlowEndpointObject(base.ConfigObjectBase):
    def __init__(self, ep = None, srcobj = None, l4lbsvc = None):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW_ENDPOINT'))
        self.type       = None
        self.dom        = None
        self.addr       = None
        self.proto      = None
        self.port       = None
        self.icmp_type  = None
        self.icmp_code  = None
        self.icmp_id    = None
        self.esp_spi    = None
        self.__flow     = None
        self.l4lb       = FlowEndpointL4LbObject(l4lbsvc)
        if self.IsL4Lb():
            self.ep     = self.l4lb.GetEp()
        else:
            self.ep     = ep

        if srcobj:
            self.__copy(srcobj)

    def __copy(self, src):
        self.ep         = src.ep
        self.type       = src.type
        self.dom        = src.dom
        self.addr       = src.addr
        self.proto      = src.proto
        self.port       = src.port
        self.icmp_type  = src.icmp_type
        self.icmp_code  = src.icmp_code
        self.icmp_id    = src.icmp_id
        self.esp_spi    = src.esp_spi
        self.l4lb       = copy.copy(src.l4lb)
        self.flowhash   = src.flowhash.get()
        return

    def __set_tcpudp_info(self, entry):
        if self.IsL4Lb():
            self.port = self.l4lb.GetServicePort()
        else:
            self.port = entry.port.get()
        if 'tracking_info' in entry.__dict__:
            self.tracking_info = objects.MergeObjects(entry.tracking_info,
                                                      self.tracking_info)
        return

    def __set_icmp_info(self, entry):
        self.icmp_type = entry.type.get()
        self.icmp_code = entry.code.get()
        self.icmp_id = entry.id.get()
        return

    def __set_esp_info(self, entry):
        self.esp_spi = entry.spi.get()
        return

    def __set_mac_info(self, entry):
        self.ethertype = entry.ethertype
        return

    def __set_info(self, entry):
        if 'flowhash' in entry.__dict__:
            self.flowhash = entry.flowhash.get()
        if self.IsIP():
            if self.IsTCP() or self.IsUDP():
                self.__set_tcpudp_info(entry)
            elif self.IsICMP() or self.IsICMPV6():
                self.__set_icmp_info(entry)
            elif self.IsESP():
                self.__set_esp_info(entry)
        elif self.IsMAC():
            self.__set_mac_info(entry)
        else:
            assert(0)
        if 'flow_info' in entry.__dict__:
            self.flow_info = objects.MergeObjects(entry.flow_info,
                                                  self.flow_info)
        return

    def SetInfo(self, entry):
        self.__set_info(entry)
        return

    def SetFlow(self, flow):
        self.__flow = flow

    def IsTCP(self):
        return self.proto == 'TCP'
    def IsUDP(self):
        return self.proto == 'UDP'
    def IsICMP(self):
        return self.proto == 'ICMP'
    def IsICMPV6(self):
        return self.proto == 'ICMPV6'
    def IsIP(self):
        return self.IsIPV4() or self.IsIPV6()
    def IsIPV4(self):
        return self.type == 'IPV4'
    def IsIPV6(self):
        return self.type == 'IPV6'
    def IsMAC(self):
        return self.type == 'MAC'
    def IsL4Lb(self):
        return self.l4lb.IsEnabled()
    def IsESP(self):
        return self.proto == 'ESP'

    def GetFlowSip(self):
        if self.IsL4Lb():
            if self.IsIPV4():
                return self.l4lb.GetIpAddress()
            elif self.IsIPV6():
                return self.l4lb.GetIpv6Address()
        return self.addr

    def GetFlowDip(self):
        return self.addr

    def GetNatSip(self):
        return self.GetFlowDip()

    def GetNatDip(self):
        return self.GetFlowSip()

    def GetFlowSport(self):
        hashgen = self.__flow.GetHashGen()
        if hashgen:
            self.port = hashgen.GetSport()
        if self.IsL4Lb():
            return self.l4lb.GetBackendPort()
        return self.port

    def GetFlowDport(self):
        hashgen = self.__flow.GetHashGen()
        if hashgen:
            self.port = hashgen.GetDport()
        return self.port

    def GetNatSport(self):
        return self.GetFlowDport()

    def IsNatPortValid(self):
        if self.IsL4Lb():
            return self.l4lb.IsBackendPortValid()
        return False

    def GetNatDport(self):
        return self.GetFlowSport()

    def IsProtoMatch(self, proto):
        if self.IsL4Lb():
            return self.l4lb.GetProto() == proto
        return True

    def GetTenant(self):
        if self.IsL4Lb():
            return self.l4lb_service.tenant
        return self.ep.tenant

    def GetTenantId(self):
        return self.ep.tenant.id

    def GetIpAddrs(self):
        if self.IsL4Lb():
            return [ self.l4lb.GetServiceIpAddress() ]
        return self.ep.ipaddrs

    def GetIpv6Addrs(self):
        if self.IsL4Lb():
            return [ self.l4lb.GetServiceIpv6Address() ]
        return self.ep.ipv6addrs

    def GetGID(self):
        if self.IsL4Lb():
            return self.l4lb.GID()
        return self.ep.GID()

    def __get_ep(self):
        return self.ep

    def GetSegment(self):
        ep = self.__get_ep()
        return ep.segment

    def GetRxQosCos(self):
        ep = self.__get_ep()
        return ep.GetRxQosCos()

    def GetRxQosDscp(self):
        ep = self.__get_ep()
        return ep.GetRxQosDscp()

    def GetTxQosCos(self):
        ep = self.__get_ep()
        return ep.GetTxQosCos()

    def GetTxQosDscp(self):
        ep = self.__get_ep()
        return ep.GetTxQosDscp()

    def Show(self, prefix):
        string = self.GetGID() + ':' + self.type + '/'
        string += "%d/%s/" % (self.dom, self.addr.get())
        if self.IsIP():
            if self.IsTCP() or self.IsUDP():
                string += "%s/" % self.proto
                string += "%d" % (self.port)
            elif self.IsICMP() or self.IsICMPV6():
                string += "%s/" % self.proto
                string += "%d/%d/%d" % (self.icmp_type, self.icmp_code, self.icmp_id)
            elif self.IsESP():
                string += "%s/" % self.proto
                string += "%s" % self.esp_spi
        elif self.IsMAC():
            string += "%04x" % self.ethertype
        cfglogger.info("- %s: %s" % (prefix, string))

        if self.IsL4Lb() == False: return

        string = self.l4lb.GetBackendGID() + ':' + self.type + '/'
        string += "%d/%s/" % (self.dom, self.l4lb.GetIpAddress().get())
        if self.IsTCP() or self.IsUDP():
            string += "%s/" % self.proto
            string += "%d" % (self.l4lb.GetBackendPort())

        cfglogger.info("  - Backend: %s" % string)
        return

