#! /usr/bin/python3

import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils
import config.resmgr            as resmgr

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api            as halapi
import config.hal.defs           as haldefs

class FlowEndpointObject:
    def __init__(self, ep = None, 
                 type = None, dom = None,
                 addr = None, proto = None):
        self.ep     = ep
        self.type   = type
        self.dom    = dom
        self.addr   = addr
        self.proto  = proto
        
        self.port = None

        self.icmp_type = None
        self.icmp_code = None
        self.icmp_id   = None
        return

    def __set_tcpudp_info(self, entry):
        self.port = entry.port.get()
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
        if self.proto == 'TCP' or self.proto == 'UDP':
            self.__set_tcpudp_info(entry)
        elif self.proto == 'ICMP' or self.proto == 'ICMPV6':
            self.__set_icmp_info(entry)
        elif self.type == 'MAC':
            self.__set_mac_info(entry)
        else:
            assert(0)
        return

    def __str__(self):
        string = self.type + '/'
        string += "%d/%s/" % (self.dom, self.addr.get())
        if self.proto == 'TCP' or self.proto == 'UDP':
            string += "%s/" % self.proto
            string += "%d" % (self.port)
        elif self.proto == 'ICMP' or self.proto == 'ICMPV6':
            string += "%s/" % self.proto
            string += "%d/%d/%d" % (self.icmp_type, self.icmp_code, self.icmp_id)
        elif self.type == 'MAC':
            string += "%04x" % self.ethertype
        return string

