#! /usr/bin/python3

import binascii
import config.resmgr            as resmgr

import infra.factory.base as base
from infra.common.logging   import cfglogger

import model_sim.src.model_wrap as model_wrap


class RdmaBufferObject(base.FactoryObjectBase):

    def __init__(self):
        super().__init__()

    def Init(self, spec):
        #self.LockAttributes()
        self.size   = spec.fields.size    # Size of the buffer
        self.data   = spec.fields.data    # Contents of the buffer
        self.offset = spec.fields.offset  # Offset of the data
        self.address   = spec.fields.slab.address
        cfglogger.info("Creating Rdma Buffer @0x%x = size: 0x%d offset: 0x%d crc(data): 0x%x" %
                       (self.address, self.size, self.offset, binascii.crc32(self.data)))


    def Write(self):
        """
        Writes the buffer to address "self.address"
        :return:
        """
        cfglogger.info("Writing Buffer @0x%x = size: 0x%d offset: 0x%d crc(data): 0x%x" %
                       (self.address, self.size, self.offset, binascii.crc32(self.data)))
        print("%s" %type(self.data))
        model_wrap.write_mem(self.address, self.data[self.offset:], self.size)

    def Read(self):
        """
        Reads a Buffer from address "self.address"
        :return:
        """
        self.data = model_wrap.read_mem(self.address, self.size)
        cfglogger.info("Read Buffer @0x%x = size: 0x%d offset: 0x%d crc(data): 0x%x" %
                       (self.address, self.size, self.offset, binascii.crc32(self.data)))
