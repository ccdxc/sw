#! /usr/bin/python3
import pdb
import math

import infra.common.defs        as defs
import infra.common.objects     as objects
import config.resmgr            as resmgr
import config.objects.rdma.ring  as ring
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from config.store               import Store
from infra.common.logging       import cfglogger
from config.objects.queue       import QueueObject

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts import GlobalOptions

from scapy.all import *


class RdmaRQstate(Packet):
    name = "RdmaRQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosA", 0, 4),
        BitField("cosB", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("host", 0, 4),
        BitField("total", 0, 4),
        ShortField("pid", 0),

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
    
        IntField("pt_base_addr", 0),

        IntField("rsq_base_addr", 0),

        BitField("state", 0, 3),
        BitField("log_rsq_size", 0, 5),
        ByteField("token_id", 0),
        ByteField("nxt_to_go_token_id", 0),
        ShortField("rsq_pindex_prime", 0),                                            

        BitField("log_pmtu", 0xa, 5),
        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("serv_type", 0, 3),
        BitField("srq_enabled", 0, 1),
        BitField("busy", 0, 1),
        BitField("in_progress", 0, 1),
        BitField("disable_speculation", 0, 1),
        BitField("adjust_rsq_c_index_in_progress", 0, 1),
        BitField("rsq_quiesce", 0, 1),
        BitField("cache", 0, 1),
        BitField("immdt_as_dbell", 0, 1),
        BitField("congestion_mgmt_enable", 0, 1),

        X3BytesField("e_psn", 0),
        ByteField("adjust_rsq_c_index", 0),

        X3BytesField("msn", 0),

        IntField("pd", 0),

        LEShortField("proxy_cindex", 0),
        LEShortField("spec_cindex", 0),

        # RQCB1 
        LongField("va", 0),
        IntField("len", 0),
        IntField("r_key", 0),
        LongField("wrid", 0),
        X3BytesField("cq_id", 0),
        BitField("read_rsp_in_progress", 0, 1),
        BitField("read_rsp_lock", 0, 1),
        BitField("dummy1", 0, 6),
        X3BytesField("curr_read_rsp_psn", 0),
        X3BytesField("ack_nak_psn", 0),
        ByteField("aeth_syndrome", 0),
        X3BytesField("aeth_msn", 0),
        X3BytesField("last_ack_nak_psn", 0),
        IntField("header_template_addr", 0),
        X3BytesField("dst_qp", 0),
        LongField("curr_wqe_ptr", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),
        IntField("current_sge_offset", 0),
        ByteField("p4plus_to_p4_flags", 0),
        ByteField("header_template_size", 0),

        #RQCB2
        BitField("rqcb2", 0, 512),

        #RQCB3
        IntField("roce_opt_ts_value", 0),
        IntField("roce_opt_ts_echo", 0),
        ShortField("roce_opt_mss", 0),
        BitField("rqcb3_pad", 0, 432),

        #RQCB4 - RESP_RX stats
        LongField("num_bytes", 0),
        IntField("num_pkts", 0),
        ShortField("num_send_msgs", 0),
        ShortField("num_write_msgs", 0),
        ShortField("num_read_req_msgs", 0),
        ShortField("num_atomic_fna_msgs", 0),
        ShortField("num_atomic_cswap_msgs", 0),
        ShortField("num_send_msgs_inv_rkey", 0),
        ShortField("num_send_msgs_imm_data", 0),
        ShortField("num_write_msgs_imm_data", 0),
        ShortField("num_ack_requested", 0),
        ShortField("num_ring_dbell", 0),
        ShortField("num_pkts_in_cur_msg", 0),
        ShortField("max_pkts_in_any_msg", 0),
        BitField("pad", 0, 224),
    ]

class RdmaSQstate(Packet):
    name = "RdmaSQstate"
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
    
        IntField("pt_base_addr", 0),
        BitField("log_pmtu", 0xa, 5),
        BitField("log_sq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("serv_type", 0, 4),
        ByteField("curr_op_type", 0),
        LongField("curr_wqe_ptr", 0),
        IntField("current_sge_offset", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),
        BitField("in_progress", 0, 1),
        BitField("signalled_completion", 0, 1),
        BitField("disable_e2e_fc", 0, 1),
        BitField("fast_reg_enable", 0, 1),
        BitField("fence", 0, 1),
        BitField("li_fence", 0, 1),
        BitField("retry_timer_on", 0, 1),
        BitField("bktrack_in_progress", 0, 1),
        IntField("pd", 0),
        BitField("rsvd0", 0, 6),
        BitField("congestion_mgmt_enable",0, 1),
        BitField("busy", 0, 1),
        ByteField("cb1_byte", 0),
        ShortField("spec_sq_cindex", 0),
        BitField("state", 0, 3),
        BitField("rsvd2", 0, 5),

        # SQCB1 
        IntField("rrq_base_addr", 0),
        IntField("imm_data", 0),
        IntField("inv_key", 0),
        ByteField("log_rrq_size", 0),
        X3BytesField("cq_id", 0),
        X3BytesField("dst_qp", 0),
        X3BytesField("tx_psn", 0),
        X3BytesField("ssn", 0),
        X3BytesField("lsn", 0),
        X3BytesField("msn", 0),
        BitField("rsvd1", 0, 3),
        BitField("credits", 0, 5),
        BitField("in_progress", 0, 1),
        BitField("service", 0, 4),
        BitField("timer_active", 0, 1),
        BitField("local_ack_timeout", 0, 5),
        BitField("rsvd2", 0, 5),
        ByteField("rrqwqe_num_sges", 0),
        ByteField("rrqwqe_cur_sge_id", 0),
        IntField("rrqwqe_cur_sge_offset", 0),
        IntField("header_template_addr", 0),
        ByteField("nxt_to_go_token_id", 0),
        ByteField("token_id", 0),
        X3BytesField("e_rsp_psn", 0),
        X3BytesField("rexmit_psn", 0),
        X3BytesField("wqe_start_psn", 0),
        BitField("nak_retry_ctr", 0, 3),
        BitField("err_retry_ctr", 0, 3),
        BitField("roce_opt_ts_enable", 0, 1),
        BitField("roce_opt_mss_enable", 0, 1),
        ByteField("p4plus_to_p4_flags", 0),
        ByteField("header_template_size", 0),
        ShortField("timestamp", 0),
        ShortField("timestamp_echo", 0),
        ShortField("mss", 0),
    ]

class RdmaCQstate(Packet):
    name = "RdmaCQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosA", 0, 4),
        BitField("cosB", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("host", 0, 4),
        BitField("total", 0, 4),
        ShortField("pid", 0),

        ShortField("p_index0", 0),
        ShortField("c_index0", 0),

        IntField("pt_base_addr", 0),
        BitField("log_cq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("rsvd1", 0, 1),
        X3BytesField("cq_id", 0),
        X3BytesField("eq_id", 0),
        BitField("arm", 0, 1),
        BitField("color", 0, 1),
        BitField("rsvd2", 0, 6),
        ByteField("rsvd3", 0),
        ByteField("rsvd4", 0),
        ByteField("rsvd5", 0),
        ByteField("rsvd6", 0),
        ByteField("rsvd7", 0),
        ByteField("rsvd8", 0),
        ByteField("rsvd9", 0),
    ]

class RdmaEQstate(Packet):
    name = "RdmaEQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosA", 0, 4),
        BitField("cosB", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("host", 0, 4),
        BitField("total", 0, 4),
        ShortField("pid", 0),

        ShortField("p_index0", 0),
        ShortField("c_index0", 0),

        LongField("eqe_base_addr", 0),
        IntField("int_num", 0),
        X3BytesField("eq_id", 0),
        BitField("log_num_wqes", 0, 5),
        BitField("log_wqe_size", 2, 5),
        BitField("int_enabled", 0, 1),
        BitField("color", 0, 1),
        BitField("rsvd", 0, 28),
    ]

qt_params = {
    #fix the label/program for following entry reflecting txdma params
    'RDMA_SQ': {'state': RdmaSQstate, 'hrings': 1, 'trings': 2, 'has_label':1, 'label': 'rdma_req_rx_stage0', 'prog': 'rxdma_stage0.bin'},
    'RDMA_RQ': {'state': RdmaRQstate, 'hrings': 1, 'trings': 2, 'has_label':1, 'label': 'rdma_resp_rx_stage0', 'prog': 'rxdma_stage0.bin'},
    'RDMA_CQ': {'state': RdmaCQstate, 'hrings': 1, 'trings': 1, 'has_label':0, 'label': '', 'prog': ''},
    'RDMA_EQ': {'state': RdmaEQstate, 'hrings': 1, 'trings': 1, 'has_label':0, 'label': '', 'prog': ''},
}

class RdmaQstateObject(object):
    def __init__(self, queue_type, addr, size):
        self.queue_type = queue_type
        self.addr = addr
        self.size = size
        self.Read()

    def Write(self):
        if (GlobalOptions.dryrun): return
        cfglogger.info("Writing Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))
        model_wrap.write_mem(self.addr, bytes(self.data), len(self.data))
        self.Read()

    def Read(self):
        if (GlobalOptions.dryrun):
            data = bytes(self.size)
            self.data = qt_params[self.queue_type]['state'](data)
            return
        self.data = qt_params[self.queue_type]['state'](model_wrap.read_mem(self.addr, self.size))
        self.data.show()
        cfglogger.info("Read Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))
    
    def incr_pindex(self, ring, ring_size):
        assert(ring < 7)
        self.set_pindex(ring, ((self.get_pindex(ring) + 1) & (ring_size - 1)))
        self.Write()

    def incr_cindex(self, ring, ring_size):
        assert(ring < 7)
        self.set_cindex(ring, ((self.get_cindex(ring) + 1) & (ring_size - 1)))
        self.Write()

    def set_pindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data, 'p_index%d' % ring, value)
        self.Write()

    def set_cindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data, 'c_index%d' % ring, value)
        self.Write()

    def set_ring_base(self, value):
        self.data.ring_base = value
        self.Write()

    def set_ring_size(self, value):
        self.data.ring_size = value
        self.Write()

    def get_pindex(self, ring):
        assert(ring < 7)
        return getattr(self.data, 'p_index%d' % ring)

    def get_cindex(self, ring):
        assert(ring < 7)
        return getattr(self.data, 'c_index%d' % ring)

    def get_proxy_cindex(self):
        return getattr(self.data, 'proxy_cindex')

    def reset_cindex(self, ring):
        assert(ring < 7)
        self.set_cindex(ring, self.get_pindex(ring))
        self.Write()

    def ArmCq(self):
        assert(self.queue_type == 'RDMA_CQ')
        setattr(self.data, 'arm', 1)
        self.Write()

    def Show(self, lgh = cfglogger):
        lgh.ShowScapyObject(self.data) 



class RdmaQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate    = None

    def Init(self, queue_type, spec):
        self.queue_type = queue_type
        self.id         = queue_type.GetQid()
        self.GID(str(self.id))

        self.rings      = objects.ObjectDatabase(cfglogger)
        self.obj_helper_ring = ring.RdmaRingObjectHelper()
        self.obj_helper_ring.Generate(self, spec)
        self.rings.SetAll(self.obj_helper_ring.rings)

        self.Show()

    @property
    def qstate(self):
        if self._qstate is None:
            self._qstate = RdmaQstateObject(queue_type=self.queue_type.GID(), addr=self.GetQstateAddr(), size=self.queue_type.size)
        return self._qstate

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.lif_handle = 0  # HW LIF ID is not known in DOL. Assume it is filled in by hal::LifCreate.
        req_spec.type_num = self.queue_type.type
        req_spec.qid = self.id
        qt_params_entry = qt_params[self.queue_type.GID()]
        qstate = qt_params_entry["state"]()
        qstate.host = qt_params_entry["hrings"]
        qstate.total = qt_params_entry["trings"]
        qstate.show()
        req_spec.queue_state = bytes(qstate)
        if qt_params_entry["has_label"]:
            req_spec.label.handle = "p4plus"
            req_spec.label.prog_name = qt_params_entry["prog"]
            req_spec.label.label = qt_params_entry["label"]
        #print("End of id: %d " % self.id)

    def GetQstateAddr(self):
        return self.queue_type.GetQstateAddr() + (self.id * self.queue_type.size)

    def ConfigureRings(self):
        self.obj_helper_ring.Configure()

    def Show(self):
        cfglogger.info('Queue: %s' % self.GID())
        cfglogger.info('- type   : %s' % self.queue_type.GID())
        cfglogger.info('- id     : %s' % self.id)

    def SetRingParams(self, ring_id, host, mem_handle, address, size, desc_size):
        r = self.rings.Get(ring_id)
        if r is None:
            assert(0)
        r.SetRingParams(host, mem_handle, address, size, desc_size)
        return

class RdmaQueueObjectHelper:
    def __init__(self):
        self.queues = []

    def Generate(self, queue_type, spec):
        for espec in spec.queues:
            for qspec in range(espec.queue.count):
                queue = RdmaQueueObject()
                queue.Init(queue_type, espec.queue)
                self.queues.append(queue)

    def Configure(self):
        for queue in self.queues:
            queue.ConfigureRings()
