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

class FlowEndpointObject(base.ConfigObjectBase):
    def __init__(self, ep = None, srcobj = None, l4lbsvc = None):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW_ENDPOINT'))
        self.ep         = ep
        self.type       = None
        self.dom        = None
        self.addr       = None
        self.proto      = None
        self.port       = None
        self.icmp_type  = None
        self.icmp_code  = None
        self.icmp_id    = None
        self.l4lb_service = l4lbsvc
        self.l4lb_backend = None
        if srcobj:
            self.__copy(srcobj)

    def SelectL4LbBackend(self):
        if self.IsL4LbServiceFlowEp():
            self.l4lb_backend = self.l4lb_service.SelectBackend()
            if self.l4lb_backend == None:
                return defs.status.ERROR
        return defs.status.SUCCESS

    def __copy(self, src):
        self.ep     = src.ep
        self.type   = src.type
        self.dom    = src.dom
        self.addr   = src.addr
        self.proto  = src.proto
        self.port   = src.port
        self.icmp_type  = src.icmp_type
        self.icmp_code  = src.icmp_code
        self.icmp_id    = src.icmp_id
        self.l4lb_service = src.l4lb_service
        self.l4lb_backend = src.l4lb_backend
        return

    def __set_tcpudp_info(self, entry):
        if self.IsL4LbServiceFlowEp():
            self.port = self.l4lb_service.port.get()
        else:
            self.port = entry.port.get()

        if 'flow_info' in entry.__dict__:
            self.flow_info = objects.MergeObjects(entry.flow_info,
                                                  self.flow_info)
        if 'tracking_info' in entry.__dict__:
            self.tracking_info = objects.MergeObjects(entry.tracking_info,
                                                      self.tracking_info)
        return

    def __set_icmp_info(self, entry):
        self.icmp_type = entry.type.get()
        self.icmp_code = entry.code.get()
        self.icmp_id = entry.id.get()
        return

    def __set_mac_info(self, entry):
        self.ethertype = entry.ethertype
        return

    def __set_info(self, entry):
        if self.IsIP():
            if self.IsTCP() or self.IsUDP():
                self.__set_tcpudp_info(entry)
            elif self.IsICMP() or self.IsICMPV6():
                self.__set_icmp_info(entry)
        elif self.IsMAC():
            self.__set_mac_info(entry)
        else:
            assert(0)
        return

    def SetInfo(self, entry):
        self.__set_info(entry)
        return

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
    def IsL4LbServiceFlowEp(self):
        return self.l4lb_service != None

    def GetFlowSip(self):
        if self.IsL4LbServiceFlowEp():
            if self.IsIPV4():
                return self.l4lb_backend.GetIpAddress()
            elif self.IsIPV6():
                return self.l4lb_backend.GetIpv6Address()
        return self.addr

    def GetFlowDip(self):
        return self.addr

    def GetNatSip(self):
        return self.GetFlowDip()

    def GetNatDip(self):
        return self.GetFlowSip()

    def GetFlowSport(self):
        if self.IsL4LbServiceFlowEp():
            return self.l4lb_backend.port.get()
        return self.port

    def GetFlowDport(self):
        return self.port

    def GetNatSport(self):
        return self.GetFlowDport()

    def GetNatDport(self):
        return self.GetFlowSport()

    def IsProtoMatch(self, proto):
        if self.IsL4LbServiceFlowEp():
            return self.l4lb_service.proto == proto
        return True

    def GetTenantId(self):
        if self.IsL4LbServiceFlowEp():
            return self.l4lb_service.tenant.id
        return self.ep.tenant.id

    def GetSegment(self):
        if self.IsL4LbServiceFlowEp():
            return self.l4lb_backend.ep.segment
        return self.ep.segment

    def GetIpAddrs(self):
        if self.IsL4LbServiceFlowEp():
            return [ self.l4lb_service.vip ]
        return self.ep.ipaddrs

    def GetIpv6Addrs(self):
        if self.IsL4LbServiceFlowEp():
            return [ self.l4lb_service.vip6 ]
        return self.ep.ipv6addrs

    def GetGID(self):
        if self.IsL4LbServiceFlowEp():
            return self.l4lb_service.GID()
        return self.ep.GID()

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
        elif self.IsMAC():
            string += "%04x" % self.ethertype
        cfglogger.info("- %s: %s" % (prefix, string))
        
        if self.IsL4LbServiceFlowEp() == False: return
        
        string = self.l4lb_backend.GID() + ':' + self.type + '/'
        string += "%d/%s/" % (self.dom, self.l4lb_backend.GetIpAddress().get())
        if self.IsTCP() or self.IsUDP():
            string += "%s/" % self.proto
            string += "%d" % (self.l4lb_backend.port.get())

        cfglogger.info("  - Backend: %s" % string)
        return

