#! /usr/bin/python3

import infra.factory.scapyfactory   as scapyfactory
import infra.factory.base           as base

import infra.common.objects as objects
from infra.common.logging import logger



class EthBufferObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()

    def Init(self, spec):
        super().Init(spec)
        self.size = getattr(spec.fields, 'size', 0)
        self.data = getattr(spec.fields, 'data', None)
        self.bind = getattr(spec.fields, 'bind', False)
        self.hostmemmgr = objects.GetHostMemMgrObject()

        # Bind the buffer if required by spec
        self._mem = None
        self.addr = None    # Required to fill the 'buf_addr' field of descriptor
        if self.bind:
            # Allocate Memory for the buffer
            self._mem = self.hostmemmgr.get(self.size, page_aligned=False)
            self.addr = self._mem.pa

        logger.info("Init %s" % self)

    def Write(self):
        """
        Writes the buffer to Host Memory
        :return:
        """
        if not self._mem: return

        logger.info("Writing %s" % self)
        if self.data is None:
            self.hostmemmgr.zero(self._mem, self.size)
        else:
            logger.info("=" * 30, "WRITING BUFFER", "=" * 30)
            scapyfactory.Parse(self.data).Show()
            self.hostmemmgr.write(self._mem, bytes(self.data))
            logger.info("=" * 30, "END WRITING BUFFER", "=" * 30)

    def Read(self):
        """
        Reads a Buffer from Host Memory
        :return:
        """
        if not self._mem: return self.data

        logger.info("Read %s" % self)
        self.data = self.hostmemmgr.read(self._mem, self.size)

        logger.info("=" * 30, "READ BUFFER", "=" * 30)
        scapyfactory.Parse(self.data).Show()
        logger.info("=" * 30, "END READ BUFFER", "=" * 30)

        return self.data

    def Bind(self, mem):
        assert(isinstance(mem, objects.MemHandle))
        super().Bind(mem)
        self.addr = mem.pa

    def __str__(self):
        return "%s GID:%s/Memory:%s/Size:0x%x" % (
                self.__class__.__name__, self.GID(), self._mem, self.size)

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
