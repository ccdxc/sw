#! /usr/bin/python3
import infra.factory.base as base
import model_sim.src.model_wrap as model_wrap
from struct import pack, unpack
import infra.common.objects as objects


BufferIdAllocator = objects.TemplateFieldObject("range/1/256")


class InfraUtBufferObject(base.FactoryObjectBase):

    ADDR_TYPE = 1

    def __init__(self, size):
        super().__init__()
        self.size = size
        self.data = bytearray(size)
        return

    def Init(self, buf_id=None):
        #self.logger.info("Initializing INFRA_UT_BUFFER %s" % self.GID())
        if not buf_id:
            self.GID(BufferIdAllocator.get())
        else:
            self.GID(buf_id)
        return

    def Write(self):
        #self.logger.info("Writing INFRA_UT_BUFFER %s" % self.GID())
        addr = pack('>HHHH', 0, 0, int(self.GID()), int(self.ADDR_TYPE))
        addr = unpack(">Q", addr)[0]
        model_wrap.write_mem(addr, self.data, self.size)
        return

    def Read(self):
        #self.logger.info("Reading INFRA_UT_BUFFER %s" % self.GID())
        addr = pack('>HHHH', 0, 0, int(self.GID()), int(self.ADDR_TYPE))
        addr = unpack(">Q", addr)[0]
        self.data = (model_wrap.read_mem(addr, self.size))
        return
