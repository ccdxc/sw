#! /usr/bin/python3

from infra.common.defs          import status
import config.resmgr            as resmgr
import config.objects.ring      as ring
from infra.common.logging       import cfglogger
from infra.common.glopts        import GlobalOptions


class EthRingObject(ring.RingObject):
    def __init__(self):
        super().__init__()
        self._mem = None
        self.pi = 0     # Local PI
        self.ci = 0     # Local CI
        self.exp_color = 1  # Expect this color until ring wrap, then toggle

    def Init(self, queue, spec):
        super().Init(queue, spec)
        self.size = self.queue.size
        assert isinstance(self.size, int) and (self.size != 0) and ((self.size & (self.size - 1)) == 0)
        self.desc_size = self.descriptor_template.meta.size
        assert isinstance(self.desc_size, int) and (self.desc_size != 0) and ((self.desc_size & (self.desc_size - 1)) == 0)

    def Configure(self):

        if GlobalOptions.dryrun:
            return

        # Make sure ring_size is a power of 2
        self._mem = resmgr.HostMemoryAllocator.get(self.size * self.desc_size)
        resmgr.HostMemoryAllocator.zero(self._mem, self.size * self.desc_size)
        cfglogger.info("Creating Ring %s" % self)

    def Post(self, descriptor):

        if GlobalOptions.dryrun:
            return

        cfglogger.info("Posting %s @ %s on %s" % (descriptor, self.pi, self))

        # Check descriptor compatibility
        assert(descriptor is not None)
        assert(self.desc_size == descriptor.size)

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.pi))
        # Remember descriptor for completion processing
        self.queue.descriptors[self.pi] = descriptor
        descriptor.Write()

        # Increment posted index
        self.pi += 1
        self.pi %= self.size

        return status.SUCCESS

    def Consume(self, descriptor):

        if GlobalOptions.dryrun:
            return

        cfglogger.info("Consuming %s @ %s on %s" % (descriptor, self.ci, self))

        # Check descriptor compatibility
        assert(descriptor is not None)
        assert(self.desc_size == descriptor.size)

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.ci))
        # Retreive descriptor for completion processing
        d = self.queue.descriptors[self.ci]
        if descriptor.GetBuffer() is not None and d.GetBuffer() is not None:
            descriptor.GetBuffer().Bind(d.GetBuffer()._mem)
        descriptor.Read()

        # Have we received a completion?
        if descriptor.GetColor() != self.exp_color:
            return status.RETRY

        # If we have reached the end of the ring then, toggle the expected color
        if descriptor.GetCompletionIndex() < self.ci:
            self.exp_color = 0 if self.exp_color else 1

        # Increment consumer index
        self.ci = (descriptor.GetCompletionIndex() + 1) % self.size

        return status.SUCCESS

    def __str__(self):
        return ("%s Lif:%s/QueueType:%s/Queue:%s/Ring:%s/Mem:%s/Size:%d/DescSize:%d/PI:%d/CI:%d" %
                (self.__class__.__name__,
                 self.queue.queue_type.lif.hw_lif_id,
                 self.queue.queue_type.type,
                 self.queue.id,
                 self.id,
                 self._mem, self.size, self.desc_size,
                 self.pi, self.ci))

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
