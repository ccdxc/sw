#! /usr/bin/python3

import binascii
import iris.config.resmgr            as resmgr

import infra.factory.base as base
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts        import GlobalOptions

from scapy.all import *

class RdmaBufferObject(base.FactoryObjectBase):

    def __init__(self):
        super().__init__()
        self.data = []
        self.size = 0
        self.offset = 0
        self.segments = []
        self.address = 0
        self.phy_address = 0
        self.slab_id = None

    def Init(self, spec):
        #self.LockAttributes()
        super().Init(spec)
        if (GlobalOptions.dryrun): return

        self.buffer_type = spec.buffer_type if hasattr(spec, 'buffer_type') else 0

        if self.buffer_type == 1:
            sq_buffer = RdmaQuerySqBuffer(rnr_timer=spec.fields.rnr_timer, retry_timeout=spec.fields.retry_timeout,
                        access_perms_flags=0, access_perms_rsvd=0, qkey_dest_qpn=spec.fields.qkey_dest_qpn,
                        rate_limit_kbps=0, pkey_id=0, rq_psn=spec.fields.rq_psn)
            rq_buffer = RdmaQueryRqBuffer(state=spec.fields.state, pmtu=spec.fields.pmtu, retry_cnt=spec.fields.retry_cnt,
                        rnr_retry=spec.fields.rnr_retry, rrq_depth=spec.fields.rrq_depth, rsq_depth=spec.fields.rsq_depth,
                        sq_psn=spec.fields.sq_psn, ah_id_len=spec.fields.ah_id_len)
            buffer = sq_buffer/rq_buffer
            self.data = bytes(buffer)
            self.size = len(buffer)
            logger.info("Creating Rdma Query Buffer size: %d " % (self.size))
            logger.ShowScapyObject(sq_buffer)
            logger.ShowScapyObject(rq_buffer)

        else:
            if hasattr(spec.fields, 'segments'):
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
                self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            logger.info("Creating Rdma Buffer @0x%x = phy_address: 0x%x size: %d offset: %d " %
                           (self.address, self.phy_address, self.size, self.offset))

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

class RdmaQuerySqBuffer(Packet):
    fields_desc = [
        ByteField("rnr_timer", 0),
        ByteField("retry_timeout", 0),
        ShortField("access_perms_flags", 0),
        ShortField("access_perms_rsvd", 0),
        ShortField("pkey_id", 0),
        IntField("qkey_dest_qpn", 0),
        IntField("rate_limit_kbps", 0),
        IntField("rq_psn", 0),
    ]

class RdmaQueryRqBuffer(Packet):
    fields_desc = [
        BitField("state", 0, 4), 
        BitField("pmtu", 0, 4), 
        BitField("retry_cnt", 0, 4), 
        BitField("rnr_retry", 0, 4), 
        ByteField("rrq_depth", 0),
        ByteField("rsq_depth", 0),
        IntField("sq_psn", 0),
        IntField("ah_id_len", 0),
    ]
