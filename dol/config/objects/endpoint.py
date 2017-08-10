#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.enic      as enic

import config.hal.api           as halapi
import config.hal.defs          as haldefs

from config.store               import Store
from infra.common.logging       import cfglogger

class EndpointObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('ENDPOINT'))
        self.id = resmgr.EpIdAllocator.get()
        self.GID("EP%04d" % self.id)
        return

    def Init(self, segment):
        self.segment    = segment
        self.local      = True
        self.tenant     = segment.tenant
        self.segment    = segment
        self.remote     = False

        num_ip_addrs = defs.HAL_NUM_IPADDRS_PER_ENDPOINT
        num_ipv6_addrs = defs.HAL_NUM_IPV6ADDRS_PER_ENDPOINT
        if segment.IsInfraSegment():
            num_ip_addrs = 1
            num_ipv6_addrs = 1

        self.ipaddrs = []
        for ipidx in range(num_ip_addrs):
            ipaddr = segment.ip_addr_allocator.get()
            self.ipaddrs.append(ipaddr)

        self.ipv6addrs = []
        for ipidx in range(num_ip_addrs):
            ipv6addr   = segment.ipv6_addr_allocator.get()
            self.ipv6addrs.append(ipv6addr)

        self.security_groups = []

        self.useg_vlan_id = 0
        #self.attachment = self.tenant.spec.endpoints.attachment[0].entry.type
        return

    def SetRemote(self):
        self.remote = True

    def GetIpAddress(self, idx = 0):
        return self.ipaddrs[idx]
    def GetIpv6Address(self, idx = 0):
        return self.ipv6addrs[idx]
    def GetInterface(self):
        return self.intf

    def AttachInterface(self, intf):
        self.intf       = intf
        if self.remote:
            self.macaddr    = resmgr.RemoteEpMacAllocator.get()
        else:
            self.macaddr    = intf.macaddr

        cfglogger.info("- Endpoint = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("  - Tenant    = %s" % self.tenant)
        cfglogger.info("  - Macaddr   = %s" % self.macaddr.get())
        cfglogger.info("  - Interface = %s" % self.intf.GID())
        for ipaddr in self.ipaddrs:
            cfglogger.info("  - Ipaddr    = %s" % ipaddr.get())
        for ipv6addr in self.ipv6addrs:
            cfglogger.info("  - Ipv6addr  = %s" % ipv6addr.get())
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.tenant_id     = self.tenant.id
        req_spec.l2_segment_handle  = self.segment.hal_handle
        req_spec.interface_handle   = self.intf.hal_handle
        req_spec.mac_address        = self.macaddr.getnum()

        for ipaddr in self.ipaddrs:
            ip = req_spec.ip_address.add() 
            ip.ip_af = haldefs.common.IP_AF_INET
            ip.v4_addr = ipaddr.getnum()

        for ipv6addr in self.ipv6addrs:
            ip = req_spec.ip_address.add() 
            ip.ip_af = haldefs.common.IP_AF_INET6
            ip.v6_addr = ipv6addr.getnum().to_bytes(16, 'big')
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.endpoint_status.endpoint_handle
        cfglogger.info("  - Endpoint %s = %s (HDL = 0x%x)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Endpoint Objects.
class EndpointObjectHelper:
    def __init__(self):
        self.direct     = []
        self.useg       = []
        self.pvlan      = []
        self.local      = []
        self.remote     = []
        self.eps        = []
        return

    def Show(self):
        cfglogger.info("  - # EP: Dir=%d Useg=%d Pvlan=%d Loc=%d Rem=%d Tot=%d" %\
                       (len(self.direct), len(self.useg), len(self.pvlan),
                        len(self.local), len(self.remote), len(self.eps)))
        return

    def Configure(self):
        cfglogger.info("Configuring %d Endpoints." % len(self.eps))
        halapi.ConfigureEndpoints(self.eps)
        return

    def __create(self, segment, intfs, count, remote = False):
        eps = []
        for e in range(count):
            intf = intfs[e % len(intfs)]
            ep = EndpointObject()
            # Setup EP <--> ENIC association
            ep.Init(segment)
            if remote:
                ep.SetRemote()
            ep.AttachInterface(intf)
            if not remote:
                intf.AttachEndpoint(ep)
            eps.append(ep)
        return eps

    def __create_remote(self, segment, spec):
        if spec.remote:
            cfglogger.info("Creating %d REMOTE EPs in Segment = %s" %\
                           (spec.remote, segment.GID()))
            uplinks = Store.trunk_uplinks.GetAll()
            self.remote = self.__create(segment, uplinks, spec.remote,
                                        remote = True)
        return

    def __create_local(self, segment, spec):
        if spec.direct:
            cfglogger.info("Creating %d DIRECT EPs in Segment = %s" %\
                           (spec.direct, segment.GID()))
            direct_enics = segment.GetDirectEnics()
            self.direct = self.__create(segment, direct_enics, spec.direct)
            self.local  += self.direct

        if spec.pvlan:
            cfglogger.info("Creating %d PVLAN EPs in Segment = %s" %\
                           (spec.pvlan, segment.GID()))
            pvlan_enics = segment.GetPvlanEnics()
            self.pvlan  = self.__create(segment, pvlan_enics, spec.pvlan)
            self.local  += self.pvlan

        if spec.useg:
            cfglogger.info("Creating %d USEG EPs in Segment = %s" %\
                           (spec.useg, segment.GID()))
            useg_enics = segment.GetUsegEnics()
            self.useg  = self.__create(segment, useg_enics, spec.useg)
            self.local  += self.useg
        return
         
    def Generate(self, segment, spec):
        self.__create_remote(segment, spec)
        self.__create_local(segment, spec)
        self.eps += self.local
        self.eps += self.remote
        if len(self.eps):
            Store.objects.SetAll(self.eps)
        return
