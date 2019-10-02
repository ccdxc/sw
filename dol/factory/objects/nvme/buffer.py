#! /usr/bin/python3

import binascii
import iris.config.resmgr            as resmgr

import infra.factory.base as base
import infra.common.objects as objects
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts        import GlobalOptions

from scapy.all import *

class NvmeBufferObject(base.FactoryObjectBase):

    def __init__(self):
        super().__init__()
        self.data = []
        self.size = 0
        self.slabs = []
        self.phy_pages = []

    def Init(self, spec):
        super().Init(spec)
        if (GlobalOptions.dryrun): return

        if hasattr(spec.fields, 'session'):
            nvme_session = spec.fields.session
        else:
            logger.error("Error!! nvme session needs to be specified for the buffer")
            exit

        self.size = spec.fields.size if hasattr(spec.fields, 'size') else 0 
        self.offset = spec.fields.offset if hasattr(spec.fields, 'offset') else 0 
        self.data = spec.fields.data if hasattr(spec.fields, 'data') else 0 
        self.page_size = nvme_session.lif.spec.host_page_size
        assert self.page_size > 0
        assert self.size % nvme_session.ns.lba_size == 0

        logger.info("Creating Nvme Buffer: lba_size: %d size: %d page_size: %d offset: %d" 
                    % (nvme_session.ns.lba_size, self.size, self.page_size, self.offset))

        self.num_pages = (int) (self.size / self.page_size)
        if (self.size % self.page_size):
            self.num_pages += 1
        
        for i in range(self.num_pages):
           slab = nvme_session.lif.GetNextSlab()
           self.slabs.append(slab)
           logger.info("Slab with address 0x%x allocated" % (slab.address))
           phy_address = resmgr.HostMemoryAllocator.v2p(slab.address)
           self.phy_pages.append(phy_address)
           logger.info("Physical page list[%d]: 0x%x" % (i, self.phy_pages[i])) 
        assert(self.num_pages == len(self.slabs))
       
    def min (x, y):
        if x > y: 
            return x
        else:
            return y

    def Write(self):
        """
        Writes the buffer to address "self.address"
        :return:
        """
        if self.data:
            page_offset = self.offset
            data_begin = 0
            assert(self.num_pages == len(self.slabs))
            for i in range(self.num_pages):
                page_bytes = min(self.page_size, (self.size - data_begin)) - page_offset
                start_addr = self.slabs[i].address + page_offset
                page_offset = 0
                mem_handle = objects.MemHandle(start_addr, resmgr.HostMemoryAllocator.v2p(start_addr))
                resmgr.HostMemoryAllocator.write(mem_handle,
                                                 (bytes(self.data))[data_begin:data_begin+page_bytes])
                logger.info("Writing Buffer[%d] @0x%x = size: %d " %
                            (i, start_addr, page_bytes))
                data_begin += page_bytes
        else:
            logger.info("Warning:!! buffer is not bound to an address, Write is ignored !!")

    def Read(self):
        """
        Reads a Buffer from address "self.address"
        :return:
        """
        logger.info("Warning:!! buffer is not bound to an address, Read is ignored !!")

    def __eq__(self, other):
        # self: Expected, other: Actual
        logger.info("expected size: %d actual: %d" %(self.size, other.size))

        if self.size > other.size: 
           return False

        logger.info('Expected: self_data: [size: %d] %s' % (self.size, binascii.hexlify(bytes(self.data))))
        logger.info('Actual: other_data: [size: %d] %s' % (self.size, binascii.hexlify(bytes(other.data[:self.size]))))
        cmp = bytes(self.data) == bytes(other.data[:self.size])
        logger.info("comparison: %s" %cmp) 
        #logger.info("Compare data byte by byte:") 
        #for i in range(self.size):
        #    print ('[%d] %d %d %d : 0x%x 0x%x' % (i, (self.data[i] == other.data[i]),
        #                                              self.data[i], other.data[i],
        #                                              self.data[i], other.data[i])) 
        return cmp

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        # obj._mem must point to the original buffer's memory handle
        # obj.size must be same as original buffer's size
        # The following fields are filled in by Read()
        obj.data = None
        return obj

    def GetBuffer(self):
        return None

    def IsPacket(self):
        return False

