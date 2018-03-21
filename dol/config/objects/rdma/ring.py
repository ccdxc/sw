#! /usr/bin/python3

import config.resmgr            as resmgr
import config.objects.ring      as ring

from infra.common.logging   import logger
from infra.factory.store    import FactoryStore
import config.objects.rdma.doorbell as doorbell

import model_sim.src.model_wrap as model_wrap


class RdmaRingObject(ring.RingObject):
    def __init__(self):
        super().__init__()
        self.host = None
        self.address = None
        self.size = None
        self.desc_size = None
        self.initialized = False
        self.nic_resident = False

    def Init(self, queue, spec):
        super().Init(queue, spec)
        self.size = spec.size
        self.desc_size = self.descriptor_template.meta.size
        self.doorbell = doorbell.Doorbell()
        self.doorbell.Init(self, spec)

    def SetRingParams(self, host, nic_resident, mem_handle, address, size, desc_size):
        self.host = host
        self.nic_resident = nic_resident
        self.address = address
        self.mem_handle = mem_handle
        self.size = size
        self.desc_size = desc_size
        logger.info("SetRingParams: host %d nic_resident %d eqe_base_addr: 0x%x" \
                       " size %d desc_size %d " %\
                       (self.host, self.nic_resident, self.address, self.size, self.desc_size))


    def Configure(self):
        pass

    def Post(self, descriptor):
        #if not self.initialized:
        #    self.queue.qstate.set_ring_base(self.address)
        #    self.queue.qstate.set_ring_size(self.size)
        # Bind the descriptor to the ring
        logger.info('posting descriptor at pindex: %d..' %(self.queue.qstate.get_pindex(0)))
        descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_pindex(0)))
        if self.nic_resident:
            descriptor.mem_handle = None
        else:
            descriptor.mem_handle = resmgr.MemHandle(descriptor.address,
                                                     resmgr.HostMemoryAllocator.v2p(descriptor.address))

        descriptor.Write()
        logger.info('incrementing pindex..')
        self.queue.qstate.incr_pindex(0, self.size)

        # Increment posted index
        #if self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_RDMA_RECV":
        #    logger.info('incrementing pindex..')
        #    self.queue.qstate.incr_pindex(0)
        # for now, ring doorbell only for SQ.
        # Doorbell ring for RQ will be needed for Prefetch/Cache, will be done later
        #if self.queue.queue_type.purpose == "rdma_sq":
        #    self.doorbell.Ring({})  # HACK

    def Consume(self, descriptor):
        logger.info("Consuming descriptor on Queue(%s) Ring at cindex: %d" % (self.queue.queue_type.purpose.upper(), self.queue.qstate.get_cindex(0)))
        if self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_RDMA_RECV":
            descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_proxy_cindex()))
        elif self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_RDMA_SEND":
           descriptor.address = (self.address + (self.desc_size * ((self.queue.qstate.get_cindex(0) - 1) & (self.size - 1))))
        else:
            descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_cindex(0)))
        if (self.nic_resident):
            descriptor.mem_handle = None 
        else:
            descriptor.mem_handle = resmgr.MemHandle(descriptor.address,
                                                     resmgr.HostMemoryAllocator.v2p(descriptor.address))
    
        descriptor.Read()
        self.queue.qstate.Read()

        # Increment consumer index for CQs and EQs
        if ((self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_CQ") or 
            (self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_EQ")):
            logger.info('incrementing cindex..')
            self.queue.qstate.incr_cindex(0, self.size)

    def Read(self):
        raise NotImplementedError


class RdmaRingObjectHelper:
    def __init__(self):
        self.rings = []
        return

    def Generate(self, queue, spec):
        for rspec in spec.rings:
            ring = RdmaRingObject()
            ring.Init(queue, rspec.ring)
            self.rings.append(ring)
        return

    def Configure(self):
        for ring in self.rings:
            ring.Configure()
