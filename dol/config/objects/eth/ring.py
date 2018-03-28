#! /usr/bin/python3

from infra.common.defs          import status
import config.resmgr            as resmgr
import config.objects.ring      as ring
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
from factory.objects.eth.descriptor import IONIC_TX_MAX_SG_ELEMS, ctypes_pformat


class EthRingObject(ring.RingObject):

    def __init__(self):
        super().__init__()
        self._mem = None    # Primary descriptor memory
        self._sgmem = None  # Scatter Gather descriptor memory
        self.pi = 0     # Local PI
        self.ci = 0     # Local CI
        self.exp_color = 1  # Expect this color until ring wrap, then toggle

    def Init(self, queue, spec):
        super().Init(queue, spec)
        self.size = self.queue.size
        assert isinstance(self.size, int) and (self.size != 0) and ((self.size & (self.size - 1)) == 0)
        self.desc_size = self.descriptor_template.meta.size
        assert isinstance(self.desc_size, int) and (self.desc_size != 0) and ((self.desc_size & (self.desc_size - 1)) == 0)
        if self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
            self.sg_desc_size = self.descriptor_template.meta.size
            assert isinstance(self.desc_size, int) and (self.sg_desc_size != 0) and ((self.desc_size & (self.desc_size - 1)) == 0)

        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return

        # Make sure ring_size is a power of 2
        self._mem = resmgr.HostMemoryAllocator.get(self.size * self.desc_size)
        resmgr.HostMemoryAllocator.zero(self._mem, self.size * self.desc_size)
        if self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
            self._sgmem = resmgr.HostMemoryAllocator.get(IONIC_TX_MAX_SG_ELEMS * self.size * self.sg_desc_size)

        logger.info("Creating Ring %s" % self)

    def Configure(self):
        pass

    def Post(self, descriptor):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return

        logger.info("Posting %s @ %s on %s" % (descriptor, self.pi, self))

        # Check descriptor compatibility
        assert(descriptor is not None)
        assert(self.desc_size == descriptor.size)

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.pi))
        descriptor.Write()
        if 'num_sg_elems'in descriptor.fields:
            for i in range(descriptor.fields['num_sg_elems']):
                index = (self.pi * IONIC_TX_MAX_SG_ELEMS) + i
                byte_off = self.sg_desc_size * index
                memhandle = resmgr.MemHandle(va=self._sgmem.va + byte_off, pa=self._sgmem.pa + byte_off)
                sg_elem = descriptor._sgelems[i]
                logger.info("Writing EthTxSGElem @ %s mem %s" % (index, memhandle))
                resmgr.HostMemoryAllocator.write(memhandle, bytes(sg_elem))
                logger.info(ctypes_pformat(sg_elem))

        if self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            self.queue.descriptors[self.pi] = descriptor

        # Increment posted index
        self.pi = (self.pi + 1) % self.size

        return status.SUCCESS

    def Consume(self, descriptor):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return

        logger.info("Consuming %s @ %s on %s" % (descriptor, self.ci, self))

        # Check descriptor compatibility
        assert(descriptor is not None)
        assert(self.desc_size == descriptor.size)

        # Bind the descriptor to the ring
        descriptor.Bind(self._mem + (self.desc_size * self.ci))
        descriptor.Read()

        # Have we received a completion?
        if descriptor.GetColor() != self.exp_color:
            return status.RETRY

        # For RX queues, match completion descriptor with the posted descriptor.
        # We need this to verify buffer contents.
        if self.queue.queue_type.purpose ==\
                "LIF_QUEUE_PURPOSE_RX":
            d = self.queue.descriptors[self.ci]
            if d is None:
                logger.error("Consume: Descriptor not found in map.")
                return status.ERROR
            descriptor.GetBuffer().Bind(d.GetBuffer()._mem)

        # Increment consumer index
        self.ci = (self.ci + 1) % self.size

        # If we have reached the end of the ring then, toggle the expected color
        if self.ci == 0:
            self.exp_color = 0 if self.exp_color else 1

        return status.SUCCESS

    def __str__(self):
        return ("%s Lif:%s/QueueType:%s/Queue:%s/Ring:%s/Mem:%s/SgMem:%s/Size:%d/DescSize:%d/PI:%d/CI:%d" %
                (self.__class__.__name__,
                 self.queue.queue_type.lif.hw_lif_id,
                 self.queue.queue_type.type,
                 self.queue.id,
                 self.id,
                 self._mem,
                 self._sgmem,
                 self.size, self.desc_size,
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
