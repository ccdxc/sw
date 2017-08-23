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
    def __init__(self, ep = None, 
                 type = None, dom = None,
                 addr = None, proto = None):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW_ENDPOINT'))
        self.ep         = ep
        self.type       = type
        self.dom        = dom
        self.addr       = addr
        self.proto      = proto
        self.port       = None
        self.icmp_type  = None
        self.icmp_code  = None
        self.icmp_id    = None
        return

    def __set_tcpudp_info(self, entry):
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

    def SetInfo(self, entry):
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

    def __str__(self):
        string = self.type + '/'
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
        return string

