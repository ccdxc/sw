#! /usr/bin/python3

import binascii
import config.resmgr            as resmgr

import infra.factory.base as base
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts        import GlobalOptions

class RdmaBufferObject(base.FactoryObjectBase):

    def __init__(self):
        super().__init__()
        self.data = []
        self.size = 0
        self.offset = 0
        self.segments = []
        self.address = 0
        self.slab_id = None

    def Init(self, spec):
        #self.LockAttributes()
        super().Init(spec)
        if (GlobalOptions.dryrun): return
       
        for segment in spec.fields.segments:
            skip_bytes = segment.skip if hasattr(segment, 'skip') else 0
            offset = segment.offset if hasattr(segment, 'offset') else 0
            self.size += (segment.size - skip_bytes) if hasattr(segment, 'size') else 0
            self.data += segment.data[offset:len(segment.data)-skip_bytes] if (hasattr(segment,'data') and segment.data) else []
            #handle segment.offset 

        #self.size = spec.fields.size if hasattr(spec.fields, 'size') else 0
        #self.data = spec.fields.data if hasattr(spec.fields, 'data') else [] 
        # Offset of the data
        self.slab_id = spec.fields.slab
        self.offset = spec.fields.offset if hasattr(spec.fields, 'offset') else 0 
        self.address = spec.fields.slab.address if spec.fields.slab else 0
        self.address += self.offset
        if self.address:
            self.mem_handle = resmgr.MemHandle(self.address,
                                               resmgr.HostMemoryAllocator.v2p(self.address))
        logger.info("Creating Rdma Buffer @0x%x = size: %d offset: %d " %
                       (self.address, self.size, self.offset))

    def Write(self):
        """
        Writes the buffer to address "self.address"
        :return:
        """
        if self.address and self.data:
            logger.info("Writing Buffer @0x%x = size: %d offset: %d " %
                       (self.address, self.size, self.offset))
            resmgr.HostMemoryAllocator.write(self.mem_handle,
                                bytes(self.data[:self.size]))
        else:
            logger.info("Warning:!! buffer is not bound to an address, Write is ignored !!")

    def Read(self):
        """
        Reads a Buffer from address "self.address"
        :return:
        """
        if self.address:
            self.data = resmgr.HostMemoryAllocator.read(self.mem_handle, self.size)
            logger.info("Read Buffer @0x%x = size: %d offset: %d crc(data): 0x%x" %
                       (self.address, self.size, self.offset, binascii.crc32(self.data)))
        else:
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
