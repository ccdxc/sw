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

#import config.objects.qp        as qp
import config.objects.pd        as pd
import config.objects.slab      as slab

class EndpointObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('ENDPOINT'))
        self.id = resmgr.EpIdAllocator.get()
        self.GID("EP%04d" % self.id)
        self.ipaddrs = []
        self.ipv6addrs = []
        self.security_groups = []
        return

    def Init(self, segment, backend):
        self.segment    = segment
        self.local      = True
        self.tenant     = segment.tenant
        self.segment    = segment
        self.remote     = False

        self.is_l4lb_backend = backend
        self.is_l4lb_service = False
        self.l4lb_backend = None

        num_ip_addrs = defs.HAL_NUM_IPADDRS_PER_ENDPOINT
        num_ipv6_addrs = defs.HAL_NUM_IPV6ADDRS_PER_ENDPOINT
        if segment.IsInfraSegment():
            num_ip_addrs = 1
            num_ipv6_addrs = 1

        for ipidx in range(num_ip_addrs):
            ipaddr = segment.AllocIpv4Address(self.IsL4LbBackend())
            self.SetIpAddress(ipaddr)

        for ipidx in range(num_ipv6_addrs):
            ipv6addr = segment.AllocIpv6Address(self.IsL4LbBackend())
            self.SetIpv6Address(ipv6addr)

        self.useg_vlan_id = 0
        return

    def IsL4LbBackend(self):
        return self.is_l4lb_backend

    def IsL4LbService(self):
        return self.is_l4lb_service

    def SetL4LbService(self):
        self.is_l4lb_service = True
        return

    def AttachL4LbBackend(self, backend):
        self.l4lb_backend = backend
        return

    def SetRemote(self):
        self.remote = True
        return

    def SetMacAddress(self, mac):
        self.macaddr = mac
        return

    def SetIpAddress(self, ip):
        self.ipaddrs.append(ip)
        return
    def GetIpAddress(self, idx = 0):
        return self.ipaddrs[idx]

    def SetIpv6Address(self, ip6):
        self.ipv6addrs.append(ip6)
        return
    def GetIpv6Address(self, idx = 0):
        return self.ipv6addrs[idx]

    def GetInterface(self):
        return self.intf

    def AttachInterface(self, intf):
        self.intf       = intf
        if self.remote:
            self.SetMacAddress(resmgr.RemoteEpMacAllocator.get())
        else:
            self.SetMacAddress(intf.macaddr)
        self.Show()
        return
    
    def Show(self):
        cfglogger.info("- Endpoint = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("  - IsBackend = %s" % self.IsL4LbBackend())
        cfglogger.info("  - Tenant    = %s" % self.tenant)
        cfglogger.info("  - Macaddr   = %s" % self.macaddr.get())
        cfglogger.info("  - Interface = %s" % self.intf.GID())
        for ipaddr in self.ipaddrs:
            cfglogger.info("  - Ipaddr    = %s" % ipaddr.get())
        for ipv6addr in self.ipv6addrs:
            cfglogger.info("  - Ipv6addr  = %s" % ipv6addr.get())
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/Mac:%s' % self.macaddr.get()
        summary += '/Remote:%s' % self.remote
        summary += '/Bkend:%s' % self.IsL4LbBackend()
        return summary

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
        cfglogger.info("- Endpoint %s = %s (HDL = 0x%x)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def CreatePds(self, spec):
        self.pds = objects.ObjectDatabase(cfglogger)
        self.obj_helper_pd = pd.PdObjectHelper()
        self.obj_helper_pd.Generate(self, spec) 
        self.pds.SetAll(self.obj_helper_pd.pds)

    def ConfigurePds(self):
        self.obj_helper_pd.Configure()

    def CreateSlabs(self, spec):
        self.slab_allocator = objects.TemplateFieldObject("range/0/1024")
        self.slabs = objects.ObjectDatabase(cfglogger)
        self.obj_helper_slab = slab.SlabObjectHelper()
        self.obj_helper_slab.Generate(self, spec)
        self.slabs.SetAll(self.obj_helper_slab.slabs)

    def AddSlab(self, slab):
        self.obj_helper_slab.AddSlab(slab)
        self.slabs.Add(slab)

    def ConfigureSlabs(self):
        self.obj_helper_slab.Configure() 
        
    def GetSlabid(self):
        return self.slab_allocator.get()

# Helper Class to Generate/Configure/Manage Endpoint Objects.
class EndpointObjectHelper:
    def __init__(self):
        self.direct     = []
        self.useg       = []
        self.pvlan      = []
        self.local      = []
        self.remote     = []
        self.eps        = []

        self.backend_direct     = []
        self.backend_useg       = []
        self.backend_pvlan      = []
        self.backend_local      = []
        self.backend_remote     = []
        self.backend_eps        = []
        return

    def Show(self):
        cfglogger.info("  - # EP: Dir=%d Useg=%d Pvlan=%d Loc=%d Rem=%d Tot=%d" %\
                       (len(self.direct), len(self.useg), len(self.pvlan),
                        len(self.local), len(self.remote), len(self.eps)))
        return

    def Configure(self):
        cfglogger.info("Configuring %d Endpoints." % len(self.eps))
        halapi.ConfigureEndpoints(self.eps)
        halapi.ConfigureEndpoints(self.backend_eps)

        if self.rdma:
            for ep in self.local:
                ep.ConfigureSlabs()
            for ep in self.eps:
                ep.ConfigurePds()
        return

    def __create(self, segment, intfs, count,
                 remote = False, backend = False):
        eps = []
        #if count > len(intfs):
        #    count = len(intfs)
        for e in range(count):
            intf = intfs[e % len(intfs)]
            ep = EndpointObject()
            # Setup EP <--> ENIC association
            ep.Init(segment, backend)
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
            if segment.IsFabEncapVlan():
                remote_intfs = Store.GetTrunkingUplinks()
            elif segment.IsFabEncapVxlan():
                remote_intfs = Store.GetTunnelsVxlan()
            self.remote = self.__create(segment, remote_intfs,
                                        spec.remote,
                                        remote = True,
                                        backend = False)
            if segment.l4lb:
                cfglogger.info("Creating %d REMOTE L4LB Backend EPs in Segment = %s" %\
                               (spec.remote, segment.GID()))
                self.backend_remote = self.__create(segment, remote_intfs,
                                                    spec.remote,
                                                    remote = True,
                                                    backend = True)
        return

    def __create_local(self, segment, spec):
        if spec.direct:
            cfglogger.info("Creating %d DIRECT EPs in Segment = %s" %\
                           (spec.direct, segment.GID()))
            direct_enics = segment.GetDirectEnics()
            self.direct = self.__create(segment, direct_enics, spec.direct)
            self.local += self.direct

            if segment.l4lb:
                cfglogger.info("Creating %d DIRECT L4LB Backend EPs in Segment = %s" %\
                               (spec.direct, segment.GID()))
                direct_enics = segment.GetDirectEnics(backend = True)
                self.backend_direct = self.__create(segment, direct_enics,
                                                    spec.direct,
                                                    backend = True)
                self.backend_local += self.backend_direct

        if spec.pvlan:
            cfglogger.info("Creating %d PVLAN EPs in Segment = %s" %\
                           (spec.pvlan, segment.GID()))
            pvlan_enics = segment.GetPvlanEnics()
            self.pvlan = self.__create(segment, pvlan_enics, spec.pvlan)
            self.local  += self.pvlan

            if segment.l4lb:
                cfglogger.info("Creating %d PVLAN L4LB Backend EPs in Segment = %s" %\
                               (spec.pvlan, segment.GID()))
                pvlan_enics = segment.GetPvlanEnics(backend = True)
                self.backend_pvlan = self.__create(segment, pvlan_enics,
                                                   spec.pvlan,
                                                   backend = True)
                self.backend_local += self.backend_pvlan

        if spec.useg:
            cfglogger.info("Creating %d USEG EPs in Segment = %s" %\
                           (spec.useg, segment.GID()))
            useg_enics = segment.GetUsegEnics()
            self.useg = self.__create(segment, useg_enics, spec.useg)
            self.local += self.useg

            if segment.l4lb:
                cfglogger.info("Creating %d USEG L4LB Backend EPs in Segment = %s" %\
                               (spec.useg, segment.GID()))
                useg_enics = segment.GetUsegEnics(backend = True)
                self.backend_useg = self.__create(segment, useg_enics,
                                                  spec.useg,
                                                  backend = True)
                self.backend_local += self.backend_useg
        return

    def __create_pds(self, spec):
        for ep in self.eps:
            ep.CreatePds(spec)
    
    def __create_slabs(self, spec):
        # create slabs only on local eps
        for ep in self.local:
            ep.CreateSlabs(spec)
    
    def Generate(self, segment, spec):
        self.__create_remote(segment, spec)
        self.__create_local(segment, spec)
        self.eps += self.local
        self.eps += self.remote

        self.backend_eps += self.backend_local
        self.backend_eps += self.backend_remote
        if len(self.eps):
            Store.objects.SetAll(self.eps)
        if len(self.backend_eps):
            Store.objects.SetAll(self.backend_eps)

        self.rdma = getattr(spec, 'rdma', False)

        if self.rdma:
            if spec.rdma.slab:
                slab_spec = spec.rdma.slab.Get(Store)
                self.__create_slabs(slab_spec)

            if spec.rdma.pd:
                pd_spec = spec.rdma.pd.Get(Store)
                self.__create_pds(pd_spec)

        return
