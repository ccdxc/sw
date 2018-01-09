#! /usr/bin/python3

from infra.common.defs          import status
from infra.common.logging       import cfglogger
from infra.common.glopts        import GlobalOptions
from config.objects.queue       import QueueObject
import model_sim.src.model_wrap as model_wrap

from scapy.all import *

class EthRxQstate(Packet):
    fields_desc = [
        ByteField("pc", 0),
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
        LELongField("cq_ring_base", 0),
        LEShortField("rss_type", 0),
        LEIntField("intr_assert_addr", 0),
        BitField("color", 1, 1),
        BitField("__pad", 0, 7),
    ]


class EthTxQstate(Packet):
    fields_desc = [
        ByteField("pc", 0),
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
        LELongField("cq_ring_base", 0),
        LEIntField("intr_assert_addr", 0),
        ByteField("spurious_db_cnt", 0),
        BitField("color", 1, 1),
        BitField("__pad", 0, 7),
    ]


class AdminQstate(Packet):
    fields_desc = [
        ByteField("pc", 0),
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
    ]


class EthQstateObject(object):

    __data_class__ = None

    def __init__(self, cls, addr, size):
        self.addr = addr
        self.size = size
        self.__data_class__ = cls
        self.data = self.__data_class__(model_wrap.read_mem(self.addr, self.size))

    def Write(self, lgh = cfglogger):
        if GlobalOptions.skipverify:
            return
        lgh.info("Writing Qstate @0x%x size: %d" % (self.addr, self.size))
        model_wrap.write_mem(self.addr, bytes(self.data), len(self.data))
        self.Read(lgh)

    def Read(self, lgh = cfglogger):
        if GlobalOptions.skipverify:
            return
        data = self.__data_class__(model_wrap.read_mem(self.addr, self.size))
        lgh.ShowScapyObject(data)
        lgh.info("Read Qstate @0x%x size: %d" % (self.addr, self.size))

    def set_enable(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].enable = value
        model_wrap.write_mem(self.addr + 16, bytes(ctypes.c_uint8(value)), 1)

    def set_ring_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].ring_base = value
        model_wrap.write_mem(self.addr + 17, bytes(ctypes.c_uint64(value)), 8)

    def set_ring_size(self, value):
        assert(isinstance(value, int))
        value = int(math.log(value, 2))
        self.data[self.__data_class__].ring_size = value
        model_wrap.write_mem(self.addr + 25, bytes(ctypes.c_uint16(value)), 2)

    def set_cq_base(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].cq_base = value
        model_wrap.write_mem(self.addr + 27, bytes(ctypes.c_uint64(value)), 8)

    def set_rss_type(self, value):
        assert(isinstance(value, int))
        self.data[self.__data_class__].rss_type = value
        model_wrap.write_mem(self.addr + 35, bytes(ctypes.c_uint16(value)), 2)

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

    def set_ring_count(self, host, total):
        self.data[AdminQstate].host = host
        self.data[AdminQstate].total = total
        model_wrap.write_mem(self.addr + 5, bytes(ctypes.c_uint8(host << 4 | total)), 1)

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
        self._mem = None

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
                self._qstate = EthQstateObject(EthTxQstate, addr=self.GetQstateAddr(), size=self.queue_type.size)
            elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
                self._qstate = EthQstateObject(EthRxQstate, addr=self.GetQstateAddr(), size=self.queue_type.size)
            elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
                self._qstate = AdminQstateObject(addr=self.GetQstateAddr(), size=self.queue_type.size)
            else:
                cfglogger.critical("Unable to initialize Qstate for Queue Type %s" % self.queue_type.purpose)
                raise NotImplementedError
            cfglogger.info("Loading Qstate: Lif=%s QType=%s QID=%s Addr=0x%x Size=%s" %
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
                                                     enable=1))
            req_spec.label.prog_name = "txdma_stage0.bin"
            req_spec.label.label = "eth_tx_stage0"
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_RX":
            req_spec.queue_state = bytes(EthRxQstate(host=1, total=1,
                                                     enable=1,
                                                     rss_type=self.queue_type.lif.rss_type))
            req_spec.label.prog_name = "rxdma_stage0.bin"
            req_spec.label.label = "eth_rx_stage0"
        elif self.queue_type.purpose == "LIF_QUEUE_PURPOSE_ADMIN":
            req_spec.queue_state = bytes(AdminQstate(host=1, total=1,
                                                     enable=1))
            req_spec.label.prog_name = "txdma_stage0.bin"
            req_spec.label.label = "adminq_stage0"
        else:
            cfglogger.critical("Unable to set program information for Queue Type %s" % self.queue_type.purpose)
            raise NotImplementedError

    def ConfigureRings(self):
        if GlobalOptions.dryrun or GlobalOptions.cfgonly:
            return

        self.obj_helper_ring.Configure()
        self.descriptors = [None] * self.size
        for ring in self.obj_helper_ring.rings:
            if ring.id == 'R0':
                self.qstate.set_ring_base(ring._mem.pa)
                self.qstate.set_ring_size(ring.size)
            elif ring.id == 'R1':
                self.qstate.set_cq_base(ring._mem.pa)
            else:
                raise NotImplementedError

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
            queue.ConfigureRings()
