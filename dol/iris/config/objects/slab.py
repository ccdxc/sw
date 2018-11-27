#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
from iris.config.store               import Store
from infra.common.logging       import logger

import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2
from infra.common.glopts import GlobalOptions

class SlabObject(base.ConfigObjectBase):
    def __init__(self, lif, size, isPrivileged = False):
        super().__init__()
        self.lif = lif
        if isPrivileged is True:
            self.id = self.lif.GetKSlabid()
            self.GID("KSLAB%04d" % self.id)
        else:
            self.id = self.lif.GetSlabid()
            self.GID("SLAB%04d" % self.id)
        self.page_size = 4096
        if size > 0:
            self.size = size
        else:
            self.size = self.page_size
        self.address = None
        self.mem_handle = None
        return

    def Configure(self):
        if (GlobalOptions.dryrun): return
        self.mem_handle = resmgr.HostMemoryAllocator.get(self.size)
        assert(self.mem_handle != None)
        self.address = self.mem_handle.va
        assert(self.address % self.page_size == 0)

        self.phy_address = []
        for i in range(0, self.size, self.page_size):
            phy_addr = resmgr.HostMemoryAllocator.v2p(self.address + i)
            phy_addr = phy_addr & 0x7fffffffffffffff
            self.phy_address.append(phy_addr)

        self.Show()

    def GetDMATableSlab(self):
        if (GlobalOptions.dryrun): return
        assert(self.address is not None)
        # Allocating one host page size - page translation directory
        dma_mem_handle = resmgr.HostMemoryAllocator.get(self.page_size)
        assert(dma_mem_handle != None)
        src_dma_phy_addr = resmgr.HostMemoryAllocator.v2p(dma_mem_handle.va)

        data = []
        for i in range(0, len(self.phy_address)):
            data += self.phy_address[i].to_bytes(8, 'little')

        resmgr.HostMemoryAllocator.write(dma_mem_handle, bytes(data))
        return src_dma_phy_addr


    def Show(self):
        if (GlobalOptions.dryrun): return
        logger.info('SLAB: %s LIF: %s' %(self.GID(), self.lif.GID()))
        logger.info('size: %d address: 0x%x' %(self.size, self.address))

    def GetPageSize(self):
        return int(self.page_size)

class SlabObjectHelper:
    def __init__(self):
        self.slabs = []

    def Generate(self, lif, spec):
        count = spec.count
        logger.info("Creating %d Slabs. for LIF:%s" %\
                       (count, lif.GID()))
        for slab_id in range(count):
            slab = SlabObject(lif, spec.size)
            self.slabs.append(slab)

    def AddSlab(self, slab):
        self.slabs.append(slab)

    def Configure(self):
        logger.info("Configuring %d Slabs." % len(self.slabs)) 
        for slab in self.slabs:
            slab.Configure()
