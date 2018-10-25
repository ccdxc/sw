#! /usr/bin/python3

import math
from infra.common.defs          import status
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
from iris.config.objects.queue       import QueueObject
import model_sim.src.model_wrap as model_wrap

from scapy.all import *

class EthRxQstate(Packet):
    fields_desc = [
        ByteField("pc", 0),
        ByteField("rsvd", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        LEShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("comp_index", 0),

        BitField("color", 0, 1),
        BitField("rsvd1", 0, 7),

        BitField("enable", 0, 1),
        BitField("host_queue", 0, 1),
        BitField("rsvd2", 0, 6),

        LELongField("ring_base", 0),
        LEShortField("ring_size", 0),
        LELongField("cq_ring_base", 0),
        LEShortField("intr_assert_index", 0),
    ]


class EthTxQstate(Packet):
    fields_desc = [
        ByteField("pc", 0),
        ByteField("rsvd", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        LEShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("comp_index", 0),
        LEShortField("ci_fetch", 0),
        LEShortField("ci_miss", 0),

        BitField("color", 0, 1),
        BitField("spec_miss", 0, 1),
        BitField("rsvd1", 0, 6),

        BitField("enable", 0, 1),
        BitField("host_queue", 0, 1),
        BitField("rsvd2", 0, 6),

        LELongField("ring_base", 0),
        LEShortField("ring_size", 0),
        LELongField("cq_ring_base", 0),
        LEShortField("intr_assert_index", 0),
        LELongField("sg_ring_base", 0),

        LELongField("tso_hdr_addr", 0),
        BitField("tso_hdr_len", 0, 10),
        BitField("tso_ipid_delta", 0, 16),
        BitField("tso_seq_delta", 0, 32),
        BitField("spurious_db_cnt", 0, 6),
    ]


class AdminQstate(Packet):
    fields_desc = [
        ByteField("pc", 0),
        ByteField("rsvd", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        LEShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("comp_index", 0),
        LEShortField("ci_fetch", 0),

        BitField("color", 0, 1),
        BitField("spec_miss", 0, 1),
        BitField("rsvd1", 0, 6),

        BitField("enable", 0, 1),
        BitField("host_queue", 0, 1),
        BitField("rsvd2", 0, 6),

        LELongField("ring_base", 0),
        LEShortField("ring_size", 0),
        LELongField("cq_ring_base", 0),
        LELongField("intr_assert_addr", 0),
    ]


class QstateObject(object):

    def __init__(self, addr, __data_class__):
        self.addr = addr
        self.__data_class__ = __data_class__
        self.size = len(self.__data_class__())
        self.data = __data_class__(model_wrap.read_mem(self.addr, self.size))

    def Write(self, lgh = logger):
        if GlobalOptions.skipverify:
            return
        lgh.info("Writing %s @0x%x size: %d" % (self.__data_class__.__name__,
            self.addr, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        self.Read(lgh)

    def Read(self, lgh = logger):
        if GlobalOptions.skipverify:
            return
        lgh.info("Reading %s @0x%x size: %d" % (self.__data_class__.__name__,
            self.addr, self.size))
        self.data = self.__data_class__(model_wrap.read_mem(self.addr, self.size))
        lgh.ShowScapyObject(self.data)

    def Show(self, lgh = logger):
        lgh.ShowScapyObject(self.data)

    def set_pindex(self, ring, value):
        assert(isinstance(ring, int))
        assert(0 <= ring <= 1)
        ring_size = self.data[self.__data_class__].ring_size
        assert(isinstance(value, int))
        assert(0 <= value <= math.pow(2, ring_size))
        model_wrap.write_mem_pcie(self.addr + 8 + (4 * ring), bytes(ctypes.c_uint16(value)), 2)


class EthRxQstateObject(QstateObject):

    def __init__(self, addr):
        super().__init__(addr, EthRxQstate)

    def set_ring_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].ring_base = value
        model_wrap.write_mem_pcie(self.addr + 16, bytes(ctypes.c_uint64(value)), 8)

    def set_ring_size(self, value):
        assert(isinstance(value, int))
        value = int(math.log(value, 2))
        self.data[self.__data_class__].ring_size = value
        model_wrap.write_mem_pcie(self.addr + 24, bytes(ctypes.c_uint16(value)), 2)

    def set_cq_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].cq_base = value
        model_wrap.write_mem_pcie(self.addr + 26, bytes(ctypes.c_uint64(value)), 8)


class EthTxQstateObject(QstateObject):

    def __init__(self, addr):
        super().__init__( addr, EthTxQstate)

    def set_ring_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].ring_base = value
        model_wrap.write_mem_pcie(self.addr + 20, bytes(ctypes.c_uint64(value)), 8)

    def set_ring_size(self, value):
        assert(isinstance(value, int))
        value = int(math.log(value, 2))
        self.data[self.__data_class__].ring_size = value
        model_wrap.write_mem_pcie(self.addr + 28, bytes(ctypes.c_uint16(value)), 2)

    def set_cq_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].cq_base = value
        model_wrap.write_mem_pcie(self.addr + 30, bytes(ctypes.c_uint64(value)), 8)

    def set_sg_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].cq_base = value
        model_wrap.write_mem_pcie(self.addr + 40, bytes(ctypes.c_uint64(value)), 8)


class AdminQstateObject(QstateObject):

    def __init__(self, addr):
        super().__init__(addr, AdminQstate)

    def set_ring_base(self, value):
        self.data[AdminQstate].ring_base = value
        model_wrap.write_mem_pcie(self.addr + 18, bytes(ctypes.c_uint64(value)), 8)

    def set_ring_size(self, value):
        value = int(math.log(value, 2))
        self.data[AdminQstate].ring_size = value
        model_wrap.write_mem_pcie(self.addr + 26, bytes(ctypes.c_uint16(value)), 2)

    def set_cq_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].cq_base = value
        model_wrap.write_mem_pcie(self.addr + 28, bytes(ctypes.c_uint64(value)), 8)


class EthQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate = None
        self._mem = None
        self.pid = 0

    def __str__(self):
        return ("%s Lif:%s/QueueType:%s/Queue:%s/Size:%s/Qstate:%x/QstateSize:%s" %
                (self.__class__.__name__,
                self.queue_type.lif.id,
                self.queue_type.type,
                self.id,
                self.size,
                self.GetQstateAddr(),
                self.queue_type.size))

    @property
    def qstate(self):
        if self._qstate is None:
            if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
                self._qstate = EthTxQstateObject(addr=self.GetQstateAddr())
            elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
                self._qstate = EthRxQstateObject(addr=self.GetQstateAddr())
            elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
                self._qstate = AdminQstateObject(addr=self.GetQstateAddr())
            else:
                logger.critical("Unable to initialize Qstate for Queue Type %s" % self.queue_type.purpose)
                raise NotImplementedError
            logger.info("Loading Qstate: Lif=%s QType=%s QID=%s Addr=0x%x Size=%s" %
                           (self.queue_type.lif.id,
                            self.queue_type.type,
                            self.id,
                            self.GetQstateAddr(),
                            self.queue_type.size))
        return self._qstate

    def GetQstateAddr(self):
        return self.queue_type.GetQstateAddr() + (self.id * self.queue_type.size)

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.lif_handle = 0  # HW LIF ID is not known in DOL. Assume it is filled in by hal::LifCreate.
        req_spec.type_num = self.queue_type.type
        req_spec.qid = 0    # HACK
        req_spec.label.handle = "p4plus"
        if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
            req_spec.queue_state = bytes(EthTxQstate(host=1, total=1,
                                                     enable=1, color=1,
                                                     host_queue=1))
            req_spec.label.prog_name = "txdma_stage0.bin"
            req_spec.label.label = "eth_tx_stage0"
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            req_spec.queue_state = bytes(EthRxQstate(host=1, total=1,
                                                     enable=1, color=1,
                                                     host_queue=1))
            req_spec.label.prog_name = "rxdma_stage0.bin"
            req_spec.label.label = "eth_rx_stage0"
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
            req_spec.queue_state = bytes(AdminQstate(host=1, total=1,
                                                     enable=1, color=1,
                                                     host_queue=1))
            req_spec.label.prog_name = "txdma_stage0.bin"
            req_spec.label.label = "adminq_stage0"
        else:
            logger.critical("Unable to set program information for Queue Type %s" % self.queue_type.purpose)
            raise NotImplementedError

    def Fill(self):
        if GlobalOptions.eth_mode != "onepkt" and self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            from attrdict import AttrDict
            from infra.factory.store import FactoryStore
            ring_id = 0
            ring = self.obj_helper_ring.rings[ring_id]
            descriptor_template = ring.spec.desc.Get(FactoryStore) if hasattr(ring.spec, 'desc') else None
            buffer_template = ring.spec.buf.Get(FactoryStore) if hasattr(ring.spec, 'buf') else None
            for i in range(ring.size - 1):  # Leave one empty slot so PI != CI
                uid = 'LIF%s_QTYPE%s_%s_%s_%d' % (
                    self.queue_type.lif.hw_lif_id,
                    self.queue_type.type,
                    self.id,
                    ring.id,
                    i)
                # Create Buffer
                buf = buffer_template.CreateObjectInstance()
                buf.GID('%s_%s' % (buffer_template.meta.id, uid))
                buf_spec = AttrDict(fields=AttrDict(size=1518, bind=True))
                buf.Init(buf_spec)
                buf.Write()
                # Create descriptor
                desc = descriptor_template.CreateObjectInstance()
                buf.GID('%s_%s' % (descriptor_template.meta.id, uid))
                desc_spec = AttrDict(fields=AttrDict(addr=buf.addr, len=buf.size))
                desc.Init(desc_spec)
                desc.Write()
                # Post the descriptor
                ret = self.Post(desc)
                assert ret == status.SUCCESS
            self.qstate.set_pindex(ring_id, ring.pi)

    def Configure(self):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return

        self.obj_helper_ring.Configure()

        if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            self.descriptors = [None] * self.size

        for ring in self.obj_helper_ring.rings:
            if ring.id == 'R0':
                self.qstate.set_ring_base(ring._mem.pa)
                self.qstate.set_ring_size(ring.size)
                if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
                    self.qstate.set_sg_base(ring._sgmem.pa)
            elif ring.id == 'R1':
                self.qstate.set_cq_base(ring._mem.pa)
            else:
                raise NotImplementedError

        self.Fill()
        self.qstate.Read()

    def Post(self, descriptor):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return status.SUCCESS

        ring = self.obj_helper_ring.rings[0]
        return ring.Post(descriptor)

    def Consume(self, descriptor):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return status.SUCCESS

        ring = self.obj_helper_ring.rings[1]
        return ring.Consume(descriptor)


class EthQueueObjectHelper:
    def __init__(self):
        self.queues = []

    def Generate(self, queue_type, spec):
        if not hasattr(spec, 'queues'):
            return
        for espec in spec.queues:
            for qid in range(queue_type.count):
                queue = EthQueueObject()
                queue.Init(queue_type, espec.queue)
                self.queues.append(queue)

    def Configure(self):
        for queue in self.queues:
            queue.Configure()
