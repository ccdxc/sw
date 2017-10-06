#! /usr/bin/python3

import config.resmgr            as resmgr
import config.objects.ring      as ring

from infra.common.logging   import cfglogger
from infra.factory.store    import FactoryStore

import model_sim.src.model_wrap as model_wrap


class RdmaRingObject(ring.RingObject):
    def __init__(self):
        super().__init__()
        self.host = None
        self.address = None
        self.size = None
        self.desc_size = None
        self.initialized = False

    def Init(self, queue, spec):
        super().Init(queue, spec)
        self.desc_size = self.descriptor_template.meta.size

    def SetRingParams(self, host, mem_handle, address, size, desc_size):
        self.host = host
        self.address = address
        self.mem_handle = mem_handle
        self.size = size
        self.desc_size = desc_size

    def Configure(self):
        pass

    def Post(self, descriptor):
        #if not self.initialized:
        #    self.queue.qstate.set_ring_base(self.address)
        #    self.queue.qstate.set_ring_size(self.size)
        # Bind the descriptor to the ring
        cfglogger.info('posting descriptor at pindex: %d..' %(self.queue.qstate.get_pindex(0)))
        descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_pindex(0)))
        descriptor.mem_handle = resmgr.MemHandle(descriptor.address,
                                                 resmgr.HostMemoryAllocator.get_v2p(descriptor.address))
        descriptor.Write()
        cfglogger.info('incrementing pindex..')
        self.queue.qstate.incr_pindex(0, self.size)

        # Increment posted index
        #if self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_RDMA_RECV":
        #    cfglogger.info('incrementing pindex..')
        #    self.queue.qstate.incr_pindex(0)
        # for now, ring doorbell only for SQ.
        # Doorbell ring for RQ will be needed for Prefetch/Cache, will be done later
        #if self.queue.queue_type.purpose == "rdma_sq":
        #    self.doorbell.Ring({})  # HACK

    def Consume(self, descriptor):
        if self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_RDMA_RECV":
            descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_proxy_cindex()))
        else:
            descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_cindex(0)))
    
        descriptor.Read()
        self.queue.qstate.Read()

        # Increment consumer index for CQs
        if self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_CQ":
            cfglogger.info('incrementing cindex..')
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
