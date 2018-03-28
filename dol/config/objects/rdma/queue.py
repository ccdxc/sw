#! /usr/bin/python3
import pdb
import math
import time

import infra.common.defs        as defs
import infra.common.objects     as objects
import config.resmgr            as resmgr
import config.objects.rdma.ring  as ring
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from config.store               import Store
from infra.common.logging       import logger
from config.objects.queue       import QueueObject

import model_sim.src.model_wrap as model_wrap
from infra.common.glopts import GlobalOptions

from scapy.all import *


class RdmaRQstate(Packet):
    name = "RdmaRQstate"
    fields_desc = [
        
        # RQCB0
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
        LEShortField("p_index2", 0),
        LEShortField("c_index2", 0),
        LEShortField("p_index3", 0),
        LEShortField("c_index3", 0),
        LEShortField("p_index4", 0),
        LEShortField("c_index4", 0),
        LEShortField("p_index5", 0),
        LEShortField("c_index5", 0),

        IntField("pt_base_addr/rq_hbm_base_addr", 0),
        
        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("congestion_mgmt_enable", 0, 1),
        BitField("state", 0, 3),
        BitField("log_rsq_size", 0, 5),
        BitField("serv_type", 0, 3),
        BitField("log_pmtu", 0xa, 5),
        
        IntField("rsq_base_addr", 0),
    
        IntField("pd", 0),

        IntField("header_template_addr", 0),

        X3BytesField("dst_qp", 0),
        BitField("read_rsp_lock", 0, 1),
        BitField("read_rsp_in_progress", 0, 1),
        BitField("rq_in_hbm", 0, 1),
        # though busy is not used in actual asm code, keeping it in 
        # dol qstate as multiple testcases are still verifying it to be 0
        BitField("busy", 0, 1), 
        BitField("rqcb0_rsvd0", 0, 4),

        X3BytesField("curr_read_rsp_psn", 0),
        ByteField("p4plus_to_p4_flags", 0),

        ByteField("header_template_size", 0),
        BitField("ring_empty_sched_eval_done", 0, 1),
        BitField("rqcb0_rsvd1", 0, 7),
        BitField("rqcb0_pad", 0, 16),

        # RQCB1
        ByteField("pc_offset", 0),
        IntField("pt_base_addr/rq_hbm_base_addr", 0),
        
        BitField("log_rq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("congestion_mgmt_enable", 0, 1),
        BitField("state", 0, 3),
        BitField("log_rsq_size", 0, 5),
        BitField("serv_type", 0, 3),
        BitField("log_pmtu", 0xa, 5),
        
        IntField("rsq_base_addr", 0),
    
        IntField("pd", 0),

        IntField("header_template_addr", 0),

        ByteField("token_id", 0),
        ByteField("nxt_to_go_token_id", 0),
        ByteField("rsq_pindex_prime", 0),                                            
        BitField("srq_enabled", 0, 1),
        BitField("cache", 0, 1),
        BitField("immdt_as_dbell", 0, 1),
        BitField("rq_in_hbm", 0, 1),
        BitField("rqcb1_rsvd0", 0, 4),

        BitField("disable_speculation", 0, 1),
        BitField("adjust_rsq_c_index_in_progress", 0, 1),
        BitField("rsq_quiesce", 0, 1),
        BitField("rqcb1_rsvd1", 0, 5),
        BitField("in_progress", 0, 1),
        BitField("rqcb1_rsvd2", 0, 7),
        LEShortField("spec_cindex", 0),

        X3BytesField("e_psn", 0),
        ByteField("adjust_rsq_c_index", 0),

        X3BytesField("msn", 0),
        ByteField("header_template_size", 0),

        LEShortField("proxy_cindex", 0),
        LEShortField("proxy_pindex", 0),

        X3BytesField("cq_id", 0),
        ByteField("rqcb1_rsvd3", 0),

        ByteField("rsq_pindex", 0),

        LongField("curr_wqe_ptr", 0),
        IntField("current_sge_offset", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),

        X3BytesField("srq_id", 0),
        BitField("rqcb1_pad", 0, 8),

        # RQCB2
        ByteField("rqcb2_rsvd0", 0),
        X3BytesField("ack_nak_psn", 0),

        ByteField("aeth_syndrome", 0),
        X3BytesField("aeth_msn", 0),

        BitField("rqcb2_pad", 0, 448),

        # RQCB3
        LongField("wrid", 0),
        LongField("va", 0),
        IntField("len", 0),
        IntField("r_key", 0),

        IntField("roce_opt_ts_value", 0),
        IntField("roce_opt_ts_echo", 0),

        ShortField("roce_opt_mss", 0),
        BitField("rqcb3_rsvd1", 0, 16),

        BitField("rqcb3_pad", 0, 224),

        #RQCB4 - RESP_TX stats
        BitField("rqcb4", 0, 512),

        #RQCB5 - RESP_RX stats
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
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
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
    
        IntField("pt_base_addr/sq_hbm_base_addr", 0),
        IntField("sqcb0_header_template_addr", 0),
        IntField("pd", 0),
        BitField("log_pmtu", 0xa, 5),
        BitField("log_sq_page_size", 0xc, 5),
        BitField("log_wqe_size", 6, 5),
        BitField("log_num_wqes", 0, 5),
        BitField("serv_type", 0, 4),

        BitField("rsvd_cfg_flags", 0, 4),
        BitField("poll_for_work", 0, 1),
        BitField("signalled_completion", 0, 1),
        BitField("disable_e2e_fc", 0, 1),
        BitField("fast_reg_enable", 0, 1),
        BitField("sq_in_hbm", 0, 1),
        BitField("congestion_mgmt_enable",0, 1),
        BitField("local_ack_timeout", 0, 5),
        BitField("ring_empty_sched_eval_done", 0, 1),

        ShortField("spec_sq_cindex", 0),

        LongField("curr_wqe_ptr", 0),
        IntField("current_sge_offset", 0),
        ByteField("current_sge_id", 0),
        ByteField("num_sges", 0),

        BitField("rsvd_state_flags", 0, 5),
        BitField("dcqcn_rl_failure", 0, 1),
        BitField("bktrack_in_progress", 0, 1),
        BitField("retry_timer_on", 0, 1),

        BitField("poll_in_progress", 0, 1),
        BitField("color", 0, 1),
        BitField("fence", 0, 1),
        BitField("li_fence", 0, 1),
        BitField("state", 0, 3),
        BitField("busy", 0, 1),

        BitField("cb1_busy", 0, 1),
        BitField("in_progress", 0, 1),
        BitField("need_credits", 0, 1),
        BitField("rsvd_cb1_flags", 0, 5),
        

        # SQCB1 
        ByteField("pc_offset", 0),
        X3BytesField("cq_id", 0),
        ShortField("sqcb1_p_index5", 0),
        ShortField("c_index5", 0),

        IntField("sqcb1_rrq_base_addr", 0),
        ByteField("sqcb1_log_rrq_size", 0),
        BitField("service", 0, 4),
        BitField("congestion_mgmt_enable", 0, 1),
        BitField("sqcb1_log_pmtu", 0xa, 5),
        BitField("rsvd", 0, 14),

        X3BytesField("sqcb1_tx_psn", 0),
        X3BytesField("sqcb1_ssn", 0),
        X3BytesField("sqcb1_lsn", 0),

        IntField("sqcb1_header_template_addr", 0),
        ByteField("sqcb1_header_template_size", 0),

        ByteField("nxt_to_go_token_id", 0),
        ByteField("token_id", 0),

        X3BytesField("e_rsp_psn", 0),
        X3BytesField("rexmit_psn", 0),
        X3BytesField("msn", 0),

        BitField("credits", 0, 5),
        BitField("nak_retry_ctr", 0, 3),
        BitField("err_retry_ctr", 0, 3),
        BitField("rsvd1", 0, 5),

        ByteField("rrqwqe_num_sges", 0),
        ByteField("rrqwqe_cur_sge_id", 0),
        IntField("rrqwqe_cur_sge_offset", 0),
        BitField("rrq_in_progress", 0, 1),
        BitField("rsvd2", 0, 7),

        BitField("pad1", 0, 112),

        #SQCB2
        X3BytesField("dst_qp", 0),
        ByteField("sqcb2_header_template_size", 0),
        IntField("sqcb2_header_template_addr", 0),

        IntField("sqcb2_rrq_base_addr", 0),
        BitField("sqcb2_log_rrq_size", 0, 5),
        BitField("log_sq_size", 0, 5),
        BitField("roce_opt_ts_enable", 0, 1),
        BitField("roce_opt_mss_enable", 0, 1),
        BitField("service", 0, 4),

        X3BytesField("sqcb2_msn", 0),
        ByteField("sqcb2_credits", 0),
        X3BytesField("sqcb2_rexmit_psn", 0),
     
        BitField("last_ack_or_req_ts", 0, 48),

        BitField("sqcb2_in_progress", 0, 1),
        BitField("sqcb2_need_credits", 0, 1),
        BitField("timer_on", 0, 1),
        BitField("local_ack_timeout", 0, 5),

        X3BytesField("tx_psn", 0),
        X3BytesField("ssn", 0),
        X3BytesField("lsn", 0),
        X3BytesField("wqe_start_psn", 0),
        ByteField("curr_op_type", 0),

        IntField("imm_data", 0),
        IntField("inv_key", 0),
        ShortField("sq_cindex", 0),
        ShortField("p_index5", 0),

        ByteField("p4plus_to_p4_flags", 0),
        X3BytesField("exp_rsp_psn", 0),

        ShortField("timestamp", 0),
        ShortField("timestamp_echo", 0),
        ShortField("mss", 0),

        BitField("pad2", 0, 8),
    ]

class RdmaCQstate(Packet):
    name = "RdmaCQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
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

        BitField("wakeup_dpath", 0, 1),
        BitField("color", 0, 1),
        BitField("wakeup_lif", 0, 12),
        BitField("wakeup_qtype", 0, 3),
        BitField("wakeup_qid", 0, 24),
        BitField("wakeup_ring_id", 0, 3),

        BitField("rsvd4", 0, 19),

        LongField("pt_pa", 0),
        LongField("pt_next_pa", 0),
        ShortField("pt_pa_index", 0),
        ShortField("pt_next_pa_index", 0),
        BitField("pad", 0, 96),
    ]

class RdmaEQstate(Packet):
    name = "RdmaEQstate"
    fields_desc = [
        ByteField("pc_offset", 0),
        ByteField("rsvd0", 0),
        BitField("cosB", 0, 4),
        BitField("cosA", 0, 4),
        ByteField("cos_sel", 0),
        ByteField("eval_last", 0),
        BitField("total", 0, 4),
        BitField("host", 0, 4),
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
        logger.info("Writing Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))

    def WriteWithDelay(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing Qstate @0x%x Type: %s size: %d with delay" % (self.addr, self.queue_type, self.size))
        model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        if GlobalOptions.rtl:
            time.sleep(10)

    def Read(self):
        if (GlobalOptions.dryrun):
            data = bytes(self.size)
            self.data = qt_params[self.queue_type]['state'](data)
            return
        self.data = qt_params[self.queue_type]['state'](model_wrap.read_mem(self.addr, self.size))
        logger.ShowScapyObject(self.data)
        logger.info("Read Qstate @0x%x Type: %s size: %d" % (self.addr, self.queue_type, self.size))
    
    def incr_pindex(self, ring, ring_size):
        assert(ring < 7)
        prev_value = self.get_pindex(ring)
        new_value = ((self.get_pindex(ring) + 1) & (ring_size - 1))
        logger.info("  incr_pindex: pre-val: %d new-val: %d ring_size %d" % (prev_value, new_value, ring_size))
        self.set_pindex(ring, new_value)

    def incr_cindex(self, ring, ring_size):
        assert(ring < 7)
        self.set_cindex(ring, ((self.get_cindex(ring) + 1) & (ring_size - 1)))

    def set_pindex(self, ring, value):
        assert(ring < 7)
        setattr(self.data, 'p_index%d' % ring, value)
        #Avoid writting Qstate/PI to ASIC, and let DB pick up updated PI (0x9)

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

    def Show(self, lgh = logger):
        lgh.ShowScapyObject(self.data) 



class RdmaQueueObject(QueueObject):
    def __init__(self):
        super().__init__()
        self._qstate    = None

    def Init(self, queue_type, spec):
        self.queue_type = queue_type
        self.id         = queue_type.GetQid()
        self.qp_e_psn   = 0      #Needed for rx multi QP scale tests to pick next psn
        self.GID(str(self.id))

        self.rings      = objects.ObjectDatabase()
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
        logger.ShowScapyObject(qstate)
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
        if not ( GlobalOptions.dryrun or GlobalOptions.cfgonly):
            # Ignore spurious ring_empty_counter memory comparison b/w RTL and SW model
            if self.queue_type.GID() == 'RDMA_RQ':
                model_wrap.eos_ignore_addr(self.GetQstateAddr() + 44, 1)

    def Show(self):
        logger.info('Queue: %s' % self.GID())
        logger.info('- type   : %s' % self.queue_type.GID())
        logger.info('- id     : %s' % self.id)

    def SetRingParams(self, ring_id, host, nic_resident, mem_handle, address, size, desc_size):
        r = self.rings.Get(ring_id)
        if r is None:
            assert(0)
        r.SetRingParams(host, nic_resident, mem_handle, address, size, desc_size)
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
