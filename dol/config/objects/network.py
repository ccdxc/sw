#! /usr/bin/python3
import pdb
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from infra.common.logging       import cfglogger
from config.store               import Store

class NetworkObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SEGMENT'))
        return

    def Init(self, segment, prefix, af):
        self.segment = segment
        self.id = self.segment.id
        self.GID("Nw%04d" % self.segment.id)
    
        self.af = af
        self.prefix = prefix
        if self.IsIpv4():
            self.prefix_len = 24
        else:
            self.prefix_len = 96
        self.rmac = segment.macaddr
        self.hal_handle = None

        self.Show()
        return

    def IsIpv4(self):
        return self.af == 'IPV4'
    def IsIpv6(self):
        return self.af == 'IPV6'

    def Show(self, detail = False):
        cfglogger.info("- Network = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("  - RMAC       = %s" % self.rmac.get())
        cfglogger.info("  - Prefix     = %s/%d" %\
                       (self.prefix.get(), self.prefix_len))
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.tenant_id = self.segment.tenant.id
        req_spec.rmac = self.rmac.getnum() 
        if self.IsIpv4():
            req_spec.key_or_handle.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET
            req_spec.key_or_handle.ip_prefix.address.v4_addr = self.prefix.getnum()
            req_spec.key_or_handle.ip_prefix.prefix_len = self.prefix_len
        else:
            req_spec.key_or_handle.ip_prefix.address.ip_af = haldefs.common.IP_AF_INET6
            req_spec.key_or_handle.ip_prefix.address.v6_addr = self.prefix.getnum().to_bytes(16, 'big')
            req_spec.key_or_handle.ip_prefix.prefix_len = self.prefix_len
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("- Network %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.hal_handle = resp_spec.status.nw_handle
        return
    
    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Network Objects.
class NetworkObjectHelper:
    def __init__(self):
        self.nws = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Networks -- DISABLED" % len(self.nws)) 
        return
        halapi.ConfigureNetworks(self.nws)
        return

    def Generate(self, segment):
        cfglogger.info("Creating Network Objects for Segment = %s" %\
                       (segment.GID()))
        nw = NetworkObject()
        nw.Init(segment, segment.subnet, 'IPV4')
        self.nws.append(nw)

        nw = NetworkObject()
        nw.Init(segment, segment.subnet6, 'IPV6')
        self.nws.append(nw)
        return
