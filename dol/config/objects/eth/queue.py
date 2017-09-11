#! /usr/bin/python3
import pdb
import math

import infra.common.defs        as defs
import infra.common.objects     as objects
import config.resmgr            as resmgr
import config.objects.ring      as ring
import config.objects.eth.ring  as ring
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from config.store               import Store
from infra.common.logging       import cfglogger
from config.objects.queue       import QueueObject

import model_sim.src.model_wrap as model_wrap

from scapy.all import *

class EthQstate(Packet):
    name = "EthQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosA", 0, 4),
        BitField("cosB", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("host", 0, 4),
        BitField("total", 0, 4),
        LEShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),
        LEShortField("p_index2", 0),
        LEShortField("c_index2", 0),
        LEShortField("p_index3", 0),
        LEShortField("c_index3", 0),
        LEShortField("p_index4", 0),
        LEShortField("c_index4", 0),
        LEShortField("p_index5", 0),
        LEShortField("c_index5", 0),
        LEShortField("p_index6", 0),
        LEShortField("c_index6", 0),
        LEShortField("p_index7", 0),
        LEShortField("c_index7", 0),

        ByteField("enable", 0),
        LELongField("ring_base", 0),
        LEShortField("ring_size", 0),
        LELongField("cq_base", 0),
        X3BytesField("__pad0", 0),
        LEShortField("__pad1", 0),
    ]


class EthQstateObject(object):
    def __init__(self, addr, size):
        self.addr = addr
        self.size = size
        self.Read()

    def Write(self, lgh = cfglogger):
        lgh.info("Writing Qstate @0x%x size: %d" % (self.addr, self.size))
        model_wrap.write_mem(self.addr, bytes(self.data), len(self.data))
        self.Read(lgh)

    def Read(self, lgh = cfglogger):
        self.data = EthQstate(model_wrap.read_mem(self.addr, self.size))
        # lgh.ShowScapyObject(self.data)
        lgh.info("Read Qstate @0x%x size: %d" % (self.addr, self.size))

    def incr_pindex(self, ring):
        assert(ring < 7)
        self.set_pindex(ring, (self.get_pindex(ring) + 1) % self.get_ring_size(ring))
        self.Write()

    def set_pindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data[EthQstate], 'p_index%d' % ring, value)
        self.Write()

    def set_cindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data[EthQstate], 'c_index%d' % ring, value)
        self.Write()

    def set_enable(self, value):
        self.data[EthQstate].enable = value
        self.Write()

    def set_ring_base(self, value):
        self.data[EthQstate].ring_base = value
        self.Write()

    def set_ring_size(self, value):
        self.data[EthQstate].ring_size = value
        self.Write()

    def get_ring_size(self, ring):
        self.Read()
        return self.data[EthQstate].ring_size

    def get_pindex(self, ring):
        assert(ring < 7)
        self.Read()
        return getattr(self.data[EthQstate], 'p_index%d' % ring)

    def get_cindex(self, ring):
        assert(ring < 7)
        self.Read()
        return getattr(self.data[EthQstate], 'c_index%d' % ring)

    def Show(self, lgh = cfglogger):
        self.Read()
        lgh.ShowScapyObject(self.data)


class EthQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate    = None

    def Init(self, queue_type, spec):
        super().Init(queue_type, spec)

    @property
    def qstate(self):
        if self._qstate is None:
            self._qstate = EthQstateObject(addr=self.GetQstateAddr(), size=self.queue_type.size)
            cfglogger.info("Loading Qstate: Lif=%s QType=%s QID=%s Addr=0x%x Size=%s" %
                           (self.queue_type.lif.id,
                            self.queue_type.type,
                            self.id,
                            self.GetQstateAddr(),
                            self.queue_type.size))
        return self._qstate

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.lif_handle = 0  # HW LIF ID is not known in DOL. Assume it is filled in by hal::LifCreate.
        req_spec.type_num = self.queue_type.type
        req_spec.qid = 0    # HACK
        req_spec.queue_state = bytes(EthQstate(host=1, total=1))
        req_spec.label.handle = "p4plus"
        if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
            req_spec.label.prog_name = "txdma_stage0.bin"
            req_spec.label.label = "eth_tx_stage0"
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            req_spec.label.prog_name = "rxdma_stage0.bin"
            req_spec.label.label = "eth_rx_stage0"
        else:
            cfglogger.critical("Unable to set program information for Queue Type %s" % self.queue_type.purpose)
            raise NotImplementedError

    def GetQstateAddr(self):
        return self.queue_type.GetQstateAddr() + (self.id * self.queue_type.size)

    def ConfigureRings(self):
        self.obj_helper_ring.Configure()

    def Show(self):
        cfglogger.info('Queue: %s' % self.GID())
        cfglogger.info('- type   : %s' % self.queue_type.GID())
        cfglogger.info('- id     : %s' % self.id)


class EthQueueObjectHelper:
    def __init__(self):
        self.queues = []

    def Generate(self, queue_type, spec):
        if not hasattr(spec, 'queues'):
            return
        for espec in spec.queues:
            for qspec in range(espec.queue.count):
                queue = EthQueueObject()
                queue.Init(queue_type, espec.queue)
                self.queues.append(queue)

    def Configure(self):
        for queue in self.queues:
            queue.ConfigureRings()
