#! /usr/bin/python3

import binascii
import config.resmgr            as resmgr
import infra.penscapy.penscapy  as penscapy
import infra.factory.base       as base
from infra.common.logging import cfglogger

import infra.factory.base as base


class EthBufferObject(base.FactoryObjectBase):

    def __init__(self):
        super().__init__()
        self.logger = cfglogger

    def Init(self, spec):
        self.spec = spec
        self.size = spec.fields.size
        self.hw = getattr(spec.fields, 'hw', False)
    
        # Contents of the buffer
        self.data = getattr(spec.fields, 'data', None)
        if self.data is None and self.hw is True:
            self.data = bytes([0x0] * spec.fields.size)

        if self.hw:
            # Allocate Memory for the buffer
            self._mem = resmgr.HostMemoryAllocator.get(self.size)
            self.addr = self._mem.pa

    def Write(self):
        """
        Writes the buffer to Host Memory
        :return:
        """
        if not self.hw: return
        self.logger.info("Writing Packet into Buffer")
        penscapy.ShowRawPacket(self.data, self.logger)
        self.logger.info("Writing Buffer %s = size: 0x%d crc(data): 0x%x" %
                         (self._mem, self.size, binascii.crc32(self.data)))
        resmgr.HostMemoryAllocator.write(self._mem, bytes(self.data))

    def Read(self):
        """
        Reads a Buffer from Host Memory
        :return:
        """
        if self.hw:
            self.data = resmgr.HostMemoryAllocator.read(self._mem, self.size)
            self.logger.info("Read Buffer %s = size: 0x%d crc(data): 0x%x" %
                             (self._mem, self.size, binascii.crc32(self.data)))
        return self.data

    def __eq__(self, other):
        if other is None:
            return False
        return self.data is not None and other.data is not None and len(self.data) == len(other.data)

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        obj.data = bytes([0x0] * obj.size)
        return obj

    def IsPacket(self):
        return True

    def __bytes__(self):
        return bytes(self.data)
