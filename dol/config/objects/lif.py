#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects

import config.resmgr            as resmgr
import config.objects.queue     as queue

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api            as halapi
import config.hal.defs           as haldefs

import pdb

class LifObject(objects.FrameworkObject):
    def __init__(self, tenant, spec, namespace = None):
        super().__init__()

        if namespace:
            self.id = namespace.get()
        else:
            self.id = resmgr.LifIdAllocator.get()
        self.GID("Lif%d" % self.id)
        self.mac_addr   = resmgr.LifMacAllocator.get()
        self.status     = haldefs.interface.IF_STATUS_UP
        self.enable_rdma = 1;
       
        self.types      = {}

        self.queues = []
        for qt in spec.types:
            queues = queue.LoadConfigSpec(qt.entry, self)
            self.types[qt.entry.type] = queues
            self.queues += queues

        self.tenant     = tenant
        self.Show()
        return

    def Show(self):
        cfglogger.info("- LIF   : %s" % self.GID())
        cfglogger.info("  - # Types     : %d" % len(self.types))
        cfglogger.info("  - # Queues    : %d" % len(self.queues))
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.lif_id = self.id
        req_spec.mac_addr = self.mac_addr.getnum()
        req_spec.admin_status = self.status
        req_spec.enable_rdma = self.enable_rdma;

        #for q in self.queues:
        #    qspec = req_spec.queues.add()
        #    q.configure(qspec)

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.lif_handle
        cfglogger.info("- LIF %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class LifObjectHelper:
    def __init__(self):
        self.lifs = []
        self.aidx = 0
        return

    def Generate(self, tenant, spec, namespace):
        count = namespace.GetCount()
        cfglogger.info("Creating %d Lifs. for Tenant:%s" %\
                       (count, tenant.GID()))
        for l in range(count):
            lif = LifObject(tenant, spec, namespace)
            self.lifs.append(lif)
        return
        
    def Configure(self):
        cfglogger.info("Configuring %d LIFs." % len(self.lifs)) 
        halapi.ConfigureLifs(self.lifs)
        return

    def Alloc(self):
        if self.aidx == (len(self.lifs) - 1):
            assert(0)
        lif = self.lifs[self.aidx]
        self.aidx += 1
        return lif
