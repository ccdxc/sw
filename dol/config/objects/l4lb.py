#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr
import config.objects.endpoint  as endpoint

from infra.common.logging       import cfglogger
from config.store               import Store

import config.hal.api            as halapi
import config.hal.defs           as haldefs

ENIC_TYPE_DIRECT= 'DIRECT'
ENIC_TYPE_USEG  = 'USEG'
ENIC_TYPE_PVLAN = 'PVLAN'

class L4LbBackendObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.id = resmgr.L4LbBackendIdAllocator.get()
        self.GID("L4LbBackend%04d" % self.id)
        return

    def Init(self, service, spec):
        self.service    = service
        self.port       = spec.port
        self.remote     = spec.remote
        self.ep         = service.tenant.AllocL4LbBackend()
        self.ep.AttachL4LbBackend(self)
        return

    def GetIpAddress(self):
        return self.ep.GetIpAddress()
    def GetIpv6Address(self):
        return self.ep.GetIpv6Address()

    def Show(self):
        cfglogger.info("- Backend = %s  %s:%d" %\
                       (self.GID(), self.ep.GID(), self.port.get()))
        return

class L4LbBackendObjectHelper:
    def __init__(self):
        self.bends  = []
        return

    def Configure(self):
        return

    def Generate(self, service, bspec_list):
        for bspec in bspec_list:
            count = bspec.count.get()
            for c in range(count):
                bend = L4LbBackendObject()
                bend.Init(service, bspec)
                self.bends.append(bend)
        return
        

class L4LbServiceObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.id = resmgr.L4LbServiceIdAllocator.get()
        self.GID("L4LbService%04d" % self.id)
        return

    def Init(self, tenant, spec):
        self.tenant     = tenant
        self.spec       = spec
        self.label      = spec.label.upper()
        self.proto      = spec.proto.upper()
        
        self.vip        = resmgr.L4LbServiceIpAllocator.get()
        self.vip6       = resmgr.L4LbServiceIpv6Allocator.get()
        self.vmac       = resmgr.L4LbVMacAllocator.get()
        
        self.port       = spec.port
        self.mode       = spec.mode.upper()
        self.snat_ips   = []
        self.snat_ports = []
        if self.IsTwiceNAT():
            for c in range(spec.snat_ips.GetCount()):
                sip = spec.snat_ips.get()
                self.snat_ips.append(sip)
                sport = spec.snat_ports.get()
                self.snat_ports.append(sport)

        self.bend_idx = 0
        self.obj_helper_bend = L4LbBackendObjectHelper()
        self.obj_helper_bend.Generate(self, spec.backends)
       
        self.__create_ep()
        self.Show()
        return

    def IsNAT(self):
        return self.mode == 'NAT'
    def IsTwiceNAT(self):
        return self.mode == 'TWICE_NAT'

    def SelectBackend(self):
        if self.bend_idx >= len(self.obj_helper_bend.bends):
            return None
        bend = self.obj_helper_bend.bends[self.bend_idx]
        self.bend_idx += 1
        return bend

    def __create_ep(self):
        # Create a dummy endpoint for the service
        self.ep = endpoint.EndpointObject()
        self.ep.macaddr = self.vmac
        self.ep.SetIpAddress(self.vip)
        self.ep.SetIpv6Address(self.vip6)
        self.ep.SetL4LbService()
        self.ep.tenant = self.tenant
        return


    def Show(self):
        cfglogger.info("L4LbService: %s" % self.GID())
        cfglogger.info("- Label   = %s" % self.label)
        cfglogger.info("- Mode    = %s" % self.mode)
        cfglogger.info("- Tenant  = %s" % self.tenant.GID())
        cfglogger.info("- Proto   = %s" % self.proto)
        cfglogger.info("- VIP     = %s" % self.vip.get())
        cfglogger.info("- VMac    = %s" % self.vmac.get())
        cfglogger.info("- Port    = %s" % self.port.get())
        for s in range(len(self.snat_ips)):
            cfglogger.info("- SNAT Ipaddr:Port = %s:%d" %\
                           (self.snat_ips[s].get(), self.snat_ports[s]))
        for bend in self.obj_helper_bend.bends:
            bend.Show()

        return

    def PrepareHALRequestSpec(self, req_spec):
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class L4LbServiceObjectHelper:
    def __init__(self):
        self.svcs   = []
        return
    
    def Generate(self, tenant, spec):
        for espec in spec.entries:
            svc = L4LbServiceObject()
            svc.Init(tenant, espec.entry)
            self.svcs.append(svc)
        Store.objects.SetAll(self.svcs)
        return

    def Configure(self):
        cfglogger.info("Configuring %d L4LbServices." % len(self.svcs))
        #halapi.ConfigureInterfaces(self.svcs)
        return
