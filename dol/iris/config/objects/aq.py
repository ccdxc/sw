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

from factory.objects.rdma.descriptor import RdmaAqDescriptorBase
from factory.objects.rdma.descriptor import RdmaAqDescriptorNOP

from infra.common.glopts import GlobalOptions

class AqObject(base.ConfigObjectBase):
    def __init__(self, lif, aq_id, num_aqe, hostmem_pg_size):
        super().__init__()
        self.Clone(Store.templates.Get('ADMIN_RDMA'))
        self.lif = lif
        self.id = aq_id
        self.GID("AQ%04d" % self.id)

        self.hostmem_pg_size = hostmem_pg_size
        self.num_aq_wqes = self.__roundup_to_pow_2(num_aqe)
        self.aqwqe_size = self.__get_aqwqe_size()
        self.aq_size = self.num_aq_wqes * self.aqwqe_size

        self.aq_base_addr = None
        self.aq_base_addr_phy = None

        # AdminQ IDs start from 1 for RDMA
        self.aq = lif.GetQ('RDMA_AQ', self.id)

        if (self.aq is None):
            assert(0)
        
        self.cq = lif.GetQ('RDMA_CQ', lif.rdma_admin_cq_id)
        if (self.cq is None):
                assert(0)
    
        self.eq = lif.GetQ('RDMA_EQ', lif.rdma_admin_eq_id)  # Sharing Async EQ with AQ
        if (self.eq is None):
            assert(0)

        self.aq_slab = slab.SlabObject(self.lif, self.aq_size, True)
        self.lif.AddSlab(self.aq_slab)

        self.Show()

        return

    def __roundup_to_pow_2(self, x):
        power = 1
        while power < x : 
            power *= 2
        return power
 
    def __get_aqwqe_size(self):
        return  self.__roundup_to_pow_2(
                len(RdmaAqDescriptorBase()) + 
                len(RdmaAqDescriptorNOP()))

    def __get_log_size(self, x):
        power = 1
        log = 0
        while power < x:
            power *= 2
            log += 1
        return log

    def Show(self):
        logger.info('AQ: %s LIF: %s' %(self.GID(), self.lif.GID()))
        logger.info('AQ num_wqes: %d wqe_size: %d' %(self.num_aq_wqes, self.aqwqe_size))
        logger.info('CQ: %s EQ: %s' %(self.cq.GID(), self.eq.GID()))
        if not self.aq_base_addr is None:
            logger.info('AQBase Addr- VA: 0x%x PHY: 0x%x' %(self.aq_base_addr, self.aq_base_addr_phy))

    def ShowTestcaseConfig(self, obj):
        logger.info("Config Objects for %s" % (self.GID()))
        return

    def PrepareHALRequestSpec(self, req_spec):
        self.tbl_pos = self.lif.GetRdmaTblPos(len(self.aq_slab.phy_address))
        logger.info("AQ: %s LIF: %s HW_LIF: %d TBL_POS: %d" %\
                        (self.GID(), self.lif.GID(), self.lif.hw_lif_id, self.tbl_pos))

        if (GlobalOptions.dryrun): return

        req_spec.aq_num = self.id
        req_spec.hw_lif_id = self.lif.hw_lif_id
        req_spec.log_wqe_size = self.__get_log_size(self.aqwqe_size)
        req_spec.log_num_wqes = self.__get_log_size(self.num_aq_wqes)
        req_spec.cq_num = self.cq.id
        req_spec.phy_base_addr = self.aq_base_addr = self.aq_slab.phy_address[0]

    def ProcessHALResponse(self, req_spec, resp_spec):

        self.lif.rdma_atomic_res_addr = resp_spec.rdma_atomic_res_addr
        self.aq.SetRingParams('AQ', 0, True, False,
                              self.aq_slab.mem_handle,
                              self.aq_slab.address,
                              self.num_aq_wqes, 
                              self.aqwqe_size)
        logger.info("AQ: %s LIF: %s "
                       "aqcb_addr: 0x%x aq_base_addr: 0x%x "
                       "rdma_atomic_res_addr: 0x%x " %\
                        (self.GID(), self.lif.GID(),
                         self.aq.GetQstateAddr(), self.aq_slab.address,
                         self.lif.rdma_atomic_res_addr))
    
        #logger.ShowScapyObject(self.aq.qstate.data)
