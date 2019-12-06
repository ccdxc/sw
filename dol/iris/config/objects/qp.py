#! /usr/bin/python3
import pdb

import scapy.all                as scapy
import model_sim.src.model_wrap as model_wrap

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
from iris.config.store               import Store
from infra.common.logging       import logger

import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2

import iris.config.objects.slab      as slab
import iris.config.objects.mr        as mr

import iris.config.objects.rdma.adminapi    as adminapi

from factory.objects.rdma.descriptor import RdmaSqDescriptorBase
from factory.objects.rdma.descriptor import RdmaSqDescriptorSend
from factory.objects.rdma.descriptor import RdmaRqDescriptorBase
from factory.objects.rdma.descriptor import RdmaRrqDescriptorBase
from factory.objects.rdma.descriptor import RdmaRrqDescriptorRead
from factory.objects.rdma.descriptor import RdmaRrqDescriptorAtomic
from factory.objects.rdma.descriptor import RdmaRsqDescriptorBase
from factory.objects.rdma.descriptor import RdmaRsqDescriptorRead
from factory.objects.rdma.descriptor import RdmaRsqDescriptorAtomic
from factory.objects.rdma.descriptor import RdmaSge
from factory.objects.rdma.descriptor import RdmaSgeLen8x4
from factory.objects.rdma.descriptor import RdmaSgeLen16x2

from infra.common.glopts import GlobalOptions

# asm/rdma/common/include/types.h
AH_ENTRY_T_SIZE_BYTES = 72
DCQCN_CB_T_SIZE_BYTES = 64
AT_ENTRY_SIZE_BYTES = 200
ROME_CB_T_SIZE_BYTES = 64

class QpObject(base.ConfigObjectBase):
    def __init__(self, pd, qp_id, spec, sges):
        super().__init__()
        self.Clone(Store.templates.Get('QPAIR_RDMA'))
        self.pd = pd
        self.remote = pd.remote
        self.id = qp_id
        self.GID("QP%04d" % self.id)
        self.spec = spec
        self.lif = pd.ep.intf.lif if self.remote is False else None

        self.pd = pd
        self.svc = spec.service

        self.pmtu = spec.pmtu
        self.hostmem_pg_size = spec.hostmem_pg_size
        self.atomic_enabled = spec.atomic_enabled
        self.sq_in_nic = spec.sq_in_nic
        self.rq_in_nic = spec.rq_in_nic
        self.modify_qp_oper = 0
        self.qstate = 0
        self.q_key = 0
        self.dst_qp = 0
        self.sq_tbl_pos = -1
        self.rq_tbl_pos = -1
        self.rsq_tbl_pos = -1
        self.rrq_tbl_pos = -1
        self.HdrTemplate = None

        # we can use the following attributes to filter testcases
        self.tiny = False
        self.max_sge_8 = False
        self.max_sge_16 = False
        self.spec_en = True

        if sges <= 2:
            self.tiny = True
        elif sges > 2 and sges <=4:
            self.tiny = False
        elif sges > 4 and sges <=8:
            self.max_sge_8 = True
        elif sges > 8 and sges <=16:
            self.max_sge_8 = True
            self.max_sge_16 = True
        elif sges > 16:
            self.spec_en = False
            self.max_sge_8 = True
            self.max_sge_16 = True

        # Pass the same spec_en value to both SQ and RQ
        self.flags = (self.spec_en << 8) | (self.spec_en << 9) | (spec.sq_in_nic << 13) | (spec.rq_in_nic << 14)

        self.num_sq_sges = sges
        self.num_sq_wqes = self.__roundup_to_pow_2(spec.num_sq_wqes)
        self.num_rrq_wqes = self.__roundup_to_pow_2(spec.num_rrq_wqes)
        self.log_sq_size = self.__get_log_size(self.num_sq_wqes)

        self.num_rq_sges = sges
        self.num_rq_wqes = self.__roundup_to_pow_2(spec.num_rq_wqes)
        self.num_rsq_wqes = self.__roundup_to_pow_2(spec.num_rsq_wqes)
        self.log_rq_size = self.__get_log_size(self.num_rq_wqes)

        self.sqwqe_size = self.__get_sqwqe_size()
        self.rqwqe_size = self.__get_rqwqe_size()
        self.rrqwqe_size = self.__get_rrqwqe_size()
        self.rsqwqe_size = self.__get_rsqwqe_size()

        self.sq_size = self.num_sq_wqes * self.sqwqe_size
        self.rq_size = self.num_rq_wqes * self.rqwqe_size
        self.rrq_size = self.num_rrq_wqes * self.rrqwqe_size
        self.rsq_size = self.num_rsq_wqes * self.rsqwqe_size

        if not self.remote:
            self.sq = pd.ep.intf.lif.GetQ('RDMA_SQ', self.id)
            self.rq = pd.ep.intf.lif.GetQ('RDMA_RQ', self.id)
            
            if (self.sq is None or self.rq is None):
                assert(0)

            self.cq_id = pd.ep.cqs.Get("CQ%04d" % self.id).id
            self.eq_id = pd.ep.cqs.Get("CQ%04d" % self.id).eq_id
            #logger.info('QP: %s PD: %s Remote: %s intf: %s lif: %s' %(self.GID(), self.pd.GID(), self.remote, pd.ep.intf.GID(), pd.ep.intf.lif.GID()))

            self.tx = pd.ep.intf.lif.GetQt('TX')
            self.rx = pd.ep.intf.lif.GetQt('RX')
            
            if (self.tx is None or self.rx is None):
                assert(0)

            # for now map both sq/rq cq to be same
            self.sq_cq = pd.ep.intf.lif.GetQ('RDMA_CQ', self.cq_id)
            self.rq_cq = pd.ep.intf.lif.GetQ('RDMA_CQ', self.cq_id)
            if (self.sq_cq is None or self.rq_cq is None):
                assert(0)
    
            # allocating one EQ for one PD
            self.eq = pd.ep.intf.lif.GetQ('RDMA_EQ', self.eq_id)
            if (self.eq is None):
                assert(0)
    
            # create sq/rq slabs
            self.sq_slab = slab.SlabObject(self.pd.ep.intf.lif, self.sq_size)
            self.rq_slab = slab.SlabObject(self.pd.ep.intf.lif, self.rq_size)
            self.hdr_slab = slab.SlabObject(self.pd.ep.intf.lif, 0)
            self.pd.ep.AddSlab(self.sq_slab)
            self.pd.ep.AddSlab(self.rq_slab)
            self.pd.ep.AddSlab(self.hdr_slab)

            # create sq/rq mrs
            #self.sq_mr = mr.MrObject(self.pd, self.sq_slab)
            #self.rq_mr = mr.MrObject(self.pd, self.rq_slab)
            #self.pd.AddMr(self.sq_mr)
            #self.pd.AddMr(self.rq_mr)

        if (GlobalOptions.dryrun):
            dcqcn_data = bytes(DCQCN_CB_T_SIZE_BYTES)
            self.dcqcn_data = RdmaDCQCNstate(dcqcn_data)
        
        self.Show()

        return

    def __roundup_to_pow_2(self, x):
        power = 1
        while power < x : 
            power *= 2
        return power
 
    def __get_sqwqe_size(self):
        return  self.__roundup_to_pow_2(
                len(RdmaSqDescriptorBase()) + 
                len(RdmaSqDescriptorSend()) +
                (self.num_sq_sges * len(RdmaSge()))) 

    def __get_rqwqe_size(self):
        if self.num_rq_sges > 2 and self.num_rq_sges <= 8:
            return self.__roundup_to_pow_2(
                    len(RdmaRqDescriptorBase()) +
                    (self.num_rq_sges * len(RdmaSge())) +
                    (8 * len(RdmaSgeLen8x4())))
        elif self.num_rq_sges > 8 and self.num_rq_sges <= 16:
            return self.__roundup_to_pow_2(
                    len(RdmaRqDescriptorBase()) +
                    (self.num_rq_sges * len(RdmaSge())) +
                    (16 * len(RdmaSgeLen16x2())))
        else:
            return self.__roundup_to_pow_2(
                    len(RdmaRqDescriptorBase()) +
                    (self.num_rq_sges * len(RdmaSge())))

    def __get_rrqwqe_size(self):
        return self.__roundup_to_pow_2(
                len(RdmaRrqDescriptorBase()) + 
                len(RdmaRrqDescriptorRead()))

    def __get_rsqwqe_size(self):
        return self.__roundup_to_pow_2(
                len(RdmaRsqDescriptorBase()) + 
                len(RdmaRsqDescriptorRead()))

    def Show(self):
        logger.info('QP: %s PD: %s Remote: %s' %(self.GID(), self.pd.GID(), self.remote))
        logger.info('SQ num_sges: %d num_wqes: %d wqe_size: %d' %(self.num_sq_sges, self.num_sq_wqes, self.sqwqe_size)) 
        logger.info('RQ num_sges: %d num_wqes: %d wqe_size: %d' %(self.num_rq_sges, self.num_rq_wqes, self.rqwqe_size)) 
        logger.info('RRQ num_wqes: %d wqe_size: %d' %(self.num_rrq_wqes, self.rrqwqe_size)) 
        logger.info('RSQ num_wqes: %d wqe_size: %d' %(self.num_rsq_wqes, self.rsqwqe_size)) 
        logger.info('SGE max 8: %d SGE max 16: %d spec_en: %d' %(self.max_sge_8, self.max_sge_16, self.spec_en)) 
        if not self.remote:
            logger.info('SQ_CQ: %s RQ_CQ: %s' %(self.sq_cq.GID(), self.rq_cq.GID()))
            logger.info('CQ ID: %d EQ ID: %d' %(self.cq_id, self.eq_id))

    def set_access_flags(self, remote_write, remote_read, remote_atomic):
        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_ACCESS_FLAGS
        mask = (remote_write & 1) | ((remote_read & 1) << 1) | ((remote_atomic & 1) << 2)
        self.flags = self.flags | mask
        logger.info(" RdmaQpUpdate Oper: SET_ACCESS_FLAGS access_flags: %d " %
                        (mask))
        if (GlobalOptions.dryrun): return

    def set_dst_qp(self, value):
        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_DEST_QPN
        self.dst_qp = value
        logger.info(" RdmaQpUpdate Oper: SET_DEST_QPN dst_qp: %d " %
                      (self.dst_qp))
        if (GlobalOptions.dryrun): return

        #adminapi.ModifyQps(self.lif, [self])
        #self.modify_qp_oper = 0

    def set_q_key(self, value):
        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_QKEY
        self.q_key = value
        logger.info(" RdmaQpUpdate Oper: SET_QKEY q_key: %d "% (self.q_key))
        if (GlobalOptions.dryrun): return

        #adminapi.ModifyQps(self.lif, [self])
        #self.modify_qp_oper = 0

    def set_q_state(self, value):
        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_STATE
        self.qstate = value
        logger.info(" RdmaQpUpdate Oper: SET_STATE qstate: %d "% (self.qstate))
        if (GlobalOptions.dryrun): return

        adminapi.ModifyQps(self.lif, [self])
        self.modify_qp_oper = 0

    def set_pmtu(self, value = None):
        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_PATH_MTU
        if value is not None:
            self.pmtu = value
        logger.info(" RdmaQpUpdate Oper: SET_PATH_MTU pmtu: %d "% (self.pmtu))
        if (GlobalOptions.dryrun): return

        #adminapi.ModifyQps(self.lif, [self])
        #self.modify_qp_oper = 0

    def set_rsq_wqes(self, flip = False):
        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_MAX_DEST_RD_ATOMIC
        if flip:
            temp = self.num_rrq_wqes
            self.num_rrq_wqes = self.num_rsq_wqes
            self.num_rsq_wqes = temp
        pages = self.rsq_size + self.sq_slab.GetPageSize()
        self.rsq_tbl_pos = self.pd.ep.intf.lif.GetRdmaTblPos(int(pages/self.sq_slab.GetPageSize()))
        logger.info(" RdmaQpUpdate Oper: SET_MAX_DEST_RD_ATOMIC rsq_index: %d "%
                        (self.rsq_tbl_pos))
        if (GlobalOptions.dryrun): return

        #adminapi.ModifyQps(self.lif, [self])
        #self.modify_qp_oper = 0

    def set_rrq_wqes(self, flip = False):
        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_MAX_QP_RD_ATOMIC
        if flip:
            temp = self.num_rrq_wqes
            self.num_rrq_wqes = self.num_rsq_wqes
        pages = self.rrq_size + self.sq_slab.GetPageSize()
        self.rrq_tbl_pos = self.pd.ep.intf.lif.GetRdmaTblPos(int(pages/self.sq_slab.GetPageSize()))
        logger.info(" RdmaQpUpdate Oper: SET_MAX_QP_RD_ATOMIC rrq_index: %d "%
                        (self.rrq_tbl_pos))
        if (GlobalOptions.dryrun): return

        #adminapi.ModifyQps(self.lif, [self])
        #self.modify_qp_oper = 0

    def PrepareAdminRequestSpec(self, req_spec):
        if not self.sq_in_nic:
            self.sq_tbl_pos = self.lif.GetRdmaTblPos(len(self.sq_slab.phy_address))
        else:
            self.sq_tbl_pos = self.lif.GetHbmTblPos(len(self.sq_slab.phy_address))
        if not self.rq_in_nic:
            self.rq_tbl_pos = self.lif.GetRdmaTblPos(len(self.rq_slab.phy_address))
        else:
            self.rq_tbl_pos = self.lif.GetHbmTblPos(len(self.rq_slab.phy_address))

        logger.info("QP: %s PD: %s Remote: %s HW_LIF: %d EP->Intf: %s SQ_TBL_POS: %d RQ_TBL_POS: %d" \
                        " RSQ_TBL_POS: %d RRQ_TBL_POS: %d" %\
                        (self.GID(), self.pd.GID(), self.remote, self.pd.ep.intf.lif.hw_lif_id,
                         self.pd.ep.intf.GID(), self.sq_tbl_pos, self.rq_tbl_pos,
                         self.rsq_tbl_pos, self.rrq_tbl_pos))

        if (GlobalOptions.dryrun): return

        # op = IONIC_V1_ADMIN_CREATE_QP
        req_spec.op = 2
        req_spec.type_state = self.svc
        req_spec.dbid_flags = self.lif.hw_lif_id
        req_spec.id_ver = self.id
        req_spec.pd_id = self.pd.id
        req_spec.access_perms_flags = self.flags
        req_spec.sq_cq_id = self.sq_cq.id
        req_spec.sq_depth_log2 = self.__get_log_size(self.num_sq_wqes)
        req_spec.sq_stride_log2 = self.__get_log_size(self.sqwqe_size)
        req_spec.sq_page_size_log2 = self.__get_log_size(self.hostmem_pg_size)
        req_spec.sq_tbl_index_xrcd_id = self.sq_tbl_pos
        pt_size = len(self.sq_slab.phy_address)
        req_spec.sq_map_count = 1 if self.sq_in_nic else pt_size
        if self.sq_in_nic:
            dma_addr = self.sq_tbl_pos * self.hostmem_pg_size
        else:
            dma_addr = self.sq_slab.GetDMATableSlab() if pt_size > 1 else self.sq_slab.phy_address[0]
        req_spec.sq_dma_addr = dma_addr
        req_spec.rq_cq_id = self.rq_cq.id
        req_spec.rq_depth_log2 = self.__get_log_size(self.num_rq_wqes)
        req_spec.rq_stride_log2 = self.__get_log_size(self.rqwqe_size)
        req_spec.rq_page_size_log2 = self.__get_log_size(self.hostmem_pg_size)
        req_spec.rq_tbl_index_srq_id = self.rq_tbl_pos
        pt_size = len(self.rq_slab.phy_address)
        req_spec.rq_map_count = 1 if self.rq_in_nic else pt_size
        if self.rq_in_nic:
            dma_addr = self.rq_tbl_pos * self.hostmem_pg_size
        else:
            dma_addr = self.rq_slab.GetDMATableSlab() if pt_size > 1 else self.rq_slab.phy_address[0]
        req_spec.rq_dma_addr = dma_addr

    def PrepareModAdminRequestSpec(self, req_spec):
        # op = IONIC_V1_ADMIN_MODIFY_QP
        req_spec.op = 9
        req_spec.type_state = self.qstate
        req_spec.dbid_flags = self.lif.hw_lif_id
        req_spec.id_ver = self.id
        req_spec.attr_mask = self.modify_qp_oper
        req_spec.dcqcn_profile = 1  # Attaching default dcqcn_profile.
        req_spec.access_flags = self.flags
        req_spec.pmtu = self.__get_log_size(self.pmtu)
        req_spec.rsq_depth = self.__get_log_size(self.num_rsq_wqes)
        req_spec.rsq_index = self.rsq_tbl_pos if self.rsq_tbl_pos > 0 else 0
        req_spec.rrq_depth = self.__get_log_size(self.num_rrq_wqes)
        req_spec.rrq_index = self.rrq_tbl_pos if self.rrq_tbl_pos > 0 else 0
        if self.svc == 0:
            req_spec.qkey_dest_qpn = self.dst_qp
        else:
            req_spec.qkey_dest_qpn = self.q_key
        if self.HdrTemplate is not None:
            req_spec.ah_id_len = (self.ah_handle | len(self.HdrTemplate) << 24)
            req_spec.dma_addr = self.hdr_slab.phy_address[0]

    def ProcessModAdminResponse(self, cqe):
        if (GlobalOptions.dryrun): return
        # Congestion management will be enabled by the tests that need it
        self.sq.qstate.Read()
        self.sq.qstate.data.congestion_mgmt_type = 0
        self.sq.qstate.WriteWithDelay()

        self.rq.qstate.Read()
        self.rq.qstate.data.congestion_mgmt_type = 0
        self.rq.qstate.WriteWithDelay()

        if not self.svc == 3 and not self.remote:
            # Configure Dcqcn CB
            self.ReadDcqcnCb()
            self.dcqcn_data.log_sq_size = self.__get_log_size(self.num_sq_wqes)
            self.dcqcn_data.rate_enforced = 4000
            self.dcqcn_data.token_bucket_size = 32768
            self.dcqcn_data.target_rate = 4000
            self.dcqcn_data.alpha_value = 65535
            self.dcqcn_data.byte_counter_thr = 4194304
            self.WriteDcqcnCb()

    def PrepareHALRequestSpec(self, req_spec):
        self.sq_tbl_pos = self.pd.ep.intf.lif.GetRdmaTblPos(len(self.sq_slab.phy_address))
        self.rq_tbl_pos = self.pd.ep.intf.lif.GetRdmaTblPos(len(self.rq_slab.phy_address))
        logger.info("QP: %s PD: %s Remote: %s HW_LIF: %d EP->Intf: %s SQ_TBL_POS: %d RQ_TBL_POS: %d" \
                        " RSQ_TBL_POS: %d RRQ_TBL_POS: %d" %\
                        (self.GID(), self.pd.GID(), self.remote, self.pd.ep.intf.lif.hw_lif_id,
                         self.pd.ep.intf.GID(), self.sq_tbl_pos, self.rq_tbl_pos,
                         self.rsq_tbl_pos, self.rrq_tbl_pos))

        if (GlobalOptions.dryrun): return

        if req_spec.__class__.__name__ == "RdmaQpSpec":  
    
            req_spec.qp_num = self.id
            req_spec.hw_lif_id = self.pd.ep.intf.lif.hw_lif_id
            req_spec.if_handle = self.pd.ep.intf.hal_handle
            req_spec.sq_wqe_size = self.sqwqe_size
            req_spec.rq_wqe_size = self.rqwqe_size
            req_spec.num_sq_wqes = self.num_sq_wqes
            req_spec.num_rq_wqes = self.num_rq_wqes
            req_spec.pd = self.pd.id
            req_spec.pmtu = self.pmtu
            req_spec.hostmem_pg_size = self.hostmem_pg_size
            req_spec.svc = self.svc
            req_spec.atomic_enabled = self.atomic_enabled

            #req_spec.sq_lkey = self.sq_mr.lkey
            #req_spec.rq_lkey = self.rq_mr.lkey
            
            req_spec.va_pages_phy_addr[:] = self.sq_slab.phy_address + self.rq_slab.phy_address
            req_spec.num_sq_pages = len(self.sq_slab.phy_address)

            req_spec.sq_cq_num = self.sq_cq.id
            req_spec.rq_cq_num = self.rq_cq.id
            req_spec.sq_in_nic_memory = self.sq_in_nic
            req_spec.rq_in_nic_memory = self.rq_in_nic
            req_spec.sq_table_index = self.sq_tbl_pos
            req_spec.rq_table_index = self.rq_tbl_pos
    
        elif req_spec.__class__.__name__ == "RdmaQpUpdateSpec":

            req_spec.qp_num = self.id
            req_spec.hw_lif_id = self.pd.ep.intf.lif.hw_lif_id
            req_spec.oper = self.modify_qp_oper
            if req_spec.oper == rdma_pb2.RDMA_UPDATE_QP_OPER_SET_DEST_QPN:
               logger.info(" RdmaQpUpdate Oper: SET_DEST_QPN dst_qp: %d " %
                             (self.dst_qp))
               req_spec.dst_qp_num = self.dst_qp
            elif req_spec.oper == rdma_pb2.RDMA_UPDATE_QP_OPER_SET_QKEY:
               logger.info(" RdmaQpUpdate Oper: SET_QKEY ")
               req_spec.q_key = self.q_key
            elif req_spec.oper == rdma_pb2.RDMA_UPDATE_QP_OPER_SET_AV:
               logger.info(" RdmaQpUpdate Oper: SET_AV")
               req_spec.header_template = bytes(self.HdrTemplate)
               req_spec.ahid = self.ah_handle
            elif req_spec.oper == rdma_pb2.RDMA_UPDATE_QP_OPER_SET_STATE:
               logger.info(" RdmaQpUpdate Oper: SET_STATE")
               req_spec.qstate = self.qstate
            elif req_spec.oper == rdma_pb2.RDMA_UPDATE_QP_OPER_SET_MAX_QP_RD_ATOMIC:
               logger.info(" RdmaQpUpdate Oper: SET_MAX_QP_RD_ATOMIC")
               req_spec.rrq_depth = self.num_rrq_wqes
               req_spec.rrq_index = self.rrq_tbl_pos
            elif req_spec.oper == rdma_pb2.RDMA_UPDATE_QP_OPER_SET_MAX_DEST_RD_ATOMIC:
               logger.info(" RdmaQpUpdate Oper: SET_MAX_DEST_RD_ATOMIC")
               req_spec.rsq_depth = self.num_rsq_wqes
               req_spec.rsq_index = self.rsq_tbl_pos

    def ProcessHALResponse(self, req_spec, resp_spec):
        if req_spec.__class__.__name__ == "RdmaQpSpec":  
            self.SetupRings(resp_spec)
    
        elif req_spec.__class__.__name__ == "RdmaQpUpdateSpec":

            logger.info("Resp for RdmaQpUpdateSpec, "
                           "QP: %s PD: %s oper: %d" %\
                            (self.GID(), self.pd.GID(), req_spec.oper))

    def SetupRings(self, resp_spec=None):
        # Dev Cmd
        if resp_spec:
            self.nic_sq_base_addr = resp_spec.nic_sq_base_addr
            self.nic_rq_base_addr = resp_spec.nic_rq_base_addr
            self.rdma_atomic_res_addr = resp_spec.rdma_atomic_res_addr
        else:
            self.nic_sq_base_addr = self.lif.hbm_barmap_base + (self.sq_tbl_pos * self.hostmem_pg_size)
            self.nic_rq_base_addr = self.lif.hbm_barmap_base + (self.rq_tbl_pos * self.hostmem_pg_size)
            self.rdma_atomic_res_addr = self.lif.rdma_atomic_res_addr

        logger.info("QP: %s PD: %s Remote: %s " %\
                    (self.GID(), self.pd.GID(), self.remote))
        if self.sq_in_nic:
            logger.info("sq_base_addr: 0x%x " % (self.nic_sq_base_addr))
            self.sq.SetRingParams('SQ', 0, True, True,
                              None,
                              self.nic_sq_base_addr,
                              self.num_sq_wqes,
                              self.sqwqe_size)
            self.sq.SetRingQpSpecEn('SQ', self.spec_en)
        else:
            logger.info("sq_base_addr: 0x%x " % (self.sq_slab.address))
            self.sq.SetRingParams('SQ', 0, True, False,
                              self.sq_slab.mem_handle,
                              self.sq_slab.address,
                              self.num_sq_wqes,
                              self.sqwqe_size)
            self.sq.SetRingQpSpecEn('SQ', self.spec_en)
        if self.rq_in_nic:
            logger.info("rq_base_addr: 0x%x " % (self.nic_rq_base_addr))
            self.rq.SetRingParams('RQ', 0, True, True,
                              None,
                              self.nic_rq_base_addr,
                              self.num_rq_wqes,
                              self.rqwqe_size)
            self.rq.SetRingQpSpecEn('RQ', self.spec_en)
        else:
            logger.info("rq_base_addr: 0x%x " % (self.rq_slab.address))
            self.rq.SetRingParams('RQ', 0, True, False,
                              self.rq_slab.mem_handle,
                              self.rq_slab.address,
                              self.num_rq_wqes,
                              self.rqwqe_size)
            self.rq.SetRingQpSpecEn('RQ', self.spec_en)
        logger.info("rsq_tbl_pos: %d rrq_tbl_pos: %d " %\
                         (self.rsq_tbl_pos, self.rrq_tbl_pos))

    def IsFilterMatch(self, spec):
        logger.debug("Matching QID %d svc %d" %(self.id, self.svc))
        match = super().IsFilterMatch(spec.filters)

        return match

    def ShowTestcaseConfig(self, obj):
        logger.info("Config Objects for %s" % (self.GID()))
        return

    def ConfigureHeaderTemplate(self, rdma_session, initiator, responder, flow, isipv6, forward, ah_handle):
        logger.info("rdma_session: %s" % rdma_session.GID())
        logger.info("session: %s" % rdma_session.session.GID())
        logger.info("flow_ep1: %s ep1: %s" \
            %(initiator.GID(), initiator.ep.GID()))
        logger.info("flow_ep2: %s ep2: %s" \
             %(responder.GID(), responder.ep.GID()))
        logger.info("src_ip: %s" % initiator.addr.get())
        logger.info("dst_ip: %s" % responder.addr.get())
        logger.info("src_mac: %s" % initiator.ep.macaddr.get())
        logger.info("dst_mac: %s" % responder.ep.macaddr.get())
        logger.info("proto: %s" % flow.proto)
        logger.info("sport: %s" % flow.sport)
        logger.info("dport: %s" % flow.dport)
        logger.info("ah_handle: %d" % ah_handle)
        if forward:
            logger.info("src_qp: %d pd: %s" % (rdma_session.lqp.id, rdma_session.lqp.pd.GID()))
            logger.info("dst_qp: %d pd: %s" % (rdma_session.rqp.id, rdma_session.rqp.pd.GID()))
        else:
            logger.info("src_qp: %d pd: %s" % (rdma_session.rqp.id, rdma_session.rqp.pd.GID()))
            logger.info("dst_qp: %d pd: %s" % (rdma_session.lqp.id, rdma_session.lqp.pd.GID()))
        logger.info("isipv6: %d" % (isipv6))
        logger.info("isVXLAN: %d" % (rdma_session.IsVXLAN))

        EthHdr = scapy.Ether(src=initiator.ep.macaddr.get(),
                             dst=responder.ep.macaddr.get())
        Dot1qHdr = scapy.Dot1Q(vlan=initiator.ep.intf.encap_vlan_id,
                               prio=flow.txqos.cos)
        if isipv6:
            IpHdr = scapy.IPv6(src=initiator.addr.get(),
                             dst=responder.addr.get(),
                             tc=flow.txqos.dscp,
                             plen = 0)
        else:
            IpHdr = scapy.IP(src=initiator.addr.get(),
                             dst=responder.addr.get(),
                             tos=flow.txqos.dscp,
                             len = 0, chksum = 0)
        if forward:
            UdpHdr = scapy.UDP(sport=flow.sport,
                               dport=flow.dport,
                               len = 0, chksum = 0)
        else:
            UdpHdr = scapy.UDP(sport=flow.dport,
                               dport=flow.sport,
                               len = 0, chksum = 0)
        self.HdrTemplate = EthHdr/Dot1qHdr/IpHdr/UdpHdr
        self.ah_handle = ah_handle

        # header_template size is 66. The address is maintained only to calculate
        # dcqcn_cb address.
        # 200 = [ 66 (header_template_t) + 1 (ah_size) + 64 (dcqcncb_t) + 64 (rome_receiver_cb_t)] 8 byte aligned
        self.header_temp_addr = self.lif.rdma_at_base_addr + (ah_handle * 200)

        self.modify_qp_oper |= 1 << rdma_pb2.RDMA_UPDATE_QP_OPER_SET_AV
        logger.info(" RdmaQpUpdate Oper: SET_AV ah_handle: %d "%
                        (self.ah_handle))

        if (GlobalOptions.dryrun): return

        resmgr.HostMemoryAllocator.write(self.hdr_slab.mem_handle, bytes(self.HdrTemplate))
        #adminapi.ModifyQps(self.lif, [self])
        #self.modify_qp_oper = 0
        return

    # Routines to read and write to dcqcn_cb    
    def WriteDcqcnCb(self):
        if (GlobalOptions.dryrun): return
        # dcqcn_cb is located after header_template. header_template is 66 bytes len and 1 byte ah_size.
        logger.info("Writing DCQCN Qstate @0x%x  size: %d" % (self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, DCQCN_CB_T_SIZE_BYTES))
        model_wrap.write_mem_pcie(self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, bytes(self.dcqcn_data), DCQCN_CB_T_SIZE_BYTES)
        self.ReadDcqcnCb()
        return

    def ReadDcqcnCb(self):
        if (GlobalOptions.dryrun):
            return
        self.dcqcn_data = RdmaDCQCNstate(model_wrap.read_mem(self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, DCQCN_CB_T_SIZE_BYTES))
        logger.ShowScapyObject(self.dcqcn_data)
        logger.info("Read DCQCN Qstate @0x%x size: %d" % (self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, DCQCN_CB_T_SIZE_BYTES))
        return

    def WriteRomeSenderCb(self):
        if (GlobalOptions.dryrun): return
        # rome_sender_cb is located after header_template. header_template is 66 bytes len and 1 byte ah_size.
        logger.info("Writing ROME_SENDER Qstate @0x%x  size: %d" % (self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, ROME_CB_T_SIZE_BYTES))
        model_wrap.write_mem_pcie(self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, bytes(self.rome_sender_data), ROME_CB_T_SIZE_BYTES)
        self.ReadRomeSenderCb()
        return

    def ReadRomeSenderCb(self):
        if (GlobalOptions.dryrun):
            return
        self.rome_sender_data = RdmaROME_SENDERstate(model_wrap.read_mem(self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, ROME_CB_T_SIZE_BYTES))
        logger.ShowScapyObject(self.rome_sender_data)
        logger.info("Read ROME_SENDER Qstate @0x%x size: %d" % (self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES, ROME_CB_T_SIZE_BYTES))
        return

    # Routines to read and write to rome_cb
    def WriteRomeReceiverCb(self):
        if (GlobalOptions.dryrun): return
        # rome_receiver_cb is located after ah table and dcqcn table. header_template is 66 bytes len and 1 byte ah_size.
        logger.info("Writing ROME_RECEIVER Qstate @0x%x  size: %d" % (self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES, ROME_CB_T_SIZE_BYTES))
        model_wrap.write_mem_pcie(self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES, bytes(self.rome_receiver_data), ROME_CB_T_SIZE_BYTES)
        self.ReadRomeReceiverCb()
        return

    def ReadRomeReceiverCb(self):
        if (GlobalOptions.dryrun):
            rome_receiver_data = bytes(ROME_CB_T_SIZE_BYTES)
            self.rome_receiver_data = RdmaROME_RECEIVERstate(rome_receiver_data)
            return
        self.rome_receiver_data = RdmaROME_RECEIVERstate(model_wrap.read_mem(self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES, ROME_CB_T_SIZE_BYTES))
        logger.ShowScapyObject(self.rome_receiver_data)
        logger.info("Read ROME_RECEIVER Qstate @0x%x size: %d" % (self.header_temp_addr + AH_ENTRY_T_SIZE_BYTES + DCQCN_CB_T_SIZE_BYTES, ROME_CB_T_SIZE_BYTES))
        return

    # Routines to read and write to atomic_res_addr
    def WriteAtomicResData(self):
        if (GlobalOptions.dryrun): return
        logger.info("Writing Atomic resource address @0x%x  size: %d" % (self.rdma_atomic_res_addr, 64))
        model_wrap.write_mem_pcie(self.rdma_atomic_res_addr, bytes(self.atomic_res_data), 64)
        self.ReadAtomicResData()
        return

    def ReadAtomicResData(self):
        if (GlobalOptions.dryrun):
            atomic_res_data = bytes(64)
            self.atomic_res_data = RdmaAtomicResState(atomic_res_data)
            return
        self.atomic_res_data = RdmaAtomicResState(model_wrap.read_mem(self.rdma_atomic_res_addr, 64))
        logger.ShowScapyObject(self.atomic_res_data)
        return

    def __get_log_size(self, x):
        power = 1
        log = 0
        while power < x:
            power *= 2
            log += 1
        return log

class RdmaDCQCNstate(scapy.Packet):
    name = "RdmaDCQCNstate"
    fields_desc = [
        scapy.BitField("last_cnp_timestamp", 0, 48),

        scapy.IntField("byte_counter_thr",0),
        scapy.ByteField("rsvd1",0),
        scapy.BitField("sq_msg_psn", 0, 24),

        scapy.IntField("rate_enforced", 0),
        scapy.IntField("target_rate",0),
        scapy.BitField("alpha_value", 0, 16),

        scapy.IntField("cur_byte_counter",0),
        scapy.BitField("byte_counter_exp_cnt",0, 16),
        scapy.BitField("timer_exp_cnt", 0, 16),
        scapy.BitField("num_alpha_exp_cnt", 0, 16), 
        scapy.ByteField("num_cnp_rcvd", 0),
        scapy.ByteField("num_cnp_processed", 0),
        scapy.BitField("max_rate_reached", 0, 1),
        scapy.BitField("log_sq_size", 0, 5),
        scapy.BitField("resp_rl_failure", 0, 1),
        scapy.BitField("rsvd0", 0, 1),
        
        scapy.BitField("last_sched_timestamp", 0, 48),
        scapy.BitField("delta_tokens_last_sched", 0, 16),
        scapy.BitField("cur_avail_tokens", 0, 48),
        scapy.BitField("token_bucket_size", 0, 48),
        scapy.BitField("sq_cindex", 0, 16),

        scapy.ByteField("num_sched_drop", 0),
        scapy.BitField("cur_timestamp", 0, 32),
    ]

class RdmaROME_SENDERstate(scapy.Packet):
    name = "RdmaROME_SENDERstate"
    fields_desc = [
        scapy.BitField("localClkResolution", 0, 8),
        scapy.BitField("remoteClkRightShift", 0, 8),
        scapy.BitField("clkScaleFactor", 0, 31),
        scapy.BitField("txMinRate", 0, 17),

        scapy.BitField("remoteClkStartEpoch", 0, 32),
        scapy.BitField("stalocalClkStartEpochte", 0, 48),
        scapy.BitField("totalBytesSent", 0, 32),
        scapy.BitField("totalBytesAcked", 0, 32),
        scapy.BitField("window", 0, 32),
        scapy.BitField("currentRate", 0, 27),
        scapy.BitField("log_sq_size", 0, 5),
        scapy.BitField("numCnpPktsRx", 0, 32),

        scapy.BitField("last_sched_timestamp", 0, 48),
        scapy.BitField("delta_ticks_last_sched", 0, 16),
        scapy.BitField("cur_avail_tokens", 0, 48),
        scapy.BitField("token_bucket_size", 0, 48),
        scapy.BitField("sq_cindex", 0, 16),
        scapy.BitField("cur_timestamp", 0, 32),
    ]

class RdmaROME_RECEIVERstate(scapy.Packet):
    name = "RdmaROME_RECEIVERstate"
    fields_desc = [
        scapy.BitField("state", 0, 3),
        scapy.BitField("rsvd0", 0, 5),

        scapy.BitField("minTimestampDiff", 0, 32),

        scapy.BitField("linkDataRate", 0, 2),
        scapy.BitField("recoverRate", 0, 27),
        scapy.BitField("minRate", 0, 27),

        scapy.BitField("weight", 0, 4),
        scapy.BitField("rxDMA_tick", 0, 3),
        scapy.BitField("wait", 0, 1),

        scapy.BitField("avgDelay", 0, 20),
        scapy.BitField("preAvgDelay", 0, 20),
        scapy.BitField("cycleMinDelay", 0, 20),
        scapy.BitField("cycleMaxDelay", 0, 20),

        scapy.BitField("totalBytesRx", 0, 32),
        scapy.BitField("rxBurstBytes", 0, 16),        
        scapy.BitField("byte_update", 0, 16),
        scapy.BitField("th_byte", 0, 32),

        scapy.BitField("cur_timestamp", 0, 10),
        scapy.BitField("thput", 0, 27),
        scapy.BitField("MD_amount", 0, 27),
        scapy.BitField("last_cycle", 0, 32),
        scapy.BitField("last_thput", 0, 32),
        scapy.BitField("last_epoch", 0, 32),
        scapy.BitField("last_update", 0, 32),

        scapy.BitField("txDMA_tick", 0, 3),
        scapy.BitField("fspeed_cnt", 0, 10),
        scapy.BitField("currentRate", 0, 27),
    ]

class RdmaAtomicResState(scapy.Packet):
    name = "RdmaAtomicResState"
    fields_desc = [
        scapy.LongField("data0", 0),
        scapy.LongField("data1", 0),
        scapy.LongField("data2", 0),
        scapy.LongField("data3", 0),

        scapy.LongField("pad0", 0),
        scapy.LongField("pad1", 0),
        scapy.LongField("pad2", 0),
        scapy.LongField("pad3", 0),
    ]

class QpObjectHelper:
    def __init__(self):
        self.qps = []
        self.perf_qps = []
        self.udqps = []
        self.useAdmin = False

    def Generate(self, pd, spec):
        self.pd = pd
        j = 0

        #RC QPs
        rc_spec = spec.rc
        count = rc_spec.count
        self.useAdmin = spec.useAdmin
        logger.info("Creating %d %s Qps. for PD:%s" %\
                       (count, rc_spec.svc_name, pd.GID()))
        for i in range(count):
            qp_id = j if pd.remote else pd.ep.intf.lif.GetQpid()
            sges = 2 << i
            logger.info("RC: qp_id: %d sges: %d" %(qp_id, sges))
            qp = QpObject(pd, qp_id, rc_spec, sges)
            self.qps.append(qp)
            j += 1

        #PERF RC QPs
        rc_spec = spec.perf_rc
        count = rc_spec.count
        logger.info("Creating %d %s Perf Qps (SQ/RQ in NIC). for PD:%s" %\
                       (count, rc_spec.svc_name, pd.GID()))
        for i in range(count):
            qp_id = j if pd.remote else pd.ep.intf.lif.GetQpid()
            sges = 2 << i
            logger.info("PerfRC: qp_id: %d sges: %d" %(qp_id, sges))
            qp = QpObject(pd, qp_id, rc_spec, sges)
            self.perf_qps.append(qp)
            j += 1

        #PERF RC (Only SQ in NIC) QPs
        rc_spec = spec.perf_rc_sqnic
        count = rc_spec.count
        logger.info("Creating %d %s Perf Qps (Only SQ in NIC). for PD:%s" %\
                       (count, rc_spec.svc_name, pd.GID()))
        for i in range(count):
            qp_id = j if pd.remote else pd.ep.intf.lif.GetQpid()
            sges = 2 << i
            logger.info("PerfRC (SQ in NIC) : qp_id: %d sges: %d" %(qp_id, sges))
            qp = QpObject(pd, qp_id, rc_spec, sges)
            self.perf_qps.append(qp)
            j += 1

        #PERF RC (Only RQ in NIC) QPs
        rc_spec = spec.perf_rc_rqnic
        count = rc_spec.count
        logger.info("Creating %d %s Perf Qps (Only RQ in NIC). for PD:%s" %\
                       (count, rc_spec.svc_name, pd.GID()))
        for i in range(count):
            qp_id = j if pd.remote else pd.ep.intf.lif.GetQpid()
            sges = 2 << i
            logger.info("PerfRC (RQ in NIC): qp_id: %d sges: %d" %(qp_id, sges))
            qp = QpObject(pd, qp_id, rc_spec, sges)
            self.perf_qps.append(qp)
            j += 1

        #UD QPs
        ud_spec = spec.ud
        count = ud_spec.count
        logger.info("Creating %d %s Qps. for PD:%s" %\
                       (count, ud_spec.svc_name, pd.GID()))
        for i in range(count):
            qp_id = j if pd.remote else pd.ep.intf.lif.GetQpid()
            sges = 2 << i
            logger.info("UD: qp_id: %d sges: %d" %(qp_id, sges))
            qp = QpObject(pd, qp_id, ud_spec, sges)
            self.qps.append(qp)
            self.udqps.append(qp)
            j += 1


    def Configure(self):
        if self.pd.remote:
            logger.info("skipping QP configuration for remote PD: %s" %(self.pd.GID()))
            return
        logger.info("Configuring %d QPs %d Perf QPs." % (len(self.qps), len(self.perf_qps)))

        if (GlobalOptions.dryrun): return

        if self.useAdmin is True:
            adminapi.ConfigureQps(self.qps[0].lif, self.qps)
            adminapi.ConfigureQps(self.perf_qps[0].lif, self.perf_qps)
        else:
            halapi.ConfigureQps(self.qps)
            halapi.ConfigureQps(self.perf_qps)
