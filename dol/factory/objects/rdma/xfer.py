#! /usr/bin/python3

import binascii
import config.resmgr            as resmgr

import infra.factory.base as base
from infra.common.logging   import cfglogger

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts import GlobalOptions


class RdmaXferObject(base.FactoryObjectBase):

    def __init__(self):
        super().__init__()
        self.size = 0
        self.offset = 0
        self.address = 0

    def Init(self, spec):
        #self.LockAttributes()
        if (GlobalOptions.dryrun): return
       
        self.size = spec.fields.size if hasattr(spec.fields, 'size') else 0 
        self.offset = spec.fields.offset if hasattr(spec.fields, 'offset') else 0 
        self.address = spec.fields.slab.address if spec.fields.slab else 0
        self.address += self.offset
        if self.address:
            self.mem_handle = resmgr.MemHandle(self.address, 
                                    resmgr.HostMemoryAllocator.get_v2p(self.address))
        cfglogger.info("Creating Rdma Xfer @0x%x = size: %d offset: %d " %
                       (self.address, self.size, self.offset))

    def Write(self):
        pass

    def Read(self):
        pass

    def Consume(self, buffer):
        """
        Reads a Xfer from address "self.address"
        :return:
        """
        if self.address:
            buffer.data = resmgr.HostMemoryAllocator.read(self.mem_handle, self.size)
            buffer.size = self.size
            cfglogger.info("Read Xfer @0x%x = size: %d offset: %d" %
                       (self.address, self.size, self.offset))
        else:
            cfglogger.info("Warning:!! Xfer is not bound to an address, Read is ignored !!")

    def __eq__(self, other):
        pass

    def IsPacket(self):
        return False
