#! /usr/bin/python3
import pdb
import math
import time

import infra.common.defs        as defs
import infra.common.objects     as objects
import iris.config.resmgr            as resmgr
import iris.config.objects.nvme.ring  as ring
import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs

from iris.config.store               import Store
from infra.common.logging       import logger
from iris.config.objects.queue       import QueueObject

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts import GlobalOptions

from scapy.all import *


class NvmeSQstate(Packet):
    name = "NvmeSQstate"
    fields_desc = [
        # SQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),

        BitField("pad", 0, 416),
    ]

class NvmeCQstate(Packet):
    name = "NvmeCQstate"
    fields_desc = [
        # CQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),

        BitField("pad", 0, 416),
    ]

class NvmeARMQstate(Packet):
    name = "NvmeARMQstate"
    fields_desc = [
        # ARMQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),

        BitField("pad", 0, 416),
    ]

class NvmeSessXTSTxQstate(Packet):
    name = "NvmeSessXTSTxQstate"
    fields_desc = [
        # SessXTSTxQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),

        BitField("pad", 0, 384),
    ]

class NvmeSessDGSTTxQstate(Packet):
    name = "NvmeSessDGSTTxQstate"
    fields_desc = [
        # SessDGSTTxQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),

        BitField("pad", 0, 384),
    ]

class NvmeSessXTSRxQstate(Packet):
    name = "NvmeSessXTSRxQstate"
    fields_desc = [
        # SessXTSRxQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),

        BitField("pad", 0, 384),
    ]

class NvmeSessDGSTRxQstate(Packet):
    name = "NvmeSessDGSTRxQstate"
    fields_desc = [
        # SessDGSTRxQCB0
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
        ShortField("pid", 0),

        LEShortField("p_index0", 0),
        LEShortField("c_index0", 0),
        LEShortField("p_index1", 0),
        LEShortField("c_index1", 0),

        BitField("pad", 0, 384),
    ]

qt_params = {
    #TBD: fix the labels
    'NVME_SQ': {'state': NvmeSQstate, 'hrings': 1, 'trings': 1, 'has_label':0, 'label': '', 'prog': ''},
    'NVME_CQ': {'state': NvmeCQstate, 'hrings': 1, 'trings': 1, 'has_label':0, 'label': '', 'prog': ''},
    'NVME_ARMQ': {'state': NvmeARMQstate, 'hrings': 0, 'trings': 1, 'has_label':0, 'label': '', 'prog': ''},
    'NVME_SESS_XTS_TX': {'state': NvmeSessXTSTxQstate, 'hrings': 0, 'trings': 2, 'has_label':0, 'label': '', 'prog': ''},
    'NVME_SESS_DGST_TX': {'state': NvmeSessDGSTTxQstate, 'hrings': 0, 'trings': 2, 'has_label':0, 'label': '', 'prog': ''},
    'NVME_SESS_XTS_RX': {'state': NvmeSessXTSRxQstate, 'hrings': 0, 'trings': 2, 'has_label':0, 'label': '', 'prog': ''},
    'NVME_SESS_DGST_RX': {'state': NvmeSessDGSTRxQstate, 'hrings': 0, 'trings': 2, 'has_label':0, 'label': '', 'prog': ''},
}

class NvmeQstateObject(object):
    def __init__(self, queue_type, addr, size):
        self.queue_type = queue_type
        self.addr = addr
        self.size = size
        self.proxy_cindex = 0
        if queue_type == 'NVME_CQ':
            self.proxy_cindex_en = True
        else:
            self.proxy_cindex_en = False
        self.Read()

    def Write(self, debug = True):
        if (GlobalOptions.dryrun): return
        if debug is True:
            logger.info("Writing Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))

    def WriteWithDelay(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing Qstate @0x%x Type: %s size: %d with delay" % (self.addr, self.queue_type, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        # On RTL, write is asyncronous and taking long time to complete
        # Wait until the write succeed otherwise tests may fail in RTL (timing)
        # Read(synchrouns) in loop and wait until the read data is same as what was inteded to write
        # Be aware this logic would break if there is continous change to QStata data in hardware 
        # like spruious schdule count in qstate, etc.
        wdata = self.data[0:64]
        if GlobalOptions.rtl:
            count = 1
            while True:
                self.data = qt_params[self.queue_type]['state'](model_wrap.read_mem(self.addr, self.size))
                rdata = self.data[0:64]
                if rdata == wdata:
                    break
                # Read data is not same as we wrote, so sleep for 1 sec and try again
                time.sleep(1)
                count = count + 1
            logger.info("Qstate Write @0x%x Type: %s size: %d completed after %d secs" % (self.addr, self.queue_type, self.size, count))

    def Read(self, debug = True):
        if (GlobalOptions.dryrun):
            data = bytes(self.size)
            self.data = qt_params[self.queue_type]['state'](data)
            return
        self.data = qt_params[self.queue_type]['state'](model_wrap.read_mem(self.addr, self.size))
        if debug is True:
            logger.info("Read Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))
    
    def incr_pindex(self, ring, ring_size):
        assert(ring < 7)
        prev_value = self.get_pindex(ring)
        new_value = ((self.get_pindex(ring) + 1) & (ring_size - 1))
        logger.info("  incr_pindex: pre-val: %d new-val: %d ring_size %d" % (prev_value, new_value, ring_size))
        self.set_pindex(ring, new_value)

    def incr_cindex(self, ring, ring_size):
        assert(ring < 7)
        prev_value = self.get_cindex(ring)
        new_value = ((self.get_cindex(ring) + 1) & (ring_size - 1))
        logger.info("  incr_cindex(%d): pre-val: %d new-val: %d ring_size %d" % (ring, prev_value, new_value, ring_size))
        self.set_cindex(ring, new_value)

    def set_pindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data, 'p_index%d' % ring, value)
        #Avoid writing Qstate/PI to ASIC, and let DB pick up updated PI (0x9)

    def set_cindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data, 'c_index%d' % ring, value)
        if self.proxy_cindex_en == 1 and ring == 0:
           self.proxy_cindex = value
        #CQ will update cindex to HW using doorbell. Do not write to HW
        if self.queue_type != 'NVME_CQ':
            self.WriteWithDelay()

    #def set_ring_base(self, value):
    #    self.data.ring_base = value
    #    self.Write()

    #def set_ring_size(self, value):
    #    self.data.ring_size = value
    #    self.Write()

    def get_pindex(self, ring):
        assert(ring < 7)
        return getattr(self.data, 'p_index%d' % ring)

    def get_cindex(self, ring):
        assert(ring < 7)
        if self.proxy_cindex_en == 1 and ring == 0:
           return self.proxy_cindex
        else:
           return getattr(self.data, 'c_index%d' % ring)

    def reset_cindex(self, ring):
        assert(ring < 7)
        self.set_cindex(ring, self.get_pindex(ring))
        self.WriteWithDelay()

    def Show(self, lgh = logger):
        lgh.ShowScapyObject(self.data) 



class NvmeQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate    = None

    def Init(self, queue_type, spec):
        self.queue_type = queue_type
        self.id = queue_type.GetQid()
        self.GID(str(self.id))
        self.rings      = objects.ObjectDatabase()
        self.obj_helper_ring = ring.NvmeRingObjectHelper()
        self.obj_helper_ring.Generate(self, spec)
        self.rings.SetAll(self.obj_helper_ring.rings)

        self.Show()

    @property
    def qstate(self):
        if self._qstate is None:
            self._qstate = NvmeQstateObject(queue_type=self.queue_type.GID(), addr=self.GetQstateAddr(), size=self.queue_type.size)
        return self._qstate

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.lif_handle = 0  # HW LIF ID is not known in DOL. Assume it is filled in by hal::LifCreate.
        req_spec.type_num = self.queue_type.type
        req_spec.qid = self.id
        qt_params_entry = qt_params[self.queue_type.GID()]
        qstate = qt_params_entry["state"]()
        qstate.host = qt_params_entry["hrings"]
        qstate.total = qt_params_entry["trings"]
        logger.ShowScapyObject(qstate)
        req_spec.queue_state = bytes(qstate)
        if qt_params_entry["has_label"]:
            req_spec.label.handle = "p4plus"
            req_spec.label.prog_name = qt_params_entry["prog"]
            req_spec.label.label = qt_params_entry["label"]

    def GetQstateAddr(self):
        return self.queue_type.GetQstateAddr() + (self.id * self.queue_type.size)

    def ConfigureRings(self):
        self.obj_helper_ring.Configure()
        return

    def Show(self):
        logger.info('Queue: %s' % self.GID())
        logger.info('- type   : %s' % self.queue_type.GID())
        logger.info('- id     : %s' % self.id)

    def SetRingParams(self, ring_id, hw_ring_id, host, nic_resident, mem_handle, address, size, desc_size):
        r = self.rings.Get(ring_id)
        if r is None:
            assert(0)
        r.SetRingParams(hw_ring_id, host, nic_resident, mem_handle, address, size, desc_size)
        return

class NvmeQueueObjectHelper:
    def __init__(self):
        self.queues = []

    def Generate(self, queue_type, spec):
        for espec in spec.queues:
            for qspec in range(espec.queue.count):
                queue = NvmeQueueObject()
                queue.Init(queue_type, espec.queue)
                self.queues.append(queue)

    def Configure(self):
        for queue in self.queues:
            queue.ConfigureRings()
