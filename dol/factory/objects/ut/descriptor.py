#! /usr/bin/python3
import infra.factory.base as base
from struct import pack, unpack
import model_sim.src.model_wrap as model_wrap
import infra.common.objects as objects
from factory.objects.ut.buffer import InfraUtBufferObject

class InfraUtTxDescriptorObject(base.FactoryObjectBase):

    ADDR_TYPE = 2

    def __init__(self, size=1024):
        super().__init__()
        self.buffs = []
        self.size = size
        return

    def Init(self, spec = None):
        #self.logger.info("Initializing INFRA_UT_TX_DESCRITOR %s" % self.GID())
        self.buffs = []
        return

    def add_buff(self, buff):
        self.buffs.append(buff)

    def Write(self, ring_id, index):
        #self.logger.inpfo("Writing INFRA_UT_TX_DESCRITOR %s" % self.GID())

        desc_addr = pack('>HHHH', 0, ring_id, index, int(self.ADDR_TYPE))
        desc_addr = unpack(">Q", desc_addr)[0]
        data = bytes(0)
        for buff in self.buffs:
            data = data + pack('QQ', buff.GID(), buff.size)
        self.size = len(data)
        model_wrap.write_mem(desc_addr, data, self.size)
        return

    def Read(self, ring_id, index):
        #self.logger.info("Reading INFRA_UT_TX_DESCRITOR %s" % self.GID())
        desc_addr = pack('>HHHH', 0, ring_id, index, int(self.ADDR_TYPE))
        desc_addr = unpack(">Q", desc_addr)[0]
        assert (self.size % 16 == 0)
        num_bufs = int(self.size / 16)
        data = model_wrap.read_mem(desc_addr, self.size)
        assert (data)
        data = unpack('QQ' * num_bufs, data)
        for addr, size in zip(data[0::2], data[1::2]):
            if not addr or not size:
                break
            buff = InfraUtBufferObject(512)
            buff.GID(addr)
            buff.size = size
            buff.Read()
            self.buffs.append(buff)


class InfraUtRxDescriptorObject(base.FactoryObjectBase):
    def __init__(self, logger):
        super().__init__()
        self.logger = logger
        return

    def Init(self, spec = None):
        self.logger.info("Initializing INFRA_UT_RX_DESCRITOR %s" % self.GID())
        return

    def Write(self):
        self.logger.info("Writing INFRA_UT_RX_DESCRITOR %s" % self.GID())
        return

    def Read(self):
        self.logger.info("Reading INFRA_UT_RX_DESCRITOR %s" % self.GID())
        return
