#! /usr/bin/python3

from scapy.all import *

import model_sim.src.model_wrap as model_wrap
from factory.objects.eth.descriptor import *

from infra.common.defs          import status
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions

from apollo.config.objects.host.queue       import QueueObject

def log2(x):
    return (x - 1).bit_length()

# nic/include/eth_common.h
eth_qstate_common_fields_desc = [
        # intr
        ByteField("pc", 0),
        ByteField("rsvd", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        LEShortField("pid", 0),
        # rings
        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),
        LEShortField("p_index2", 0),
        LEShortField("c_index2", 0),
        # cfg
        BitField("enable", 0, 1),
        BitField("debug", 0, 1),
        BitField("host_queue", 0, 1),
        BitField("cpu_queue", 0, 1),
        BitField("eq_enable", 0, 1),
        BitField("intr_enable", 0, 1),
        BitField("rsvd_cfg", 0, 2),
        # etc
        BitField("rsvd_db_cnt", 0, 3),
        BitField("ring_size", 0, 5),
        LEShortField("lif_index", 0),
    ]

class EthRxQstate(Packet):
    fields_desc = eth_qstate_common_fields_desc + [
        # shadow rings
        LEShortField("comp_index", 0),
        # sta
        BitField("color", 0, 1),
        BitField("armed", 0, 1),
        BitField("rsvd_sta", 0, 6),
        BitField("lg2_desc_sz", 0, 4),
        BitField("lg2_cq_desc_sz", 0, 4),
        BitField("lg2_sg_desc_sz", 0, 4),
        BitField("sg_max_elems", 0, 4),
        BitField("__pad256", 0, 24),
        # addr
        LELongField("ring_base", 0),
        LELongField("cq_ring_base", 0),
        LELongField("sg_ring_base", 0),
        LELongField("intr_index_or_eq_addr", 0),
    ]

class EthTxQstate(Packet):
    fields_desc = eth_qstate_common_fields_desc + [
        # shadow rings
        LEShortField("comp_index", 0),
        # sta
        BitField("color", 0, 1),
        BitField("armed", 0, 1),
        BitField("rsvd_sta", 0, 6),
        BitField("lg2_desc_sz", 0, 4),
        BitField("lg2_cq_desc_sz", 0, 4),
        BitField("lg2_sg_desc_sz", 0, 4),
        BitField("__pad256", 0, 28),
        # addr
        LELongField("ring_base", 0),
        LELongField("cq_ring_base", 0),
        LELongField("sg_ring_base", 0),
        LELongField("intr_index_or_eq_addr", 0),
        # tx2
        BitField("tso_hdr_addr", 0, 52),
        BitField("tso_hdr_len", 0, 10),
        BitField("tso_hdr_rsvd", 0, 2),
        BitField("tso_ipid_delta", 0, 16),
        BitField("tso_seq_delta", 0, 32),
        BitField("__pad128_tx2", 0, 16),
        BitField("__pad512_tx2", 0, 384),
    ]


class EthEqQstate(Packet):
    fields_desc = [
        LELongField("eq_ring_base", 0),
        ByteField("eq_ring_size", 0),
        BitField("eq_enable", 0, 1),
        BitField("intr_enable", 0, 1),
        BitField("rsvd_cfg", 0, 6),
        LEShortField("eq_index", 0),
        ByteField("eq_gen", 0),
        BitField("rsvd", 0, 8),
        LEShortField("intr_index", 0),
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

def log_ring_size(value):
    assert(isinstance(value, int))
    assert(not (value & (value - 1)))
    value = log2(value)
    assert(2 <= value <= 16)
    return value

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
        data = self.data[self.__data_class__]
        assert(isinstance(ring, int))
        assert(0 <= ring <= 1)
        assert(isinstance(value, int))
        assert(0 <= value <= 2 ** data.ring_size)
        data.p_index0 = value

    def set_pindex1(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.p_index1 = value

    def set_cindex1(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.c_index1 = value

    def set_ring_size(self, value):
        data = self.data[self.__data_class__]
        data.ring_size = log_ring_size(value)

    def set_ring_base(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.ring_base = value

    def set_sg_base(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.sg_ring_base = value

    def set_cq_base(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.cq_ring_base = value

    def enable_eq(self, eq):
        data = self.data[self.__data_class__]
        eq = eq.obj_helper_q.queues[0]
        data.intr_index_or_eq_addr = eq.GetQstateAddr()
        data.host = 2
        data.armed = 0
        data.eq_enable = 1

    def disable_eq(self):
        data = self.data[self.__data_class__]
        data.intr_index_or_eq_addr = 0
        data.host = 1
        data.armed = 0
        data.eq_enable = 0

    def is_armed(self):
        data = self.data[self.__data_class__]
        return bool(data.armed)

    def set_pc(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.pc = value

    def set_host_rings(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.host = value

    def set_total_rings(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.total = value

    def set_enable(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.enable = value

    def set_color(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.color = value

    def set_host_queue(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.host_queue = value

class EthRxQstateObject(QstateObject):

    def __init__(self, addr):
        super().__init__(addr, EthRxQstate)

    def set_descriptor_size(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.lg2_desc_sz = value

    def set_cq_descriptor_size(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.lg2_cq_desc_sz = value

    def set_sg_descriptor_size(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.lg2_sg_desc_sz = value

    def set_sg_max_elems(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.sg_max_elems = value

class EthTxQstateObject(QstateObject):

    def __init__(self, addr):
        super().__init__( addr, EthTxQstate)

    def set_descriptor_size(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.lg2_desc_sz = value

    def set_cq_descriptor_size(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.lg2_cq_desc_sz = value

    def set_sg_descriptor_size(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.lg2_sg_desc_sz = value

class EthEqQstateObject(QstateObject):
    def __init__(self, addr):
        super().__init__(addr, EthEqQstate)

    def set_ring_size(self, value):
        data = self.data[self.__data_class__]
        data.eq_ring_size = log_ring_size(value)
        # also init other fields of the eq qstate
        data.eq_enable = 1
        data.eq_index = 0
        data.eq_gen = 1

    # In DOL we will treat EQ like a kind of CQ
    def set_cq_base(self, value):
        data = self.data[self.__data_class__]
        assert(isinstance(value, int))
        data.eq_ring_base = value

class AdminQstateObject(QstateObject):

    def __init__(self, addr):
        super().__init__(addr, AdminQstate)


class EthQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate = None
        self._mem = None
        self.pid = 0

    def Init(self, queue_type, spec):
        super().Init(queue_type, spec)
        self.buffers = [None] * self.size

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
            elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_EQ":
                self._qstate = EthEqQstateObject(addr=self.GetQstateAddr())
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

    def Fill(self):
        if GlobalOptions.eth_mode != "onepkt" and self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            from attrdict import AttrDict
            from infra.factory.store import FactoryStore
            ring_id = 'R0'
            ring = self.rings.Get(ring_id)
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

        #TODO get the following dynamically
        if self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            logger.info("Filling EthRxQstateObject")
            self.qstate.set_pc(1) # eth_rx_stage0 in rxdma_stage0.bin
            self.qstate.set_host_rings(1)
            self.qstate.set_total_rings(3)
            self.qstate.set_enable(1)
            self.qstate.set_color(1)
            self.qstate.set_host_queue(1)
            self.qstate.set_pindex1(0xffff)
            self.qstate.set_cindex1(0xffff)
            self.qstate.set_descriptor_size(4) #log2(sizeof(struct rxq_desc))
            self.qstate.set_cq_descriptor_size(4) #log2(sizeof(struct rxq_comp))
            self.qstate.set_sg_descriptor_size(7) #log2(sizeof(struct rxq_sg_desc))
            self.qstate.set_sg_max_elems(8) #IONIC_RX_MAX_SG_ELEMS
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_TX":
            logger.info("Filling EthTxQstateObject")
            self.qstate.set_pc(2) #eth_tx_stage0 in txdma_stage0.bin
            self.qstate.set_host_rings(1)
            self.qstate.set_total_rings(3)
            self.qstate.set_enable(1)
            self.qstate.set_color(1)
            self.qstate.set_host_queue(1)
            self.qstate.set_pindex1(0xffff)
            self.qstate.set_cindex1(0xffff)
            self.qstate.set_descriptor_size(4) #log2(sizeof(struct txq_desc))
            self.qstate.set_cq_descriptor_size(4) #log2(sizeof(struct txq_comp))
            self.qstate.set_sg_descriptor_size(8) #log2(sizeof(struct txq_sg_desc))
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
            logger.info("Filling AdminQstateObject")
            self.qstate.set_pc(3) # adminq_stage0 in txdma_stage0.bin
            self.qstate.set_host_rings(2)
            self.qstate.set_total_rings(2)
            self.qstate.set_enable(1)
            self.qstate.set_color(1)
            self.qstate.set_host_queue(1)
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_EQ":
            # not a real qstate, will do init in set_ring_size
            pass

    def Configure(self):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return

        self.obj_helper_ring.Configure()
        self.qstate.Read()

        for ring in self.obj_helper_ring.rings:
            if ring.id == 'R0':
                self.qstate.set_ring_base(ring._mem.pa)
                self.qstate.set_ring_size(ring.size)
                if self.queue_type.purpose != "LIF_QUEUE_PURPOSE_ADMIN":
                    self.qstate.set_sg_base(ring._sgmem.pa)
            elif ring.id == 'R1':
                self.qstate.set_cq_base(ring._mem.pa)
                self.qstate.set_ring_size(ring.size)
            else:
                raise NotImplementedError

        self.Fill()
        self.qstate.Write()

    def Post(self, descriptor):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return status.SUCCESS

        ring = self.rings.Get('R0')
        return ring.Post(descriptor)

    def Consume(self, descriptor):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return status.SUCCESS

        ring = self.rings.Get('R1')
        return ring.Consume(descriptor)

    def EnableEQ(self, eq):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return
        self.qstate.Read()
        self.qstate.enable_eq(eq)
        self.qstate.Write()

    def DisableEQ(self):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return
        self.qstate.Read()
        self.qstate.disable_eq()
        self.qstate.Write()

    def IsQstateArmed(self):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return False
        self.qstate.Read()
        return self.qstate.is_armed()

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
