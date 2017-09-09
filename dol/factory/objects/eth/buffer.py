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
        super().Init(spec)
        self.size = spec.fields.size
        self.data = getattr(spec.fields, 'data', None)
        bind = getattr(spec.fields, 'bind', False)

        # Bind the buffer if required by spec
        self._mem = None
        self.addr = None    # Required to fill the 'buf_addr' field of descriptor
        if bind:
            # Allocate Memory for the buffer
            self._mem = resmgr.HostMemoryAllocator.get(self.size)
            self.addr = self._mem.pa
            # Set buffer contents
            if self.data is None:
                self.data = bytes([0x0] * spec.fields.size)

        if self.data is not None:
            penscapy.ShowRawPacket(self.data, self.logger)

    def Write(self):
        """
        Writes the buffer to Host Memory
        :return:
        """
        if not self._mem: return

        self.logger.info("Writing Buffer %s = size: 0x%x crc(data): 0x%x" %
                         (self._mem, self.size, binascii.crc32(self.data)))
        resmgr.HostMemoryAllocator.write(self._mem, bytes(self.data))

    def Read(self):
        """
        Reads a Buffer from Host Memory
        :return:
        """
        if not self._mem: return self.data

        self.data = resmgr.HostMemoryAllocator.read(self._mem, self.size)
        # penscapy.ShowRawPacket(self.data, self.logger)
        self.logger.info("Read Buffer %s = size: 0x%x crc(data): 0x%x" %
                         (self._mem, self.size, binascii.crc32(self.data)))
        return self.data

    def Bind(self, mem):
        assert(isinstance(mem, resmgr.MemHandle))
        super().Bind(mem)
        self.addr = mem.pa

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        ret = (self.size == other.size)
        if self._mem and other._mem:
            ret = ret and self._mem == other._mem and self.addr == other.addr
        return ret

    def __copy__(self):
        obj = super().__copy__()
        obj.GID('ACTUAL_' + obj.GID())
        # obj._mem must point to the original buffer's memory handle
        # obj.size must be same as original buffer's size
        # The following fields are filled in by Read()
        obj.data = None
        return obj

    def IsPacket(self):
        return True

    def __bytes__(self):
        return bytes(self.data)
