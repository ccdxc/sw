#! /usr/bin/python3
import pdb
import math
import ctypes

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
from infra.common.glopts        import GlobalOptions

import model_sim.src.model_wrap as model_wrap

from scapy.all import *

class EthQstate(Packet):
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd", 0),
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


class AdminQstate(Packet):
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd", 0),
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

        ByteField("enable", 0),
        LELongField("ring_base", 0),
        LEShortField("ring_size", 0),
        BitField("__pad0", 296, 0),
    ]


class EthQstateObject(object):
    def __init__(self, addr, size):
        self.addr = addr
        self.size = size
        self.data = EthQstate(model_wrap.read_mem(self.addr, self.size))

    def Write(self, lgh = cfglogger):
        lgh.info("Writing Qstate @0x%x size: %d" % (self.addr, self.size))
        model_wrap.write_mem(self.addr, bytes(self.data), len(self.data))
        self.Read(lgh)

    def Read(self, lgh = cfglogger):
        data = EthQstate(model_wrap.read_mem(self.addr, self.size))
        lgh.ShowScapyObject(data)
        lgh.info("Read Qstate @0x%x size: %d" % (self.addr, self.size))

    def incr_pindex(self, ring):
        assert(ring < 7)
        value = (self.get_pindex(ring) + 1) & (self.get_ring_size() - 1)
        setattr(self.data[EthQstate], 'p_index%d' % ring, value)
        model_wrap.write_mem(self.addr + 8 + (2 * ring), bytes(ctypes.c_uint16(value)), 2)

    def incr_cindex(self, ring):
        assert(ring < 7)
        value = (self.get_cindex(ring) + 1) & (self.get_ring_size() - 1)
        setattr(self.data[EthQstate], 'c_index%d' % ring, value)
        model_wrap.write_mem(self.addr + 10 + (2 * ring), bytes(ctypes.c_uint16(value)), 2)

    def get_pindex(self, ring):
        assert(ring < 7)
        return getattr(self.data[EthQstate], 'p_index%d' % ring)

    def get_cindex(self, ring):
        assert(ring < 7)
        return getattr(self.data[EthQstate], 'c_index%d' % ring)

    def set_enable(self, value):
        self.data[EthQstate].enable = value
        model_wrap.write_mem(self.addr + 40, bytes(ctypes.c_uint8(value)), 1)

    def set_ring_base(self, value):
        self.data[EthQstate].ring_base = value
        model_wrap.write_mem(self.addr + 41, bytes(ctypes.c_uint64(value)), 8)

    def set_ring_size(self, value):
        value = int(math.log(value, 2))
        self.data[EthQstate].ring_size = value
        model_wrap.write_mem(self.addr + 49, bytes(ctypes.c_uint16(value)), 2)

    def set_cq_base(self, value):
        self.data[EthQstate].cq_base = value
        model_wrap.write_mem(self.addr + 51, bytes(ctypes.c_uint64(value)), 8)

    def get_ring_size(self):
        return int(math.pow(2, self.data[EthQstate].ring_size))

    def Show(self, lgh = cfglogger):
        lgh.ShowScapyObject(self.data)


class AdminQstateObject(object):
    def __init__(self, addr, size):
        self.addr = addr
        self.size = size
        self.data = AdminQstate(model_wrap.read_mem(self.addr, self.size))

    def Write(self, lgh = cfglogger):
        lgh.info("Writing Qstate @0x%x size: %d" % (self.addr, self.size))
        model_wrap.write_mem(self.addr, bytes(self.data), len(self.data))
        self.Read(lgh)

    def Read(self, lgh = cfglogger):
        data = AdminQstate(model_wrap.read_mem(self.addr, self.size))
        lgh.ShowScapyObject(data)
        lgh.info("Read Qstate @0x%x size: %d" % (self.addr, self.size))

    def incr_pindex(self, ring):
        assert(ring < 7)
        value = (self.get_pindex(ring) + 1) & (self.get_ring_size() - 1)
        setattr(self.data[AdminQstate], 'p_index%d' % ring, value)
        model_wrap.write_mem(self.addr + 8 + (2 * ring), bytes(ctypes.c_uint16(value)), 2)

    def incr_cindex(self, ring):
        assert(ring < 7)
        value = (self.get_cindex(ring) + 1) & (self.get_ring_size() - 1)
        setattr(self.data[AdminQstate], 'c_index%d' % ring, value)
        model_wrap.write_mem(self.addr + 10 + (2 * ring), bytes(ctypes.c_uint16(value)), 2)

    def get_pindex(self, ring):
        assert(ring < 7)
        return getattr(self.data[AdminQstate], 'p_index%d' % ring)

    def get_cindex(self, ring):
        assert(ring < 7)
        return getattr(self.data[AdminQstate], 'c_index%d' % ring)

    def set_enable(self, value):
        self.data[AdminQstate].enable = value
        model_wrap.write_mem(self.addr + 16, bytes(ctypes.c_uint8(value)), 1)

    def set_ring_base(self, value):
        self.data[AdminQstate].ring_base = value
        model_wrap.write_mem(self.addr + 17, bytes(ctypes.c_uint64(value)), 8)

    def set_ring_size(self, value):
        value = int(math.log(value, 2))
        self.data[AdminQstate].ring_size = value
        model_wrap.write_mem(self.addr + 25, bytes(ctypes.c_uint16(value)), 2)

    def set_cq_base(self, value):
        pass

    def get_ring_size(self):
        return int(math.pow(2, self.data[AdminQstate].ring_size))

    def Show(self, lgh = cfglogger):
        lgh.ShowScapyObject(self.data)


class EthQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate = None

    def Init(self, queue_type, spec, qid):
        super().Init(queue_type, spec)
        self.id = qid
        #self.GID(str(self.id))
        self.GID("Q%d" % self.id)

    @property
    def qstate(self):
        if self._qstate is None:
            if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
                self._qstate = AdminQstateObject(addr=self.GetQstateAddr(), size=self.queue_type.size)
            else:
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
        req_spec.label.handle = "p4plus"
        if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
            req_spec.queue_state = bytes(EthQstate(host=1, total=1))
            req_spec.label.prog_name = "txdma_stage0.bin"
            req_spec.label.label = "eth_tx_stage0"
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            req_spec.queue_state = bytes(EthQstate(host=1, total=1))
            req_spec.label.prog_name = "rxdma_stage0.bin"
            req_spec.label.label = "eth_rx_stage0"
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
            req_spec.queue_state = bytes(AdminQstate(host=1, total=1))
            req_spec.label.prog_name = "txdma_stage0.bin"
            req_spec.label.label = "adminq_stage0"
        else:
            cfglogger.critical("Unable to set program information for Queue Type %s" % self.queue_type.purpose)
            raise NotImplementedError

    def GetQstateAddr(self):
        return self.queue_type.GetQstateAddr() + (self.id * self.queue_type.size)

    def ConfigureRings(self):
        self.obj_helper_ring.Configure()
        self.qstate.set_enable(1)
        for ring in self.obj_helper_ring.rings:
            if ring.id == 'R0':
                self.qstate.set_ring_base(ring._mem.pa)
                self.qstate.set_ring_size(ring.size)
            elif ring.id == 'R1':
                self.qstate.set_cq_base(ring._mem.pa)
            else:
                raise NotImplementedError
        self.qstate.Read()

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
            for qid in range(espec.queue.count):
                queue = EthQueueObject()
                queue.Init(queue_type, espec.queue, qid)
                self.queues.append(queue)

    def Configure(self):
        for queue in self.queues:
            queue.ConfigureRings()
