#! /usr/bin/python3

import copy

import config.resmgr            as resmgr
import config.objects.ring      as ring
from infra.common.logging   import cfglogger


class EthRingObject(ring.RingObject):
    def __init__(self):
        super().__init__()
        self._mem = None

    def __str__(self):
        return ("%s Lif:%s QueueType:%s Queue:%s Ring:%s Mem:%s Size=%d DescSize=%d" %
                (self.__class__.__name__,
                 self.queue.queue_type.lif.hw_lif_id,
                 self.queue.queue_type.type,
                 self.queue.id,
                 self.id,
                 self._mem, self.size, self.desc_size))

    def Init(self, queue, spec):
        super().Init(queue, spec)
        self.num = int(getattr(spec, 'num', 0))
        self.desc_size = self.descriptor_template.meta.size

    def Configure(self):
        # Make sure ring_size is a power of 2
        assert (self.size != 0) and ((self.size & (self.size - 1)) == 0)
        self._mem = resmgr.HostMemoryAllocator.get(self.size * self.desc_size)
        cfglogger.info("Creating Ring %s" % self)
        self.queue.descriptors = [None] * self.size

    def Post(self, descriptor):
        cfglogger.info("Posting %s @ %s on %s" % (descriptor, self.queue.qstate.get_pindex(self.num), self))

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.queue.qstate.get_pindex(self.num)))
        # Remember descriptor for completion processing
        self.queue.descriptors[self.queue.qstate.get_pindex(self.num)] = descriptor
        descriptor.Write()

        # Increment posted index
        self.queue.qstate.incr_pindex(self.num)
        self.queue.qstate.Read()

    def Consume(self, descriptor):
        cfglogger.info("Consuming %s @ %s on %s" % (descriptor, self.queue.qstate.get_cindex(self.num), self))

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.queue.qstate.get_cindex(self.num)))
        # Retreive descriptor for completion processing
        buf = self.queue.descriptors[self.queue.qstate.get_cindex(self.num)].buf
        if hasattr(descriptor, 'buf') and descriptor.buf is not None:
            descriptor.buf.Bind(buf._mem)
        descriptor.Read()

        # Increment consumer index
        self.queue.qstate.incr_cindex(self.num)
        self.queue.qstate.Read()


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
