#! /usr/bin/python3

from scapy.all import *

import iris.config.resmgr as resmgr

import infra.factory.base as base
import infra.common.objects as objects
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap

from factory.objects.rdma import buffer as rdmabuffer

from infra.factory.store    import FactoryStore

#16B
class RdmaSqDescriptorBase(Packet):
    fields_desc = [
        LongField("wrid", 0),
        BitField("op_type_rsvd", 0, 4),
        BitField("op_type", 0, 4),
        ByteField("num_sges_or_new_user_key", 0),

        BitField("wqe_format", 0, 4),
        BitField("base_rsvd_flags", 0, 7),
        BitField("color", 0, 1),
        BitField("signalled_compl", 0, 1),
        BitField("inline_data_vld", 0, 1),
        BitField("solicited_event", 0, 1),
        BitField("fence", 0, 1),

        IntField("imm_data_or_key", 0),
    ]

#16B
class RdmaSqDescriptorSend(Packet):
    fields_desc = [
        IntField("ah_handle", 0),
        ByteField("rsvd", 0),
        X3BytesField("dst_qp", 0),
        IntField("q_key", 0),
        IntField("len", 0),
    ]

#16B
class RdmaSqDescriptorRdma(Packet):
    fields_desc = [
        LongField("va", 0),
        IntField("r_key", 0),
        IntField("len", 0),
    ]

#32B
class RdmaSqDescriptorAtomic(Packet):
    fields_desc = [
        LongField("va", 0),
        IntField("r_key", 0),
        LongField("swapdt", 0),
        LongField("cmpdt", 0),
        IntField("pad", 0),
    ]

#22B + 26B pad
class RdmaSqDescriptorBindMw(Packet):
     fields_desc = [
         LongField("va", 0),
         LongField("len", 0),
         IntField("l_key", 0),
         BitField("inv_en", 0, 1),
         BitField("rsvd_flags", 0, 7),
         ByteField("access_ctrl", 0),
         BitField("pad", 0, 208),
     ]

#40B + 8B pad
class RdmaSqDescriptorFrpmr(Packet):
     fields_desc = [
         LongField("base_va", 0),
         LongField("len", 0),
         LongField("offset", 0),
         LongField("dma_src_address", 0),
         IntField("num_pt_entries", 0),
         BitField("rsvd_inv_en", 0, 1),
         BitField("rsvd_flags", 0, 7),
         ByteField("access_ctrl", 0),
         ByteField("log_dir_size", 0),
         ByteField("log_page_size", 0),
         BitField("pad", 0, 64),
     ]

class RdmaRrqDescriptorBase(Packet):
    fields_desc = [
        BitField("read_resp_or_atomic", 0, 1),
        BitField("rsvd", 0, 7),
        ByteField("num_sges", 0),
        X3BytesField("psn", 0),
        X3BytesField("e_psn", 0),
        X3BytesField("msn", 0),
    ]

class RdmaRrqDescriptorRead(Packet):
    fields_desc = [
        IntField("len", 0),
        LongField("wqe_sge_list_ptr", 0),
        IntField("rsvd0", 0),
        IntField("rsvd1", 0),
        IntField("rsvd2", 0),
        IntField("rsvd3", 0),
        IntField("rsvd4", 0),
        IntField("rsvd5", 0),
        IntField("rsvd6", 0),
        IntField("rsvd7", 0),
        IntField("rsvd8", 0),
        BitField("base_sges", 0, 256),
    ]

class RdmaRrqDescriptorAtomic(Packet):
    fields_desc = [
        LongField("va", 0),
        IntField("len", 0),
        IntField("l_key", 0),
        ByteField("op_type", 0),
        ByteField("rsvd0", 0),
        ByteField("rsvd1", 0),
        ByteField("rsvd2", 0),
        IntField("rsvd3", 0),
        IntField("rsvd4", 0),
        IntField("rsvd5", 0),
        IntField("rsvd6", 0),
        IntField("rsvd7", 0),
        IntField("rsvd8", 0),
    ]

class RdmaRqDescriptorBase(Packet):
    fields_desc = [
        LongField("wrid", 0),
        ByteField("rsvd0", 0),
        ByteField("num_sges", 0),
        BitField("wqe_format", 0, 4),
        BitField("rsvd1", 0, 4),
        ByteField("rsvd2", 0),
        IntField("rsvd3", 0),
        LongField("rsvd4", 0),
        LongField("rsvd5", 0),
    ]

class RdmaRsqDescriptorBase(Packet):
    fields_desc = [
        BitField("read_resp_or_atomic", 0, 1),
        BitField("rsvd0", 0, 7),
        X3BytesField("psn", 0),
        LongField("rsvd1", 0),
    ]

class RdmaRsqDescriptorRead(Packet):
    fields_desc = [
        IntField("r_key", 0),
        LongField("va", 0),
        IntField("len", 0),
        IntField("offset", 0),
    ]

class RdmaRsqDescriptorAtomic(Packet):
    fields_desc = [
        IntField("r_key", 0),
        LongField("va", 0),
        LongField("orig_data", 0),
    ]

class RdmaSge(Packet):
    fields_desc = [
        LongField("va", 0),
        IntField("len", 0),
        IntField("l_key", 0),
    ]

class RdmaSgeLen8x4(Packet):
    fields_desc = [
        IntField("len", 0),
    ]

class RdmaSgeLen16x2(Packet):
    fields_desc = [
        ShortField("len", 0),
    ]

class RdmaCqDescriptorRecv(Packet):
    fields_desc = [
        LongField("wrid", 0),
        BitField("flags", 0, 3),
        BitField("op_type", 0, 5),
        X3BytesField("src_qp", 0),
        MACField("smac", ETHER_ANY),
        ShortField("vlan_tag", 0),
        IntField("imm_data_or_r_key", 0),
        IntField("status_or_length", 0),
        X3BytesField("qid", 0),
        BitField("type", 0, 3),
        BitField("rsvd", 0, 3),
        BitField("error", 0, 1),
        BitField("color", 0, 1),
    ]

class RdmaCqDescriptorSend(Packet):
    fields_desc = [
        BitField("rsvd1", 0, 32),
        IntField("msn", 0),
        BitField("rsvd2", 0, 64),
        LongField("wrid", 0),
        IntField("status", 0),
        X3BytesField("qid", 0),
        BitField("type", 0, 3),
        BitField("rsvd", 0, 3),
        BitField("error", 0, 1),
        BitField("color", 0, 1),
    ]

class RdmaCqDescriptorAdmin(Packet):
    fields_desc = [
        ShortField("wqe_id", 0),
        ByteField("op", 0),
        BitField("rsvd", 0, 136),
        ShortField("old_sq_cindex", 0),
        ShortField("old_rq_cq_cindex", 0),
        IntField("status", 0),
        X3BytesField("qid", 0),
        BitField("type", 0, 3),
        BitField("rsvd", 0, 3),
        BitField("error", 0, 1),
        BitField("color", 0, 1),
    ]

class RdmaEqDescriptor(Packet):
    fields_desc = [
        X3BytesField("qid", 0),
        BitField("code", 0, 4),
        BitField("type", 0, 3),
        BitField("color", 0, 1),
    ]

class RdmaAqDescriptorBase(Packet):
    fields_desc = [
        ByteField("op", 0),
        ByteField("type_state", 0),
        LEShortField("dbid_flags", 0),
        LEIntField("id_ver", 0),
    ]

class RdmaAqDescriptorNOP(Packet):
    fields_desc = [
        BitField("pad", 0, 448),
    ]

class RdmaAqDescriptorStats(Packet):
    fields_desc = [
        LELongField("dma_addr", 0),
        LEIntField("length", 0),
        BitField("rsvd", 0, 352),
    ]

class RdmaAqDescriptorCQ(Packet):
    fields_desc = [
        LEIntField("eq_id", 0),
        ByteField("depth_log2", 0),
        ByteField("stride_log2", 0),
        ByteField("dir_size_log2_rsvd", 0),
        ByteField("page_size_log2", 0),
        LongField("rsvd1", 0),
        LongField("rsvd2", 0),
        LongField("rsvd3", 0),
        LongField("rsvd4", 0),
        LEIntField("tbl_index", 0),
        LEIntField("map_count", 0),
        LELongField("dma_addr", 0),
    ]

class RdmaAqDescriptorMR(Packet):
    fields_desc = [
        LELongField("va", 0),
        LELongField("length", 0),
        LEIntField("pd_id", 0),
        LEShortField("access_flags", 0),
        BitField("rsvd", 0, 128),
        ByteField("dir_size_log2", 0),
        ByteField("page_size_log2", 0),
        LEIntField("tbl_index", 0),
        LEIntField("map_count", 0),
        LELongField("dma_addr", 0),
    ]

class RdmaAqDescriptorQP(Packet):
    fields_desc = [
        LEIntField("pd_id", 0),
        IntField("priv_flags", 0),
        LEIntField("sq_cq_id", 0),
        ByteField("sq_depth_log2", 0),
        ByteField("sq_stride_log2", 0),
        ByteField("sq_dir_size_log2_rsvd", 0),
        ByteField("sq_page_size_log2", 0),
        LEIntField("sq_tbl_index_xrcd_id", 0),
        LEIntField("sq_map_count", 0),
        LELongField("sq_dma_addr", 0),
        LEIntField("rq_cq_id", 0),
        ByteField("rq_depth_log2", 0),
        ByteField("rq_stride_log2", 0),
        ByteField("rq_dir_size_log2_rsvd", 0),
        ByteField("rq_page_size_log2", 0),
        LEIntField("rq_tbl_index_srq_id", 0),
        LEIntField("rq_map_count", 0),
        LELongField("rq_dma_addr", 0),
    ]

class RdmaAqDescriptorModQP(Packet):
    fields_desc = [
        IntField("attr_mask", 0),
        ByteField("dcqcn_profile", 0),
        ByteField("rsvd2", 0),
        ShortField("access_flags", 0),
        LEIntField("rq_psn", 0),
        LEIntField("sq_psn", 0),
        LEIntField("qkey_dest_qpn", 0),
        LEIntField("rate_limit_kbps", 0),
        ByteField("pmtu", 0),
        ByteField("retry", 0),
        ByteField("rnr_timer", 0),
        ByteField("retry_timeout", 0),
        ByteField("rsq_depth", 0),
        ByteField("rrq_depth", 0),
        LEShortField("pkey_id", 0),
        LEIntField("ah_id_len", 0),
        LEIntField("rsvd", 0),
        LEIntField("rrq_index", 0),
        LEIntField("rsq_index", 0),
        LELongField("dma_addr", 0),
    ]

class RdmaAqDescriptorQueryQP(Packet):
  fields_desc = [
        LELongField("hdr_dma_addr", 0),
        LEIntField("ah_id", 0),
        BitField("rsvd", 0, 224),
        LELongField("sq_dma_addr", 0),
        LELongField("rq_dma_addr", 0),
  ]

class RdmaAqDescriptorQueryAH(Packet):
    fields_desc = [
        LELongField("dma_addr", 0),
        BitField("rsvd", 0, 384),
    ]

class RdmaAqDescriptorModifyDcqcn(Packet):
    fields_desc = [
        ByteField("np_incp_802p_prio", 0),
        ByteField("np_cnp_dscp", 0),
        BitField("np_rsvd", 0, 32),
        ShortField("rp_dce_tcp_g", 0),
        IntField("rp_dce_tcp_rtt", 0),
        IntField("rp_rate_reduce_monitor_period", 0),
        IntField("rp_rate_to_set_on_first_cnp", 0),
        IntField("rp_min_rate", 0),
        ShortField("rp_initial_alpha_value", 0),
        ByteField("rp_gd", 0),
        ByteField("rp_min_dec_fac", 0),
        BitField("rp_clamp_flags", 0, 8),
        ByteField("rp_threshold", 0),
        ShortField("rp_time_reset", 0),
        IntField("rp_qp_rate", 0),
        IntField("rp_byte_reset", 0),
        IntField("rp_ai_rate", 0),
        IntField("rp_hai_rate", 0),
        LongField("rp_token_bucket_size", 0),
    ]

class RdmaSqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.sges = []

    def Init(self, spec):
        super().Init(spec)
        if hasattr(self.spec.fields, 'wrid'):
            self.wrid = self.spec.fields.wrid

    def Write(self, spec_en):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("spec_en {0}".format(spec_en))

        self.wqe_format = 0
        if spec_en == True:
            if hasattr(self.spec.fields, 'num_sges') and \
               self.spec.fields.num_sges > 2:
                if self.spec.fields.num_sges <= 8:
                    self.wqe_format = 1
                elif self.spec.fields.num_sges <= 16:
                    self.wqe_format = 2

        inline_data_vld = self.spec.fields.inline_data_vld if hasattr(self.spec.fields, 'inline_data_vld') else 0
        num_sges = self.spec.fields.num_sges if hasattr(self.spec.fields, 'num_sges') else 0
        color = self.spec.fields.color if hasattr(self.spec.fields, 'color') else 0
        fence = self.spec.fields.fence if hasattr(self.spec.fields, 'fence') else 0
        logger.info("Writing SQ Descriptor @0x%x = op_type: %d wrid: 0x%x inline_data_vld: %d num_sges: %d wqe_format: %d  color: %d fence: %d" %
                       (self.address, self.spec.fields.op_type, self.wrid, inline_data_vld, num_sges, self.wqe_format, color, fence))

        base = RdmaSqDescriptorBase(op_type=self.spec.fields.op_type, wrid=self.wrid, 
                                    imm_data_or_key = 0, num_sges_or_new_user_key = 0, #to be filled in later
                                    wqe_format=self.wqe_format,
                                    inline_data_vld = inline_data_vld, color=color, fence=fence)
        desc = base

        inline_data_len = 0
        inline_data = None

        # Make sure Inline data is specificied only for Send and Write, assert in other operations
        if hasattr(self.spec.fields, 'send'):
           logger.info("Reading Send")
           base.num_sges_or_new_user_key = num_sges
           if hasattr(self.spec.fields.send, 'imm_data'):
               base.imm_data_or_key = self.spec.fields.send.imm_data 
           elif hasattr(self.spec.fields.send, 'inv_key'):
               base.imm_data_or_key = self.spec.fields.send.inv_key 
           data_len = self.spec.fields.send.len if hasattr(self.spec.fields.send, 'len') else 0
           if inline_data_vld:
               inline_data_len = data_len
               # Create the Inline data of size provided
               inline_data = bytearray(inline_data_len)
               inline_data[0:inline_data_len] = self.spec.fields.send.inline_data[0:inline_data_len]
           send = RdmaSqDescriptorSend(len=data_len)
           desc = desc/send

        if hasattr(self.spec.fields, 'ud_send'):
           logger.info("Reading UD Send")
           base.num_sges_or_new_user_key = num_sges
           if hasattr(self.spec.fields.ud_send, 'imm_data'):
               base.imm_data_or_key = self.spec.fields.ud_send.imm_data 
           dst_qp = self.spec.fields.ud_send.dst_qp if hasattr(self.spec.fields.ud_send, 'dst_qp') else 0
           q_key = self.spec.fields.ud_send.q_key if hasattr(self.spec.fields.ud_send, 'q_key') else 0
           ah_handle = self.spec.fields.ud_send.ah_handle if hasattr(self.spec.fields.ud_send, 'ah_handle') else 0
           imm_data = self.spec.fields.ud_send.imm_data if hasattr(self.spec.fields.ud_send, 'imm_data') else 0
           data_len = self.spec.fields.ud_send.len if hasattr(self.spec.fields.ud_send, 'len') else 0
           logger.info("UD Descriptor fields: dst_qp: %d q_key: 0x%x ah_handle: 0x%x imm_data: 0x%x" % \
                       (dst_qp, q_key, ah_handle, imm_data))
           send = RdmaSqDescriptorSend(q_key=q_key, dst_qp=dst_qp, ah_handle=ah_handle, len=data_len)
           desc = desc/send

        if hasattr(self.spec.fields, 'write'):
           logger.info("Reading Write")
           base.num_sges_or_new_user_key = num_sges
           if hasattr(self.spec.fields.write, 'imm_data'):
               base.imm_data_or_key = self.spec.fields.write.imm_data 
           va = self.spec.fields.write.va if hasattr(self.spec.fields.write, 'va') else 0
           dma_len = self.spec.fields.write.len if hasattr(self.spec.fields.write, 'len') else 0
           if inline_data_vld:
               inline_data_len = dma_len
               # Create the Inline data of size provided
               inline_data = bytearray(inline_data_len)
               inline_data[0:inline_data_len] = self.spec.fields.write.inline_data[0:inline_data_len]
           r_key = self.spec.fields.write.r_key if hasattr(self.spec.fields.write, 'r_key') else 0
           write = RdmaSqDescriptorRdma(va=va, len=dma_len, r_key=r_key)
           desc = desc/write

        if hasattr(self.spec.fields, 'read'):
           logger.info("Reading Read")
           assert(inline_data_vld == 0)
           base.num_sges_or_new_user_key = num_sges
           va = self.spec.fields.read.va if hasattr(self.spec.fields.read, 'va') else 0
           data_len = self.spec.fields.read.len if hasattr(self.spec.fields.read, 'len') else 0
           r_key = self.spec.fields.read.r_key if hasattr(self.spec.fields.read, 'r_key') else 0
           read = RdmaSqDescriptorRdma(va=va, len=data_len, r_key=r_key)
           desc = desc/read

        if hasattr(self.spec.fields, 'atomic'):
           logger.info("Reading Atomic")
           assert(inline_data_vld == 0)
           base.num_sges_or_new_user_key = num_sges
           r_key = self.spec.fields.atomic.r_key if hasattr(self.spec.fields.atomic, 'r_key') else 0
           va = self.spec.fields.atomic.va if hasattr(self.spec.fields.atomic, 'va') else 0
           cmpdt = self.spec.fields.atomic.cmpdt if hasattr(self.spec.fields.atomic, 'cmpdt') else 0
           swapdt = self.spec.fields.atomic.swapdt if hasattr(self.spec.fields.atomic, 'swapdt') else 0
           atomic = RdmaSqDescriptorAtomic(r_key=r_key, va=va, cmpdt=cmpdt, swapdt=swapdt)
           desc = desc/atomic

        if hasattr(self.spec.fields, 'local_inv'):
           logger.info("Reading Local Invalidate")
           assert(inline_data_vld == 0)
           if hasattr(self.spec.fields.local_inv, 'l_key'):
               base.imm_data_or_key = self.spec.fields.local_inv.l_key

        if hasattr(self.spec.fields, 'bind_mw'):
           logger.info("Reading Bind MW")
           assert(inline_data_vld == 0)
           va = self.spec.fields.bind_mw.va if hasattr(self.spec.fields.bind_mw, 'va') else 0
           logger.info("va = 0x%x" % va)
           data_len = self.spec.fields.bind_mw.len if hasattr(self.spec.fields.bind_mw, 'len') else 0
           logger.info("len = 0x%x" % data_len)
           l_key = self.spec.fields.bind_mw.l_key if hasattr(self.spec.fields.bind_mw, 'l_key') else 0
           logger.info("l_key = 0x%x" % l_key)
           r_key = self.spec.fields.bind_mw.r_key if hasattr(self.spec.fields.bind_mw, 'r_key') else 0
           logger.info("r_key = 0x%x" % r_key)
           if hasattr(self.spec.fields.bind_mw, 'r_key'):
               base.imm_data_or_key = self.spec.fields.bind_mw.r_key 
           new_r_key_key = self.spec.fields.bind_mw.new_r_key_key if hasattr(self.spec.fields.bind_mw, 'new_r_key_key') else 0
           logger.info("new_r_key_key = 0x%x" % new_r_key_key)
           if hasattr(self.spec.fields.bind_mw, 'new_r_key_key'):
               base.num_sges_or_new_user_key = self.spec.fields.bind_mw.new_r_key_key
           access_ctrl = self.spec.fields.bind_mw.access_ctrl if hasattr(self.spec.fields.bind_mw, 'access_ctrl') else 0
           logger.info("access_ctrl = 0x%x" % access_ctrl)
           mw_type = self.spec.fields.bind_mw.mw_type if hasattr(self.spec.fields.bind_mw, 'mw_type') else 0
           inv_en = 1 if (mw_type == 2) else 0
           logger.info("mw_type = 0x%x inv_en: %d" % (mw_type, inv_en))
            
           bind_mw = RdmaSqDescriptorBindMw(va=va, len=data_len, l_key=l_key, access_ctrl=access_ctrl, inv_en=inv_en)
           desc = desc/bind_mw


        if hasattr(self.spec.fields, 'frpmr'):
           logger.info("Reading FRPMR")
           assert(inline_data_vld == 0)
           l_key = self.spec.fields.frpmr.l_key if hasattr(self.spec.fields.frpmr, 'l_key') else 0
           logger.info("l_key = 0x%x" % l_key)
           if hasattr(self.spec.fields.frpmr, 'l_key'):
               base.imm_data_or_key = self.spec.fields.frpmr.l_key 
           new_user_key = self.spec.fields.frpmr.new_user_key if hasattr(self.spec.fields.frpmr, 'new_user_key') else 0
           logger.info("new_user_key = 0x%x" % new_user_key)
           if hasattr(self.spec.fields.frpmr, 'new_user_key'):
               base.num_sges_or_new_user_key = self.spec.fields.frpmr.new_user_key
           access_ctrl = self.spec.fields.frpmr.access_ctrl if hasattr(self.spec.fields.frpmr, 'access_ctrl') else 0
           logger.info("access_ctrl = 0x%x" % access_ctrl)
           log_page_size = self.spec.fields.frpmr.log_page_size if hasattr(self.spec.fields.frpmr, 'log_page_size') else 0
           logger.info("log_page_size = 0x%x" % log_page_size)
           num_pt_entries = self.spec.fields.frpmr.num_pt_entries if hasattr(self.spec.fields.frpmr, 'num_pt_entries') else 0
           logger.info("num_pt_entries = 0x%x" % num_pt_entries)
           base_va = self.spec.fields.frpmr.base_va if hasattr(self.spec.fields.frpmr, 'base_va') else 0
           logger.info("va = 0x%x" % base_va)
           dma_src_address = self.spec.fields.frpmr.dma_src_address if hasattr(self.spec.fields.frpmr, 'dma_src_address') else 0
           logger.info("dma_src_address = 0x%x" % dma_src_address)
           data_len = self.spec.fields.frpmr.len if hasattr(self.spec.fields.frpmr, 'len') else 0
           logger.info("len = 0x%x" % data_len)

           frpmr = RdmaSqDescriptorFrpmr(access_ctrl=access_ctrl, log_page_size=log_page_size, num_pt_entries=num_pt_entries, base_va=base_va, dma_src_address=dma_src_address, len=data_len)
           desc = desc/frpmr

        if inline_data_vld:
           logger.info("Inline Data: %s " % bytes(inline_data[0:inline_data_len]))
           desc = desc/bytes(inline_data)
        elif (num_sges and (num_sges > 0)):
            if self.spec.fields.num_sges:
                # write length encoding for non-default WQE format
                if self.wqe_format == 1:
                    for i in range(8):
                        if (i >= self.spec.fields.num_sges):
                            length = 0
                        else:
                            sge = self.spec.fields.sges[i]
                            length = sge.len
                        sge_len_entry = RdmaSgeLen8x4(len=length)
                        desc = desc/sge_len_entry
                elif self.wqe_format == 2:
                    for i in range(16):
                        if (i >= self.spec.fields.num_sges):
                            length = 0
                        else:
                            sge = self.spec.fields.sges[i]
                            length = sge.len
                        sge_len_entry = RdmaSgeLen16x2(len=length)
                        desc = desc/sge_len_entry

            for sge in self.spec.fields.sges:
                sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
                logger.info("Read Sge[] = va: 0x%x len: %d l_key: %d" % 
                               (sge.va, sge.len, sge.l_key))
                desc = desc/sge_entry
        
        logger.ShowScapyObject(desc)

        logger.info("q_max_desc_size = %d desc_size = %d" \
                     %(self.q_max_desc_size, len(desc)))
        assert(len(desc) <= self.q_max_desc_size)

        if self.mem_handle:
            resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        else:
            model_wrap.write_mem_pcie(self.address, bytes(desc), len(desc))

        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        if self.mem_handle:
            self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            mem_handle = objects.MemHandle(self.address, self.phy_address)
            desc = RdmaSqDescriptorBase(resmgr.HostMemoryAllocator.read(mem_handle, 32))
        else:
            hbm_addr = self.address
            desc = RdmaSqDescriptorBase(model_wrap.read_mem(hbm_addr, 32))

        logger.ShowScapyObject(desc)
        self.wrid = desc.wrid
        self.wqe_format = desc.wqe_format
        self.op_type = desc.op_type
        if (self.op_type < 8):
            self.num_sges = desc.num_sges_or_new_user_key
        else:
            self.num_sges = 0
        self.fence   = desc.fence
        logger.info("Read Desciptor @0x%x = wrid: 0x%x wqe_format: %d num_sges: %d op_type: %d fence: %d" %
                       (self.address, self.wrid, self.wqe_format, self.num_sges, self.op_type, self.fence))
        if self.mem_handle:
            mem_handle.va += 32
            #for atomic descriptor, skip 16 bytes to access SGE
            if self.op_type in [6, 7]:
                mem_handle.va += 16
        else:
            hbm_addr += 32
            if self.op_type in [6, 7]:
                hbm_addr += 16

        self.sge_len = []
        if self.wqe_format == 1:
            for i in range(8):
                if self.mem_handle:
                    self.sge_len.append(RdmaSgeLen8x4(resmgr.HostMemoryAllocator.read(mem_handle, 4)))
                else:
                    self.sge_len.append(RdmaSgeLen8x4(model_wrap.read_mem(hbm_addr, 4)))

                if self.mem_handle:
                    mem_handle.va += 4
                else:
                    hbm_addr += 4
        elif self.wqe_format == 2:
            for i in range(16):
                if self.mem_handle:
                    self.sge_len.append(RdmaSgeLen16x2(resmgr.HostMemoryAllocator.read(mem_handle, 2)))
                else:
                    self.sge_len.append(RdmaSgeLen16x2(model_wrap.read_mem(hbm_addr, 2)))

                if self.mem_handle:
                    mem_handle.va += 2
                else:
                    hbm_addr += 2

        for sge in self.sge_len:
            logger.info('%s' % type(sge))
            logger.info('len: 0x%x' % sge.len)

        self.sges = []

        for i in range(self.num_sges):
            
            if self.mem_handle:
                self.sges.append(RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16)))
            else:
                self.sges.append(RdmaSge(model_wrap.read_mem(hbm_addr, 16)))

            #sge = RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16))
            #self.sges.append(sge)
            #logger.info("Read Sge[%d] = va: 0x%x len: %d l_key: %d" % 
            #               (i, sge.va, sge.len, sge.l_key))
            if self.mem_handle:
                mem_handle.va += 16
            else:
                hbm_addr += 16

        for sge in self.sges:
            logger.info('%s' % type(sge))
            logger.info('0x%x' % sge.va)
        logger.info('id: %s' %id(self))


    def Show(self):
        desc = RdmaSqDescriptor(wrid=self.wrid, op_type=self.op_type, num_sges=self.num_sges)
        logger.ShowScapyObject(desc)
        #TODO: Check if we need to show SGEs

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor, ignoring for now..")
        return True

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor")

        #if hasattr(self.spec.fields, 'buff'):
        if not hasattr(self, 'address'):
            logger.info("Reading from buff")
            return self.spec.fields.buff

        rdmabuff = rdmabuffer.RdmaBufferObject()
        logger.info("wrid: %d num_sges: %d len: %d" % (self.wrid, self.num_sges, len(self.sges)));
        total_data = bytearray()
        total_size = 0 
        for idx in range(self.num_sges):
            sge = self.sges[idx]
            logger.info("Reading sge content : 0x%x  len: %d l_key: %d" %(sge.va, sge.len, sge.l_key))
            if sge.l_key == 0: #ReservedLkey
                mem_handle = objects.MemHandle(resmgr.HostMemoryAllocator.p2v(sge.va), sge.va)
            else:
                mem_handle = objects.MemHandle(sge.va, resmgr.HostMemoryAllocator.v2p(sge.va))
            sge_data = resmgr.HostMemoryAllocator.read(mem_handle, sge.len)
            logger.info("     sge data: %s" % bytes(sge_data))
            total_data.extend(sge_data)
            total_size += sge.len
        rdmabuff.data = bytes(total_data)
        rdmabuff.size = total_size
        logger.info("Total data: %s" % bytes(total_data))
        return rdmabuff

class RdmaRqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.sges = []

    def Init(self, spec):
        super().Init(spec)
        if hasattr(self.spec.fields, 'wrid'):
            self.wrid = self.spec.fields.wrid

    def Write(self, spec_en):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("spec_en {0}".format(spec_en))

        self.wqe_format = 0
        if spec_en == True:
            if self.spec.fields.num_sges and \
               self.spec.fields.num_sges > 2:

                if self.spec.fields.num_sges <= 8:
                    self.wqe_format = 1
                elif self.spec.fields.num_sges <= 16:
                    self.wqe_format = 2

        if self.mem_handle:
            logger.info("Writing RQ Descriptor @(va:0x%x, pa:0x%x) = wrid: 0x%x num_sges: %d wqe_format: %d" % 
                       (self.mem_handle.va, self.mem_handle.pa, self.wrid, self.spec.fields.num_sges, self.wqe_format))
        else:
            logger.info("Writing RQ Descriptor @(address:0x%x) = wrid: 0x%x num_sges: %d wqe_format: %d" % 
                       (self.address, self.wrid, self.spec.fields.num_sges, self.wqe_format))

        desc = RdmaRqDescriptorBase(wrid=self.wrid,
                                    num_sges=self.spec.fields.num_sges, 
                                    wqe_format=self.wqe_format)

        if self.spec.fields.num_sges:
            # write length encoding for non-default WQE format

            if self.wqe_format == 1:

                for i in range(8):
                    if (i >= self.spec.fields.num_sges):
                        length = 0
                    else:
                        sge = self.spec.fields.sges[i]
                        length = sge.len
                    sge_len_entry = RdmaSgeLen8x4(len=length)
                    desc = desc/sge_len_entry

            elif self.wqe_format == 2:

                for i in range(16):
                    if (i >= self.spec.fields.num_sges):
                        length = 0
                    else:
                        sge = self.spec.fields.sges[i]
                        length = sge.len
                    sge_len_entry = RdmaSgeLen16x2(len=length)
                    desc = desc/sge_len_entry

            for sge in self.spec.fields.sges:
                logger.info("sge: va: 0x%x len: %d l_key: %d" %(sge.va, sge.len, sge.l_key))
                sge_entry = RdmaSge(va=sge.va, len=sge.len, l_key=sge.l_key)
                desc = desc/sge_entry
        
        logger.ShowScapyObject(desc)

        logger.info("q_max_desc_size = %d desc_size = %d" \
                     %(self.q_max_desc_size, len(desc)))
        assert(len(desc) <= self.q_max_desc_size)

        if self.mem_handle:
            resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        else:
            model_wrap.write_mem_pcie(self.address, bytes(desc), len(desc))

        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """

        if self.mem_handle:
            self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            mem_handle = objects.MemHandle(self.address, self.phy_address)
            desc = RdmaRqDescriptorBase(resmgr.HostMemoryAllocator.read(mem_handle, 32))
        else:
            hbm_addr = self.address
            desc = RdmaRqDescriptorBase(model_wrap.read_mem(hbm_addr, 32))

        logger.ShowScapyObject(desc)
        self.wrid = desc.wrid
        self.wqe_format = desc.wqe_format
        self.num_sges = desc.num_sges
        logger.info("Read Desciptor @0x%x = wrid: 0x%x num_sges: %d wqe_format: %d" % 
                       (self.address, self.wrid, self.num_sges, self.wqe_format))

        if self.mem_handle:
            mem_handle.va += 32
        else:
            hbm_addr += 32

        self.sge_len = []
        if self.wqe_format == 1:

            for i in range(8):
                if self.mem_handle:
                    self.sge_len.append(RdmaSgeLen8x4(resmgr.HostMemoryAllocator.read(mem_handle, 4)))
                else:
                    self.sge_len.append(RdmaSgeLen8x4(model_wrap.read_mem(hbm_addr, 4)))

                if self.mem_handle:
                    mem_handle.va += 4
                else:
                    hbm_addr += 4

        elif self.wqe_format == 2:

            for i in range(16):
                if self.mem_handle:
                    self.sge_len.append(RdmaSgeLen16x2(resmgr.HostMemoryAllocator.read(mem_handle, 2)))
                else:
                    self.sge_len.append(RdmaSgeLen16x2(model_wrap.read_mem(hbm_addr, 2)))

                if self.mem_handle:
                    mem_handle.va += 2
                else:
                    hbm_addr += 2

        for sge in self.sge_len:
            logger.info('%s' % type(sge))
            logger.info('len: 0x%x' % sge.len)

        self.sges = []

        for i in range(self.num_sges):
            
            if self.mem_handle:
                self.sges.append(RdmaSge(resmgr.HostMemoryAllocator.read(mem_handle, 16)))
            else:
                self.sges.append(RdmaSge(model_wrap.read_mem(hbm_addr, 16)))

            if self.mem_handle:
                mem_handle.va += 16
            else:
                hbm_addr += 16

        for sge in self.sges:
            logger.info('%s' % type(sge))
            logger.info('va: 0x%x' % sge.va)
            logger.info('len: 0x%x' % sge.len)
            logger.info('lkey: 0x%x' % sge.l_key)
        logger.info('id: %s' %id(self))

    def Show(self):
        desc = RdmaRqDescriptor(addr=self.wrid, len=self.num_sges)
        logger.ShowScapyObject(desc)
        #TODO: Check if we need to show SGEs

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor, ignoring for now..")
        return True

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor")

        #if hasattr(self.spec.fields, 'buff'):
        if not hasattr(self, 'address'):
            return self.spec.fields.buff

        rdmabuff = rdmabuffer.RdmaBufferObject()
        logger.info("wrid: %d num_sges: %d len: %d" % (self.wrid, self.num_sges, len(self.sges)));
        total_data = bytearray()
        total_size = 0 
        for idx in range(self.num_sges):
            sge = self.sges[idx]
            va = sge.va

            # below code is specific to zbva 
            # if RQ desc has is_zbva field set, add base_va to the va
            # here, we check spec.fields.sges because it has testspec data, and would contain 
            # is_zbva and base_va if the expected descriptor was defined as such in the testspec
            # on the other hand, self.sge wouldn't have these fields because it has contents
            # of the descriptor that we consume (same as what was written as RQ descriptor)
            spec_sge = self.spec.fields.sges[idx]
            if hasattr(spec_sge, 'is_zbva') and (spec_sge.is_zbva == 1):
                va += spec_sge.base_va
                logger.info("is_zbva: %d, base_va: %d" %(spec_sge.is_zbva, spec_sge.base_va))
            # zbva specific code done

            logger.info("Reading sge content : 0x%x  len: %d l_key: %d" %(va, sge.len, sge.l_key))
            if sge.l_key == 0: #ReservedLkey
                mem_handle = objects.MemHandle(resmgr.HostMemoryAllocator.p2v(va), va)
            else:
                mem_handle = objects.MemHandle(va, resmgr.HostMemoryAllocator.v2p(va))
            sge_data = resmgr.HostMemoryAllocator.read(mem_handle, sge.len)
            logger.info("     sge data: %s" % bytes(sge_data))
            total_data.extend(sge_data)
            total_size += sge.len
        rdmabuff.data = bytes(total_data)
        rdmabuff.size = total_size
        logger.info("Total data: %s" % bytes(total_data))
        return rdmabuff

class RdmaAqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_AQ'))

    def Init(self, spec):
        super().Init(spec)
        self.op = self.spec.fields.op if hasattr(self.spec.fields, 'op') else 0
        self.type_state = self.spec.fields.type_state if hasattr(self.spec.fields, 'type_state') else 0
        self.dbid_flags = self.spec.fields.dbid_flags if hasattr(self.spec.fields, 'dbid_flags') else 0
        self.id_ver = self.spec.fields.id_ver if hasattr(self.spec.fields, 'id_ver') else 0
        self.__create_desc()

        """
        Creates a Descriptor at "self.address"
        :return:
        """
        if hasattr(self.spec.fields, 'nop') or hasattr(self.spec.fields, 'destroy_qp'):
           logger.info("Reading Admin NOOP/Destroy QP")
           nop = RdmaAqDescriptorNOP()
           desc = self.desc/nop
           self.__set_desc(desc)

    def InitCQ(self, wqe):
        self.wqe = wqe
        self.op = wqe.op
        self.type_state = 0
        self.dbid_flags = wqe.dbid_flags
        self.id_ver = wqe.id_ver
        self.__create_desc()

        logger.info("Reading Admin CQ Create")
        cq = RdmaAqDescriptorCQ(eq_id = wqe.eq_id, depth_log2 = wqe.depth_log2,
                        stride_log2 = wqe.stride_log2, page_size_log2 = wqe.page_size_log2,
                        tbl_index = wqe.tbl_index, map_count = wqe.map_count,
                        dma_addr = wqe.dma_addr)
        desc = self.desc/cq
        self.__set_desc(desc)

    def InitMR(self, wqe):
        self.wqe = wqe
        self.op = wqe.op
        self.type_state = 0
        self.dbid_flags = wqe.dbid_flags
        self.id_ver = wqe.id_ver
        self.__create_desc()

        logger.info("Reading Admin MR Create")
        mr = RdmaAqDescriptorMR(va = wqe.va, length = wqe.length, pd_id = wqe.pd_id,
                        page_size_log2 = wqe.page_size_log2, tbl_index = wqe.tbl_index,
                        map_count = wqe.map_count, dma_addr = wqe.dma_addr)
        desc = self.desc/mr
        self.__set_desc(desc)

    def InitQP(self, wqe):
        self.wqe = wqe
        self.op = wqe.op
        self.type_state = wqe.type_state
        self.dbid_flags = wqe.dbid_flags
        self.id_ver = wqe.id_ver
        self.__create_desc()

        logger.info("Reading Admin QP Create")
        qp = RdmaAqDescriptorQP(pd_id = wqe.pd_id, priv_flags = wqe.access_perms_flags,
                        sq_cq_id = wqe.sq_cq_id, sq_depth_log2 = wqe.sq_depth_log2,
                        sq_stride_log2 = wqe.sq_stride_log2,
                        sq_dir_size_log2_rsvd = 0,
                        sq_page_size_log2 = wqe.sq_page_size_log2,
                        sq_tbl_index_xrcd_id = wqe.sq_tbl_index_xrcd_id,
                        sq_map_count = wqe.sq_map_count, sq_dma_addr = wqe.sq_dma_addr,
                        rq_cq_id = wqe.rq_cq_id, rq_depth_log2 = wqe.rq_depth_log2,
                        rq_stride_log2 = wqe.rq_stride_log2,
                        rq_dir_size_log2_rsvd = 0,
                        rq_page_size_log2 = wqe.rq_page_size_log2,
                        rq_tbl_index_srq_id = wqe.rq_tbl_index_srq_id,
                        rq_map_count = wqe.rq_map_count, rq_dma_addr = wqe.rq_dma_addr)
        desc = self.desc/qp
        self.__set_desc(desc)

    def InitModQP(self, wqe):
        self.wqe = wqe
        self.op = wqe.op
        self.type_state = wqe.type_state
        self.dbid_flags = wqe.dbid_flags
        self.id_ver = wqe.id_ver
        self.__create_desc()

        logger.info("Reading Admin QP Modify")
        mod_qp = RdmaAqDescriptorModQP(attr_mask = wqe.attr_mask, dcqcn_profile = wqe.dcqcn_profile,
                                access_flags = wqe.access_flags, rq_psn = wqe.rq_psn, sq_psn = wqe.sq_psn,
                                qkey_dest_qpn = wqe.qkey_dest_qpn, rate_limit_kbps = wqe.rate_limit_kbps,
                                pmtu = wqe.pmtu, retry = wqe.retry, rnr_timer = wqe.rnr_timer,
                                retry_timeout = wqe.retry_timeout, rsq_depth = wqe.rsq_depth, rrq_depth = wqe.rrq_depth,
                                pkey_id = wqe.pkey_id, ah_id_len = wqe.ah_id_len, rrq_index = wqe.rrq_index,
                                rsq_index = wqe.rsq_index, dma_addr = wqe.dma_addr)
        desc = self.desc/mod_qp
        self.__set_desc(desc)

    def InitModDcqcn(self, wqe):
        self.wqe = wqe
        self.op = wqe.op
        self.type_state = wqe.type_state
        self.dbid_flags = wqe.dbid_flags
        self.id_ver = wqe.id_ver
        self.__create_desc()

        logger.info("Reading Admin DCQCN Modify")
        mod_dcqcn = RdmaAqDescriptorModifyDcqcn(
                            np_incp_802p_prio = wqe.np_incp_802p_prio,
                            np_cnp_dscp = wqe.np_cnp_dscp,
                            rp_token_bucket_size = wqe.rp_token_bucket_size,
                            rp_initial_alpha_value = wqe.rp_initial_alpha_value,
                            rp_dce_tcp_g = wqe.rp_dce_tcp_g,
                            rp_dce_tcp_rtt = wqe.rp_dce_tcp_rtt,
                            rp_rate_reduce_monitor_period = wqe.rp_rate_reduce_monitor_period,
                            rp_rate_to_set_on_first_cnp = wqe.rp_rate_to_set_on_first_cnp,
                            rp_min_rate = wqe.rp_min_rate,
                            rp_gd = wqe.rp_gd,
                            rp_min_dec_fac = wqe.rp_min_dec_fac,
                            rp_clamp_flags = wqe.rp_clamp_flags,
                            rp_threshold = wqe.rp_threshold,
                            rp_time_reset = wqe.rp_time_reset,
                            rp_qp_rate = wqe.rp_qp_rate,
                            rp_byte_reset = wqe.rp_byte_reset,
                            rp_ai_rate = wqe.rp_ai_rate,
                            rp_hai_rate = wqe.rp_hai_rate)
        desc = self.desc/mod_dcqcn
        self.__set_desc(desc)

    def __create_desc(self):
        self.desc = RdmaAqDescriptorBase(
            op=self.op,
            type_state=self.type_state,
            dbid_flags=self.dbid_flags,
            id_ver=self.id_ver)

    def __set_desc(self, desc):
        self.desc = desc

    def Write(self, debug = True):
        if self.spec != None and hasattr(self.spec.fields, 'stats'):
           logger.info("Reading Admin Stats")
           dma_addr = self.spec.fields.stats.dma_addr if hasattr(self.spec.fields.stats, 'dma_addr') else 0
           length = self.spec.fields.stats.length if hasattr(self.spec.fields.stats, 'length') else 0
           stats = RdmaAqDescriptorStats(dma_addr=dma_addr, length=length)
           desc = self.desc/stats
           self.__set_desc(desc)
           logger.ShowScapyObject(stats)

        if self.spec != None and hasattr(self.spec.fields, 'query_qp'):
           logger.info("Reading Admin Query QP")
           sq_dma_addr = self.spec.fields.query_qp.sq_dma_addr if hasattr(self.spec.fields.query_qp, 'sq_dma_addr') else 0
           rq_dma_addr = self.spec.fields.query_qp.rq_dma_addr if hasattr(self.spec.fields.query_qp, 'rq_dma_addr') else 0
           hdr_dma_addr = self.spec.fields.query_qp.hdr_dma_addr if hasattr(self.spec.fields.query_qp, 'hdr_dma_addr') else 0
           ah_id = self.spec.fields.query_qp.ah_id if hasattr(self.spec.fields.query_qp, 'ah_id') else 0
           queryQp = RdmaAqDescriptorQueryQP(hdr_dma_addr=hdr_dma_addr, ah_id=ah_id, rsvd=0, sq_dma_addr=sq_dma_addr, rq_dma_addr=rq_dma_addr)
           desc = self.desc/queryQp
           self.__set_desc(desc)
           logger.ShowScapyObject(queryQp)

        if self.spec != None and hasattr(self.spec.fields, 'query_ah'):
            logger.info("Reading Admin Query AH")
            dma_addr = self.spec.fields.query_ah.dma_addr if hasattr(self.spec.fields.query_ah, 'dma_addr') else 0
            queryAh = RdmaAqDescriptorQueryAH(dma_addr=dma_addr, rsvd=0)
            desc = self.desc/queryAh
            self.__set_desc(desc)
            logger.ShowScapyObject(queryAh)

        if self.spec != None and hasattr(self.spec.fields, 'create_qp'):
            logger.info("Reading Admin Create QP")
            pd_id = self.spec.fields.create_qp.pd_id if hasattr(self.spec.fields.create_qp, 'pd_id') else 0
            priv_flags = self.spec.fields.create_qp.priv_flags if hasattr(self.spec.fields.create_qp, 'priv_flags') else 0
            sq_cq_id = self.spec.fields.create_qp.sq_cq_id if hasattr(self.spec.fields.create_qp, 'sq_cq_id') else 0
            sq_depth_log2 = self.spec.fields.create_qp.sq_depth_log2 if hasattr(self.spec.fields.create_qp, 'sq_depth_log2') else 0
            sq_stride_log2 = self.spec.fields.create_qp.sq_stride_log2 if hasattr(self.spec.fields.create_qp, 'sq_stride_log2') else 0
            sq_page_size_log2 = self.spec.fields.create_qp.sq_page_size_log2 if hasattr(self.spec.fields.create_qp, 'sq_page_size_log2') else 0
            sq_tbl_index_xrcd_id = self.spec.fields.create_qp.sq_tbl_index_xrcd_id if hasattr(self.spec.fields.create_qp, 'sq_tbl_index_xrcd_id') else 0
            sq_map_count = self.spec.fields.create_qp.sq_map_count if hasattr(self.spec.fields.create_qp, 'sq_map_count') else 0
            sq_dma_addr = self.spec.fields.create_qp.sq_dma_addr if hasattr(self.spec.fields.create_qp, 'sq_dma_addr') else 0
            rq_cq_id = self.spec.fields.create_qp.rq_cq_id if hasattr(self.spec.fields.create_qp, 'rq_cq_id') else 0
            rq_depth_log2 = self.spec.fields.create_qp.rq_depth_log2 if hasattr(self.spec.fields.create_qp, 'rq_depth_log2') else 0
            rq_stride_log2 = self.spec.fields.create_qp.rq_stride_log2 if hasattr(self.spec.fields.create_qp, 'rq_stride_log2') else 0
            rq_page_size_log2 = self.spec.fields.create_qp.rq_page_size_log2 if hasattr(self.spec.fields.create_qp, 'rq_page_size_log2') else 0
            rq_tbl_index_srq_id = self.spec.fields.create_qp.rq_tbl_index_srq_id if hasattr(self.spec.fields.create_qp, 'rq_tbl_index_srq_id') else 0
            rq_map_count = self.spec.fields.create_qp.rq_map_count if hasattr(self.spec.fields.create_qp, 'rq_map_count') else 0
            rq_dma_addr = self.spec.fields.create_qp.rq_dma_addr if hasattr(self.spec.fields.create_qp, 'rq_dma_addr') else 0
            qp = RdmaAqDescriptorQP(pd_id = pd_id, priv_flags = access_perms_flags,
                            sq_cq_id = sq_cq_id, sq_depth_log2 = sq_depth_log2,
                            sq_stride_log2 = sq_stride_log2,
                            sq_dir_size_log2_rsvd = 0,
                            sq_page_size_log2 = sq_page_size_log2,
                            sq_tbl_index_xrcd_id = sq_tbl_index_xrcd_id,
                            sq_map_count = sq_map_count, sq_dma_addr = sq_dma_addr,
                            rq_cq_id = rq_cq_id, rq_depth_log2 = rq_depth_log2,
                            rq_stride_log2 = rq_stride_log2,
                            rq_dir_size_log2_rsvd = 0,
                            rq_page_size_log2 = rq_page_size_log2,
                            rq_tbl_index_srq_id = rq_tbl_index_srq_id,
                            rq_map_count = rq_map_count, rq_dma_addr = rq_dma_addr)
            desc = self.desc/qp
            self.__set_desc(desc)
            logger.ShowScapyObject(qp)

        if self.spec != None and hasattr(self.spec.fields, 'modify_qp'):
            logger.info("Reading Admin Modify QP")
            # TODO: Read values from spec as and when relevant adminQ ModQP tests are added.
            attr_mask = self.spec.fields.modify_qp.attr_mask if hasattr(self.spec.fields.modify_qp, 'attr_mask') else 0
            dcqcn_profile = self.spec.fields.modify_qp.dcqcn_profile if hasattr(self.spec.fields.modify_qp, 'dcqcn_profile') else 0
            access_flags = self.spec.fields.modify_qp.access_flags if hasattr(self.spec.fields.modify_qp, 'access_flags') else 0
            rq_psn = self.spec.fields.modify_qp.rq_psn if hasattr(self.spec.fields.modify_qp, 'rq_psn') else 0
            sq_psn = self.spec.fields.modify_qp.sq_psn if hasattr(self.spec.fields.modify_qp, 'sq_psn') else 0
            qkey_dest_qpn = self.spec.fields.modify_qp.qkey_dest_qpn if hasattr(self.spec.fields.modify_qp, 'qkey_dest_qpn') else 0
            rate_limit_kbps = 0
            pmtu = self.spec.fields.modify_qp.pmtu if hasattr(self.spec.fields.modify_qp, 'pmtu') else 0
            retry = 0
            rnr_timer = 0
            retry_timeout = 0
            rsq_depth = 0
            rrq_depth = 0
            pkey_id = 0
            ah_id_len = self.spec.fields.modify_qp.ah_id_len if hasattr(self.spec.fields.modify_qp, 'ah_id_len') else 0
            rrq_index = 0
            rsq_index = 0
            dma_addr = self.spec.fields.modify_qp.dma_addr if hasattr(self.spec.fields.modify_qp, 'dma_addr') else 0
            mod_qp = RdmaAqDescriptorModQP(attr_mask = attr_mask, dcqcn_profile = dcqcn_profile,
                                access_flags = access_flags, rq_psn = rq_psn, sq_psn = sq_psn,
                                qkey_dest_qpn = qkey_dest_qpn, rate_limit_kbps = rate_limit_kbps,
                                pmtu = pmtu, retry = retry, rnr_timer = rnr_timer,
                                retry_timeout = retry_timeout, rsq_depth = rsq_depth, rrq_depth = rrq_depth,
                                pkey_id = pkey_id, ah_id_len = ah_id_len, rrq_index = rrq_index,
                                rsq_index = rsq_index, dma_addr = dma_addr)
            desc = self.desc/mod_qp
            self.__set_desc(desc)
            logger.ShowScapyObject(mod_qp)

        if self.spec != None and hasattr(self.spec.fields, 'modify_dcqcn'):
            logger.info("Reading Admin Modify DCQCN")
            np_incp_802p_prio = self.spec.fields.modify_dcqcn.np_incp_802p_prio \
                    if hasattr(self.spec.fields.modify_dcqcn, 'np_incp_802p_prio') else 0
            np_cnp_dscp = self.spec.fields.modify_dcqcn.np_cnp_dscp \
                    if hasattr(self.spec.fields.modify_dcqcn, 'np_cnp_dscp') else 0
            rp_token_bucket_size = self.spec.fields.modify_dcqcn.rp_token_bucket_size \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_token_bucket_size') else 0
            rp_initial_alpha_value = self.spec.fields.modify_dcqcn.rp_initial_alpha_value \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_initial_alpha_value') else 0
            rp_dce_tcp_g = self.spec.fields.modify_dcqcn.rp_dce_tcp_g \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_dce_tcp_g') else 0
            rp_dce_tcp_rtt = self.spec.fields.modify_dcqcn.rp_dce_tcp_rtt \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_dce_tcp_rtt') else 0
            rp_rate_reduce_monitor_period = self.spec.fields.modify_dcqcn.rp_rate_reduce_monitor_period \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_rate_reduce_monitor_period') else 0
            rp_rate_to_set_on_first_cnp = self.spec.fields.modify_dcqcn.rp_rate_to_set_on_first_cnp \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_rate_to_set_on_first_cnp') else 0
            rp_min_rate = self.spec.fields.modify_dcqcn.rp_min_rate \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_min_rate') else 0
            rp_gd = self.spec.fields.modify_dcqcn.rp_gd \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_gd') else 0
            rp_min_dec_fac = self.spec.fields.modify_dcqcn.rp_min_dec_fac \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_min_dec_fac') else 0
            rp_clamp_flags = self.spec.fields.modify_dcqcn.rp_clamp_flags \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_clamp_flags') else 0
            rp_threshold = self.spec.fields.modify_dcqcn.rp_threshold \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_threshold') else 0
            rp_time_reset = self.spec.fields.modify_dcqcn.rp_time_reset \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_time_reset') else 0
            rp_qp_rate = self.spec.fields.modify_dcqcn.rp_qp_rate \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_qp_rate') else 0
            rp_byte_reset = self.spec.fields.modify_dcqcn.rp_byte_reset \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_byte_reset') else 0
            rp_ai_rate = self.spec.fields.modify_dcqcn.rp_ai_rate \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_ai_rate') else 0
            rp_hai_rate = self.spec.fields.modify_dcqcn.rp_hai_rate \
                    if hasattr(self.spec.fields.modify_dcqcn, 'rp_hai_rate') else 0
            mod_dcqcn = RdmaAqDescriptorModifyDcqcn(
                            np_incp_802p_prio = np_incp_802p_prio,
                            np_cnp_dscp = np_cnp_dscp,
                            rp_token_bucket_size = rp_token_bucket_size,
                            rp_initial_alpha_value = rp_initial_alpha_value,
                            rp_dce_tcp_g = rp_dce_tcp_g,
                            rp_dce_tcp_rtt = rp_dce_tcp_rtt,
                            rp_rate_reduce_monitor_period = rp_rate_reduce_monitor_period,
                            rp_rate_to_set_on_first_cnp = rp_rate_to_set_on_first_cnp,
                            rp_min_rate = rp_min_rate,
                            rp_gd = rp_gd,
                            rp_min_dec_fac = rp_min_dec_fac,
                            rp_clamp_flags = rp_clamp_flags,
                            rp_threshold = rp_threshold,
                            rp_time_reset = rp_time_reset,
                            rp_qp_rate = rp_qp_rate,
                            rp_byte_reset = rp_byte_reset,
                            rp_ai_rate = rp_ai_rate,
                            rp_hai_rate = rp_hai_rate)
            desc = self.desc/mod_dcqcn
            self.__set_desc(desc)
            logger.ShowScapyObject(mod_dcqcn)

        if debug is True:
            logger.info("Writing AQ Desciptor @0x%x = op: %d type_state: %d dbid_flags: 0x%x id_ver: %d len: %d" %
                       (self.address, self.op, self.type_state, self.dbid_flags, self.id_ver, len(bytearray(bytes(self.desc)))))
        logger.ShowScapyObject(self.desc)
        # AQ is not NIC resident
        assert(self.mem_handle)
        resmgr.HostMemoryAllocator.write(self.mem_handle,
                                         bytes(self.desc))

    def Read(self, debug = False):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        if debug is True:
            logger.info("Reading AQ Desciptor @ 0x%x phy_address: 0x%x" % (self.address, self.phy_address))
        mem_handle = objects.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaAqDescriptorBase(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaAqDescriptorBase()))))

        if self.desc.op == 10: # AQ_OP_TYPE_QUERY_QP
            mem_handle.va += len(RdmaAqDescriptorBase())
            self.queryQP = RdmaAqDescriptorQueryQP(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaAqDescriptorQueryQP())))
            self.hdr_dma_addr = self.queryQP.hdr_dma_addr
            self.sq_dma_addr = self.queryQP.sq_dma_addr
            self.rq_dma_addr = self.queryQP.rq_dma_addr
            logger.ShowScapyObject(self.queryQP)

        elif self.desc.op == 14: # AQ_OP_TYPE_QUERY_AH
            mem_handle.va += len(RdmaAqDescriptorBase())
            self.queryAH = RdmaAqDescriptorQueryAH(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaAqDescriptorQueryAH())))
            self.dma_addr = self.queryAH.dma_addr
            logger.ShowScapyObject(self.queryAH)

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on Aq descriptor..")

        logger.info('\nself(expected):')
        self.Show()
        logger.info('\nother(actual):')
        other.Show()

        if self.desc.op != other.desc.op:
            return False

        logger.info('op matched\n')

        if self.desc.type_state != other.desc.type_state:
            return False

        logger.info('type_state matched\n')

        if self.desc.id_ver != other.desc.id_ver:
            return False

        logger.info('id_ver matched\n')

        if self.desc.dbid_flags != other.desc.dbid_flags:
            return False

        logger.info('dbid_flags matched\n')

        logger.info('AQ descriptor matched\n')
        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on AQ descriptor")

        if not hasattr(self, 'address'):
            if not hasattr(self.spec.fields, 'buff'):
                return None
            logger.info("Reading from buff")
            return self.spec.fields.buff

        if self.desc.op == 10: # AQ_OP_TYPE_QUERY_QP
            rdmabuff = rdmabuffer.RdmaBufferObject()
            total_data = bytearray()
            total_size = 0
            sq_data_len = len(rdmabuffer.RdmaQuerySqBuffer())
            rq_data_len = len(rdmabuffer.RdmaQueryRqBuffer())
            page_size = 4096     # hostmem_pg_size
            logger.info("Reading query_qp content. len: %d hdr_dma_addr: 0x%x sq_dma_addr: 0x%x rq_dma_addr: 0x%x" %(len(self.queryQP), self.hdr_dma_addr, self.sq_dma_addr, self.rq_dma_addr))

            mem_handle = objects.MemHandle(resmgr.HostMemoryAllocator.p2v(self.sq_dma_addr), self.sq_dma_addr)
            sq_data = resmgr.HostMemoryAllocator.read(mem_handle, sq_data_len)
            total_data.extend(sq_data)
            total_size += sq_data_len
            logger.info("SQ data: %s" % sq_data)

            mem_handle = objects.MemHandle(resmgr.HostMemoryAllocator.p2v(self.rq_dma_addr), self.rq_dma_addr)
            rq_data = resmgr.HostMemoryAllocator.read(mem_handle, rq_data_len)
            total_data.extend(rq_data)
            total_size += rq_data_len
            logger.info("RQ data: %s" % rq_data)

            mem_handle = objects.MemHandle(resmgr.HostMemoryAllocator.p2v(self.hdr_dma_addr), self.hdr_dma_addr)
            hdr_data = resmgr.HostMemoryAllocator.read(mem_handle, page_size)
            pkt = Ether(hdr_data)
            total_data.extend(bytes(pkt))
            total_size += len(pkt)
            logger.info("HDR data: ")
            logger.ShowScapyObject(pkt)

            rdmabuff.data = bytes(total_data)
            rdmabuff.size = total_size

            return rdmabuff

        elif self.desc.op == 14: # AQ_OP_TYPE_QUERY_AH
            rdmabuff = rdmabuffer.RdmaBufferObject()
            total_data = bytearray()
            total_size = 0
            page_size = 4096    # hostmem_pg_size
            logger.info("Reading query_ah content. len: %d dma_addr: 0x%x" %(len(self.queryAH), self.dma_addr))

            mem_handle = objects.MemHandle(resmgr.HostMemoryAllocator.p2v(self.dma_addr), self.dma_addr)
            hdr_data = resmgr.HostMemoryAllocator.read(mem_handle, page_size)
            pkt = Ether(hdr_data)
            total_data.extend(bytes(pkt))
            total_size += len(pkt)
            logger.info("HDR data: ")
            logger.ShowScapyObject(pkt)

            rdmabuff.data = bytes(total_data)
            rdmabuff.size = total_size

            return rdmabuff

        else:
            # AQ is not associated with any buffer and hence simply create
            # default RDMABuffer object so that ebuf == abuf check passes
            return rdmabuffer.RdmaBufferObject()

class RdmaCqDescriptorRecvObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_CQ_RECV'))

    def Init(self, spec):
        super().Init(spec)
        self.wrid = self.spec.fields.wrid if hasattr(self.spec.fields, 'wrid') else 0
        self.flags = self.spec.fields.flags if hasattr(self.spec.fields, 'flags') else 0
        self.op_type = self.spec.fields.op_type if hasattr(self.spec.fields, 'op_type') else 0
        self.src_qp = self.spec.fields.src_qp if hasattr(self.spec.fields, 'src_qp') else 0
        self.smac = self.spec.fields.smac if hasattr(self.spec.fields, 'smac') else 0
        self.vlan_tag = self.spec.fields.vlan_tag if hasattr(self.spec.fields, 'vlan_tag') else 0
        self.imm_data_or_r_key = self.spec.fields.imm_data if hasattr(self.spec.fields, 'imm_data') else self.spec.fields.r_key if hasattr(self.spec.fields, 'r_key') else 0
        self.status_or_length = self.spec.fields.status if hasattr(self.spec.fields, 'status') else self.spec.fields.length if hasattr(self.spec.fields, 'length') else 0
        self.qid = self.spec.fields.qid if hasattr(self.spec.fields, 'qid') else 0
        self.type = self.spec.fields.type if hasattr(self.spec.fields, 'type') else 1 #CQE_TYPE_RECV
        self.rsvd = 0
        self.error = self.spec.fields.error if hasattr(self.spec.fields, 'error') else 0
        self.color = self.spec.fields.color if hasattr(self.spec.fields, 'color') else 0
        #CQE_TYPE_RECV
        assert(self.type == 1)

        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaCqDescriptorRecv(
            wrid=self.wrid,
            flags=self.flags,
            op_type=self.op_type,
            src_qp=self.src_qp,
            smac=self.smac,
            vlan_tag=self.vlan_tag,
            imm_data_or_r_key=self.imm_data_or_r_key,
            qid=self.qid,
            status_or_length=self.status_or_length,
            rsvd=self.rsvd,
            type=self.type,
            error=self.error,
            color=self.color)
        
    def __set_desc(self, desc):
        self.desc = desc
    
    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("Writing CQ(Recv) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        resmgr.HostMemoryAllocator.write(self.mem_handle, 
                                         bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        logger.info("Reading CQ(Recv) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = objects.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaCqDescriptorRecv(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaCqDescriptorRecv()))))

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on CQ(Recv) descriptor..")

        logger.info('self(expected):')
        self.Show()
        logger.info('other(actual):')
        other.Show()

        if self.desc.error == 0 and other.desc.error == 0: #CQ_STATUS_SUCCESS
            return self.desc == other.desc

        logger.info('error is not 0\n')

        if self.desc.error != other.desc.error:
            return False

        logger.info('error matched\n')

        if self.desc.wrid != other.desc.wrid:
            return False

        logger.info('wrid matched\n')

        if self.desc.flags != other.desc.flags:
            return False

        logger.info('flags matched\n')

        if self.desc.op_type != other.desc.op_type:
            return False

        logger.info('op_type matched\n')

        if self.desc.status_or_length != other.desc.status_or_length:
            return False

        logger.info('status_or_length matched\n')

        #Do not verify other fields if error is set
        if self.desc.error == 1:
           return True

        if self.desc.src_qp != other.desc.src_qp:
            return False

        logger.info('src_qp matched\n')

        if self.desc.smac != other.desc.smac:
            return False

        logger.info('smac matched\n')

        if self.desc.vlan_tag != other.desc.vlan_tag:
            return False

        logger.info('vlan_tag matched\n')

        if self.desc.imm_data_or_r_key != other.desc.imm_data_or_r_key:
            return False

        logger.info('imm_data_or_r_key matched\n')

        if self.desc.qid != other.desc.qid:
            return False

        logger.info('qid matched\n')

        if self.desc.type != other.desc.type:
            return False

        logger.info('type matched\n')

        if self.desc.color != other.desc.color:
            return False

        logger.info('color matched\n')

        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on CQ(Recv) descriptor")
        # CQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()


class RdmaCqDescriptorSendObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_CQ_SEND'))

    def Init(self, spec):
        super().Init(spec)
        self.wrid = self.spec.fields.wrid if hasattr(self.spec.fields, 'wrid') else 0
        self.msn = self.spec.fields.msn if hasattr(self.spec.fields, 'msn') else 0
        self.status = self.spec.fields.status if hasattr(self.spec.fields, 'status') else 0
        self.qid = self.spec.fields.qid if hasattr(self.spec.fields, 'qid') else 0
        self.type = self.spec.fields.type if hasattr(self.spec.fields, 'type') else 2 #CQE_TYPE_SEND_MSN
        self.rsvd = 0
        self.error = self.spec.fields.error if hasattr(self.spec.fields, 'error') else 0
        self.color = self.spec.fields.color if hasattr(self.spec.fields, 'color') else 0
        #CQE_TYPE_SEND_MSN or CQE_TYPE_SEND_NPG
        logger.info("CQ Descriptor type: %d" % self.type)
        assert(self.type == 2 or self.type == 3)

        #CQE_TYPE_SEND_MSN and msn go together
        if self.type == 2:
           assert(hasattr(self.spec.fields, 'msn'))

        #CQE_TYPE_SEND_NPG and wrid go together
        if self.type == 3:
           assert(hasattr(self.spec.fields, 'wrid'))
           #keep the wrid to 16 bit value
           assert(self.wrid < 65536)

        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaCqDescriptorSend(
            wrid=self.wrid,
            msn=self.msn,
            qid=self.qid,
            status=self.status,
            type=self.type,
            error=self.error,
            color=self.color)
        
    def __set_desc(self, desc):
        self.desc = desc
    
    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("Writing CQ(Send) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        resmgr.HostMemoryAllocator.write(self.mem_handle, 
                                         bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        logger.info("Reading CQ(Send) Desciptor @0x%x = wrid: 0x%x " % 
                       (self.address, self.wrid))
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = objects.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaCqDescriptorSend(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaCqDescriptorSend()))))

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on CQ(Send) descriptor..")

        logger.info('self(expected):')
        self.Show()
        logger.info('other(actual):')
        other.Show()

        if self.desc.error == 0 and other.desc.error == 0: #CQ_STATUS_SUCCESS
            return self.desc == other.desc

        logger.info('error is not 0\n')

        if self.desc.error != other.desc.error:
            return False

        logger.info('error matched\n')

        if self.desc.wrid != other.desc.wrid:
            return False

        logger.info('wrid matched\n')

        if self.desc.msn != other.desc.msn:
            return False

        logger.info('msn matched\n')

        if self.desc.qid != other.desc.qid:
            return False

        logger.info('qid matched\n')

        if self.desc.type != other.desc.type:
            return False

        logger.info('type matched\n')

        if self.desc.status != other.desc.status:
            return False

        logger.info('status matched\n')

        if self.desc.color != other.desc.color:
            return False

        logger.info('color matched\n')

        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on CQ(Send) descriptor")
        # CQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()

class RdmaCqDescriptorAdminObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_CQ_ADMIN'))

    def Init(self, spec):
        super().Init(spec)
        self.wqe_id = self.spec.fields.wqe_id if hasattr(self.spec.fields, 'wqe_id') else 0
        self.op = self.spec.fields.op if hasattr(self.spec.fields, 'op') else 0
        self.old_sq_cindex = self.spec.fields.old_sq_cindex if hasattr(self.spec.fields, 'old_sq_cindex') else 0
        self.old_rq_cq_cindex = self.spec.fields.old_rq_cq_cindex if hasattr(self.spec.fields, 'old_rq_cq_cindex') else 0
        self.status = self.spec.fields.status if hasattr(self.spec.fields, 'status') else 0
        self.qid = self.spec.fields.qid if hasattr(self.spec.fields, 'qid') else 0
        self.type = self.spec.fields.type if hasattr(self.spec.fields, 'type') else 0 #CQE_TYPE_ADMIN
        self.error = self.spec.fields.error if hasattr(self.spec.fields, 'error') else 0
        self.color = self.spec.fields.color if hasattr(self.spec.fields, 'color') else 0
        self.rsvd = 0
        logger.info("CQ Descriptor type: %d" % self.type)
        assert(self.type == 0)  #CQE_TYPE_ADMIN

        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaCqDescriptorAdmin(
            wqe_id=self.wqe_id,
            op=self.op,
            old_sq_cindex=self.old_sq_cindex,
            old_rq_cq_cindex=self.old_rq_cq_cindex,
            qid=self.qid,
            status=self.status,
            type=self.type,
            error=self.error,
            color=self.color)

    def __set_desc(self, desc):
        self.desc = desc

    def Write(self, debug = False):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        if debug is True:
            logger.info("Writing CQ(Admin) Desciptor @0x%x = wrid: 0x%x " %
                       (self.address, self.wrid))
        resmgr.HostMemoryAllocator.write(self.mem_handle,
                                         bytes(self.desc))

    def Read(self, debug = False):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        if debug is True:
            logger.info("Reading CQ(Admin) Desciptor @0x%x = old_sq_cindex: %d " %
                       (self.address, self.old_sq_cindex))
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = objects.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaCqDescriptorAdmin(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaCqDescriptorAdmin()))))

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on CQ(Admin) descriptor..")

        logger.info('self(expected):')
        self.Show()
        logger.info('other(actual):')
        other.Show()

        if self.desc.error == 0 and other.desc.error == 0: #CQ_STATUS_SUCCESS
            return self.desc == other.desc

        logger.info('error is not 0\n')

        if self.desc.error != other.desc.error:
            return False

        logger.info('error matched\n')

        if self.desc.old_sq_cindex != other.desc.old_sq_cindex:
            return False

        logger.info('old_sq_cindex matched\n')

        if self.desc.old_rq_cq_cindex != other.desc.old_rq_cq_cindex:
            return False

        logger.info('old_rq_cq_cindex matched\n')

        if self.desc.qid != other.desc.qid:
            return False

        logger.info('qid matched\n')

        if self.desc.type != other.desc.type:
            return False

        logger.info('type matched\n')

        if self.desc.status != other.desc.status:
            return False

        logger.info('status matched\n')

        if self.desc.color != other.desc.color:
            return False

        logger.info('color matched\n')

        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on CQ(Admin) descriptor")
        # CQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()


class RdmaEqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(FactoryStore.templates.Get('DESC_RDMA_EQ'))

    def Init(self, spec):
        super().Init(spec)
        if hasattr(spec.fields, 'qid'):
            self.qid = spec.fields.qid
        if hasattr(spec.fields, 'type'):
            self.type = spec.fields.type
        if hasattr(spec.fields, 'code'):
            self.code = spec.fields.code
        if hasattr(spec.fields, 'color'):
            self.color = spec.fields.color
        self.__create_desc()

    def __create_desc(self):
        self.desc = RdmaEqDescriptor(
            qid=self.qid,
            type=self.type,
            code=self.code,
            color=self.color)
        
    def __set_desc(self, desc):
        self.desc = desc
    
    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        logger.info("Writing EQ Desciptor @0x%x = qid: %d " % 
                       (self.address, self.qid))
        resmgr.HostMemoryAllocator.write(self.mem_handle, 
                                         bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        logger.info("Reading EQ Desciptor @ 0x%x " % (self.address))
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = objects.MemHandle(self.address, self.phy_address)
        self.__set_desc(RdmaEqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, len(RdmaEqDescriptor()))))

    def Show(self):
        logger.ShowScapyObject(self.desc)

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on Eq descriptor..")

        logger.info('\nself(expected):')
        self.Show()
        logger.info('\nother(actual):')
        other.Show()

        if self.desc.qid != other.desc.qid:
            return False

        logger.info('qid matched\n')

        if self.desc.color != other.desc.color:
            return False

        logger.info('color matched\n')

        if self.desc.type != other.desc.type:
            return False

        logger.info('type matched\n')

        if self.desc.code != other.desc.code:
            return False

        logger.info('code matched\n')

        logger.info('EQ descriptor matched\n')
        return True

        #no need to compare other params as they are meaningful only incase of SUCCESS


    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on EQ descriptor")
        # EQ is not associated with any buffer and hence simply create
        # default RDMABuffer object so that ebuf == abuf check passes
        return rdmabuffer.RdmaBufferObject()

