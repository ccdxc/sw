#! /usr/bin/python3

from iris.config.store               import Store

import iris.config.resmgr            as resmgr
import iris.config.objects.ring      as ring

from infra.common.logging   import logger
from infra.factory.store    import FactoryStore
import iris.config.objects.nvme.doorbell as doorbell

import iris.config.hal.api           as halapi

import model_sim.src.model_wrap as model_wrap

from infra.common.glopts import GlobalOptions

class NvmeRingObject(ring.RingObject):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('RING_NVME'))
        self._mem = None
        self.address = None
        self.size = None
        self.desc_size = None
        self.initialized = False
        self.nic_resident = False
        self.hw_ring_id = None

    def Init(self, queue, spec):
        super().Init(queue, spec)
        self.size = spec.size
        self.desc_size = self.descriptor_template.meta.size
        self.hw_ring_id = spec.hw_ring_id
        self.doorbell = doorbell.Doorbell()
        self.doorbell.Init(self, spec)

        if self.id == 'SQ' or self.id == 'CQ':
            self.initialized = True
            self._mem = resmgr.HostMemoryAllocator.get(self.size * self.desc_size)
            self.address = self._mem.va
            resmgr.HostMemoryAllocator.zero(self._mem, self.size * self.desc_size)

        logger.info("Creating Ring %s" % self)

    def SetRingParams(self, nic_resident, mem_handle, address, size, desc_size):
        self.initialized = True
        self.nic_resident = nic_resident
        self.address = address
        self.mem_handle = mem_handle
        self.size = size
        self.desc_size = desc_size
        logger.info("SetRingParams: nic_resident %d base_addr: 0x%x" \
                       " size %d desc_size %d hw_ring_id: %d" %\
                       (self.nic_resident, self.address, self.size, self.desc_size, self.hw_ring_id))

    def Configure(self):
        if self.id == 'SQ':
            halapi.NvmeSqCreate([self])
        elif self.id == 'CQ':
            halapi.NvmeCqCreate([self])
        else:
            return
        
    def PrepareHALRequestSpec(self, req_spec):
        if (GlobalOptions.dryrun): return

        if req_spec.__class__.__name__ == "NvmeSqSpec":  
            req_spec.hw_lif_id = self.queue.queue_type.lif.hw_lif_id
            req_spec.sq_num = self.queue.id
            req_spec.sq_wqe_size = self.desc_size
            req_spec.num_sq_wqes = self.size
            req_spec.base_addr = self._mem.pa
            #sq with id 'i' is always mapped to cq with id 'i'
            req_spec.cq_num = self.queue.id
        elif req_spec.__class__.__name__ == "NvmeCqSpec":  
            req_spec.hw_lif_id = self.queue.queue_type.lif.hw_lif_id
            req_spec.cq_num = self.queue.id
            req_spec.cq_wqe_size = self.desc_size
            req_spec.num_cq_wqes = self.size
            req_spec.base_addr = self._mem.pa
            #cq with id 'i' is always mapped to interrupt with id 'i'
            req_spec.int_num = self.queue.id
        else:
            assert(0)

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("HAL Response for ring %s" % self)
        return

    def Post(self, descriptor, debug = True):
        #if not self.initialized:
        #    self.queue.qstate.set_ring_base(self.address)
        #    self.queue.qstate.set_ring_size(self.size)
        # Bind the descriptor to the ring
        if debug is True:
            logger.info('posting descriptor at pindex: %d..' %(self.queue.qstate.get_pindex(self.hw_ring_id)))
        descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_pindex(self.hw_ring_id)))
        if self.nic_resident:
            descriptor.mem_handle = None
        else:
            descriptor.mem_handle = resmgr.MemHandle(descriptor.address,
                                                     resmgr.HostMemoryAllocator.v2p(descriptor.address))

        descriptor.Write()
        logger.info('incrementing pindex..')
        self.queue.qstate.incr_pindex(0, self.size)

    def Consume(self, descriptor, debug = True):
        if debug is True:
            logger.info("Consuming descriptor on Queue(%s) Ring at cindex: %d" % (self.queue.queue_type.purpose.upper(), self.queue.qstate.get_cindex(self.hw_ring_id)))
        descriptor.address = (self.address + (self.desc_size * self.queue.qstate.get_cindex(self.hw_ring_id)))
        if (self.nic_resident):
            descriptor.mem_handle = None 
        else:
            descriptor.mem_handle = resmgr.MemHandle(descriptor.address,
                                                     resmgr.HostMemoryAllocator.v2p(descriptor.address))
    
        descriptor.Read()
        self.queue.qstate.Read()

        # Increment consumer index for CQs and EQs
        if (self.queue.queue_type.purpose.upper() == "LIF_QUEUE_PURPOSE_NVME_CQ"):
            logger.info('incrementing cindex..')
            self.queue.qstate.incr_cindex(self.hw_ring_id, self.size)

    def Read(self):
        raise NotImplementedError

    def __str__(self):
        return ("%s Lif:%s/QueueType:%s/Queue:%s/Ring:%s/HwRingId:%s/Mem:%s/Size:%d/DescSize:%d/BaseAddr:0x%x" %
                (self.__class__.__name__,
                 self.queue.queue_type.lif.hw_lif_id,
                 self.queue.queue_type.type,
                 self.queue.id,
                 self.id,
                 self.hw_ring_id,
                 self._mem,
                 self.size, self.desc_size,
                 self._mem.pa if self._mem is not None else 0))


class NvmeRingObjectHelper:
    def __init__(self):
        self.rings = []
        return

    def Generate(self, queue, spec):
        for rspec in spec.rings:
            ring = NvmeRingObject()
            ring.Init(queue, rspec.ring)
            self.rings.append(ring)
        return

    def Configure(self):
        if (GlobalOptions.dryrun):  return

        for ring in self.rings:
            ring.Configure()
