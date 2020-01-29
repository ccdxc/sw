#! /usr/bin/python3

from infra.common.defs               import status
from infra.common.logging            import logger
from infra.common.glopts             import GlobalOptions

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.store import EzAccessStore
import apollo.config.objects.host.ring    as ring
from factory.objects.eth.descriptor  import *


class EthRingObject(ring.RingObject):

    def __init__(self):
        super().__init__()
        self._mem = None    # Primary descriptor memory
        self._sgmem = None  # Scatter Gather descriptor memory
        self.pi = 0     # Local PI
        self.ci = 0     # Local CI
        self.alt_pi = 0 # Alternative PI
        self.exp_color = 1  # Expect this color until ring wrap, then toggle
        self.Node = EzAccessStore.GetDUTNode()

    def Init(self, queue, spec):
        super().Init(queue, spec)

        self.size = self.queue.size
        assert isinstance(self.size, int) and (self.size != 0) and ((self.size & (self.size - 1)) == 0)

        if self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
            self.desc_size = sizeof(EthTxDescriptor)
            self.sg_elem_size = sizeof(EthTxSgElement)
            self.sg_desc_size = sizeof(EthTxSgDescriptor)
        elif self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            self.desc_size = sizeof(EthRxDescriptor)
            self.sg_elem_size = sizeof(EthRxSgElement)
            self.sg_desc_size = sizeof(EthRxSgDescriptor)
        elif self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_EQ":
            self.desc_size = sizeof(EthEqDescriptor)
        elif self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
            self.desc_size = sizeof(AdminDescriptor)

        if self.queue.queue_type.purpose in ["LIF_QUEUE_PURPOSE_TX", "LIF_QUEUE_PURPOSE_RX"]:
            assert (self.desc_size != 0) and ((self.desc_size & (self.desc_size - 1)) == 0)
            assert (self.sg_desc_size != 0) and ((self.desc_size & (self.desc_size - 1)) == 0)

        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return

        #Make sure ring_size is a power of 2
        self._mem = ResmgrClient[self.Node].HostMemoryAllocator.get(self.size * self.desc_size)
        ResmgrClient[self.Node].HostMemoryAllocator.zero(self._mem, self.size * self.desc_size)
        if self.queue.queue_type.purpose in ["LIF_QUEUE_PURPOSE_TX", "LIF_QUEUE_PURPOSE_RX"]:
            self._sgmem = ResmgrClient[self.Node].HostMemoryAllocator.get(self.size * self.sg_desc_size)

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

        # Write the SG descriptor
        self.queue.buffers[self.pi] = descriptor.GetBuffer()
        if hasattr(descriptor, '_sgelems'):
            offset = self.pi * self.sg_desc_size
            for i in range(0, len(descriptor._sgelems)):
                sg_elem = descriptor._sgelems[i]
                logger.info("Writing EthSGElem @ %s mem %s" % (i, self._sgmem + offset))
                ResmgrClient[self.Node].HostMemoryAllocator.write(self._sgmem + offset, bytes(sg_elem))
                logger.info(ctypes_pformat(sg_elem))
                offset += self.sg_elem_size

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
            logger.error("Consume: Retry, descriptor color %d did not match expected color %d" % (descriptor.GetColor(), self.exp_color))
            return status.RETRY

        if descriptor.GetCompletionIndex() >= self.size:
            logger.error("Consume: Invalid completion index.")
            return status.ERROR

        # For RX queues, match the completion with the posted buffer.
        # We need this to verify buffer contents.
        buf = self.queue.buffers[self.ci]
        if self.queue.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            if buf is None:
                logger.error("Consume: Buffer not found in map.")
                return status.ERROR
            descriptor.GetBuffer().Bind(buf._mem)

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
