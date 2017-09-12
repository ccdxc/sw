#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api           as halapi
import config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2

class MrObject(base.ConfigObjectBase):
    def __init__(self, pd, slab,    
                 local_wr=False, remote_wr=False, 
                 remote_rd=False, remote_atomic=False):
        super().__init__()
        self.pd = pd
        self.slab = slab
        self.lkey = pd.ep.intf.lif.GetMrKey()
        self.rkey = self.lkey+512
        self.id = slab.id
        self.GID("MR-%s" % self.slab.GID())
        self.local_wr = local_wr
        self.remote_wr = remote_wr
        self.remote_rd = remote_rd
        self.remote_atomic = remote_atomic
        return

    def PrepareHALRequestSpec(self, req_spec):
        cfglogger.info("MR: %s PD: %s SLAB: %s LIF: %s\n" %\
                        (self.GID(), self.pd.GID(), self.slab.GID(),
                         self.pd.ep.intf.lif.hw_lif_id))

        req_spec.hw_lif_id = self.pd.ep.intf.lif.hw_lif_id
        req_spec.pd = self.pd.id
        req_spec.va = self.slab.address
        req_spec.len = self.slab.size
        req_spec.ac_local_wr = self.local_wr
        req_spec.ac_remote_wr = self.remote_wr
        req_spec.ac_remote_rd = self.remote_rd
        req_spec.ac_remote_atomic = self.remote_atomic
        req_spec.lkey = self.lkey
        req_spec.rkey = self.rkey
        req_spec.hostmem_pg_size = self.slab.page_size
        req_spec.va_pages_phy_addr[:] = self.slab.phy_address
        self.Show()

    def ProcessHALResponse(self, req_spec, resp_spec):
        pass

    def Show(self):
        cfglogger.info('MR: %s PD: %s ' %(self.GID(), self.pd.GID()))
        cfglogger.info('LKey: %d RKey: %d address: 0x%x size: %d '\
                        %(self.lkey, self.rkey, self.slab.address, self.slab.size))

class MrObjectHelper:
    def __init__(self):
        self.mrs = []

    def Generate(self, pd, spec):
        self.pd = pd
        if self.pd.remote:
            cfglogger.info("skipping MR generation for remote PD: %s" %(pd.GID()))
            return

        mr_count = spec.count
        slab_count = len(pd.ep.slabs)
        cfglogger.info('mr_count: %d slab_count: %d' \
                        %(mr_count, slab_count))
        count = min(mr_count, slab_count)
        cfglogger.info("Creating %d Mrs. for PD:%s" %\
                       (count, pd.GID()))
        for i in range(count):
            slab = pd.ep.slabs.Get('SLAB%04d' % i)
            mr = MrObject(pd, slab, local_wr=True, remote_wr=True, remote_rd=True, remote_atomic=True)
            self.mrs.append(mr)

    def AddMr(self, mr):
        self.mrs.append(mr)

    def Configure(self):
        if self.pd.remote:
            cfglogger.info("skipping MR configuration for remote PD: %s" %(self.pd.GID()))
            return

        cfglogger.info("Configuring %d Mrs." % len(self.mrs))
        halapi.ConfigureMrs(self.mrs)
