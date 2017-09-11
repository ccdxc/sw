#! /usr/bin/python3

import math

import config.resmgr            as resmgr
import config.objects.ring      as ring
from infra.common.logging   import cfglogger


class EthRingObject(ring.RingObject):
    def __init__(self):
        super().__init__()
        self._mem = None
        self.initialized = False

    def Init(self, queue, spec):
        super().Init(queue, spec)
        self.desc_size = self.descriptor_template.meta.size

    def Configure(self):
        # Make sure ring_size is a power of 2
        assert (self.size != 0) and ((self.size & (self.size - 1)) == 0)
        self._mem = resmgr.HostMemoryAllocator.get(self.size * self.desc_size)
        cfglogger.info("Creating Ring: Lif=%s QType=%s QID=%s Ring=%s Mem=%s size=%d desc_size=%d" %
                       (self.queue.queue_type.lif.id,
                        self.queue.queue_type.type,
                        self.queue.id,
                        self.id,
                        self._mem, self.size, self.desc_size))

    def Post(self, descriptor):
        cfglogger.info("Posting %s" % descriptor)
        if not self.initialized:
            self.queue.qstate.set_enable(1)
            self.queue.qstate.set_ring_base(self._mem.pa)
            self.queue.qstate.set_ring_size(self.size)
            self.initialized = True
            self.queue.qstate.Read()

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.queue.qstate.get_pindex(0)))
        descriptor.Write()

        # Increment local posted index
        self.queue.qstate.incr_pindex(0)

    def Consume(self, descriptor):
        cfglogger.info("Consuming %s" % descriptor)
        self.queue.qstate.Read()

        assert self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX"

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.queue.qstate.get_cindex(0)))
        descriptor.Read()

        # Increment local consumer index
        self.queue.qstate.incr_cindex(0)


class EthRingObjectHelper:
    def __init__(self):
        self.rings = []
        return

    def Generate(self, queue, spec):
        for rspec in spec.rings:
            ring = EthRingObject()
            ring.Init(queue, rspec.ring)
            self.rings.append(ring)
        return

    def Configure(self):
        for ring in self.rings:
            ring.Configure()
