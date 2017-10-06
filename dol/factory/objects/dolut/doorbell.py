#! /usr/bin/python3
import infra.factory.base as base
from struct import pack, unpack
import model_sim.src.model_wrap as model_wrap


class InfraUtDoorbell(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, qid):
        #self.logger.info("Initializing INFRA_UT_TX_DESCRITOR %s" % self.GID())
        self.qid = qid
        return

    def write(self, ring):
        data = pack('ii', ring.GID(), ring.pi)
        model_wrap.doorbell(data, len(data))