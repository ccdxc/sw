#! /usr/bin/python3

import binascii
import config.resmgr            as resmgr

import infra.factory.base as base
from infra.common.logging   import cfglogger

import model_sim.src.model_wrap as model_wrap


class RdmaBufferObject(base.FactoryObjectBase):

    def __init__(self):
        super().__init__()
        self.data = []
        self.size = 0
        self.offset = 0

    def Init(self, spec):
        #self.LockAttributes()
        self.size = spec.fields.size if hasattr(spec.fields, 'size') else 0
        self.data = spec.fields.data if hasattr(spec.fields, 'data') else [] 
        # Offset of the data
        self.offset = spec.fields.offset if hasattr(spec.fields, 'offset') else 0 
        self.address = spec.fields.slab.address if spec.fields.slab else 0
        if self.address:
            self.mem_handle = resmgr.MemHandle(self.address, 
                                    resmgr.HostMemoryAllocator.get_v2p(self.address))
        cfglogger.info("Creating Rdma Buffer @0x%x = size: 0x%d offset: 0x%d " %
                       (self.address, self.size, self.offset))

    def Write(self):
        """
        Writes the buffer to address "self.address"
        :return:
        """
        if self.address and self.data:
            cfglogger.info("Writing Buffer @0x%x = size: 0x%d offset: 0x%d " %
                       (self.address, self.size, self.offset))
            resmgr.HostMemoryAllocator.write(self.mem_handle, self.data)
        else:
            cfglogger.info("Warning:!! buffer is not bound to an address, Write is ignored !!")

    def Read(self):
        """
        Reads a Buffer from address "self.address"
        :return:
        """
        if self.address:
            self.data = resmgr.HostMemoryAllocator.read(self.mem_handle, self.size)
            cfglogger.info("Read Buffer @0x%x = size: 0x%d offset: 0x%d crc(data): 0x%x" %
                       (self.address, self.size, self.offset, binascii.crc32(self.data)))
        else:
            cfglogger.info("Warning:!! buffer is not bound to an address, Read is ignored !!")

    def __eq__(self, other):
        # self: Expected, other: Actual
        cfglogger.info("expected size: %d actual: %d" %(self.size, other.size))
        if self.size-20 > other.size:
            return False
        #print('self_data: [size: %d] %s' % (self.size-20, bytes(self.data[:-20])))
        #print('other_data: [size: %d] %s' % (self.size-20, bytes(other.data[:self.size-20])))
        #cmp = self.data[0:self.size] == other.data[0:self.size]
        cmp = self.data[:-20] == other.data[:self.size-20]
        cfglogger.info("comparison: %s" %cmp) 
        return cmp

    def IsPacket(self):
        return False
