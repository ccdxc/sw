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
    def __init__(self, pd, aq_id, spec):
        super().__init__()
        self.Clone(Store.templates.Get('ADMIN_RDMA'))
        self.pd = pd
        self.remote = pd.remote
        self.id = aq_id
        self.GID("AQ%04d" % self.id)
        self.spec = spec

        self.hostmem_pg_size = spec.hostmem_pg_size
        self.num_aq_wqes = self.__roundup_to_pow_2(spec.num_wqes)
        self.aqwqe_size = self.__get_aqwqe_size()
        self.aq_size = self.num_aq_wqes * self.aqwqe_size

        self.aq_base_addr = None
        self.aq_base_addr_phy = None

        if not self.remote:
            # AdminQ IDs start from 1 for RDMA
            self.aq = pd.ep.intf.lif.GetQ('RDMA_AQ', self.id)

            if (self.aq is None):
                assert(0)
        
            self.cq = pd.ep.intf.lif.GetQ('RDMA_CQ', 0)
            if (self.cq is None):
                assert(0)
    
            # allocating one EQ for one PD
            self.eq = pd.ep.intf.lif.GetQ('RDMA_EQ', pd.id)  # PD id is the EQ number
            if (self.eq is None):
                assert(0)
    
            # create aq slab
            self.aq_slab = slab.SlabObject(self.pd.ep, self.aq_size)
            self.pd.ep.AddSlab(self.aq_slab)

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
        logger.info('AQ: %s PD: %s Remote: %s' %(self.GID(), self.pd.GID(), self.remote))
        logger.info('AQ num_wqes: %d wqe_size: %d' %(self.num_aq_wqes, self.aqwqe_size))
        if not self.remote:
            logger.info('CQ: %s EQ: %s' %(self.cq.GID(), self.eq.GID()))
        if not self.aq_base_addr is None:
            logger.info('AQBase Addr- VA: 0x%x PHY: 0x%x' %(self.aq_base_addr, self.aq_base_addr_phy))

    def PrepareHALRequestSpec(self, req_spec):
        logger.info("AQ: %s PD: %s Remote: %s HW_LIF: %d EP->Intf: %s" %\
                        (self.GID(), self.pd.GID(), self.remote, self.pd.ep.intf.lif.hw_lif_id,
                         self.pd.ep.intf.GID()))

        if (GlobalOptions.dryrun): return

        req_spec.aq_num = self.id
        req_spec.hw_lif_id = self.pd.ep.intf.lif.hw_lif_id
        req_spec.log_wqe_size = self.__get_log_size(self.aqwqe_size)
        req_spec.log_num_wqes = self.__get_log_size(self.num_aq_wqes)
        req_spec.cq_num = self.cq.id
        self.aq_base_addr = self.aq_slab.address
        req_spec.phy_base_addr = resmgr.HostMemoryAllocator.v2p(self.aq_base_addr)

    def ProcessHALResponse(self, req_spec, resp_spec):

        self.aq.SetRingParams('AQ', 0, True, False,
                              self.aq_slab.mem_handle,
                              self.aq_slab.address,
                              self.num_aq_wqes, 
                              self.aqwqe_size)
        logger.info("AQ: %s PD: %s Remote: %s"
                       "aqcb_addr: 0x%x aq_base_addr: 0x%x " %\
                        (self.GID(), self.pd.GID(), self.remote,
                         self.aq.GetQstateAddr(), self.aq_slab.address))
    
        #logger.ShowScapyObject(self.aq.qstate.data)

class AqObjectHelper:
    def __init__(self):
        self.aqs = []

    # Only called for PD 1
    def Generate(self, pd, spec):
        self.pd = pd
        if self.pd.remote:
            logger.info("skipping AQ generation for remote PD: %s" %(pd.GID()))
            return

        assert(pd.id == 1)
        count = spec.count
        logger.info("Creating %d Aqs. for PD:%s" %\
                       (count, pd.GID()))
        for i in range(count):
            # Aqid 0 is owned by ETH
            aq_id = pd.ep.intf.lif.GetAqid() + 1
            aq = AqObject(pd, aq_id, spec)
            self.aqs.append(aq)

    def Configure(self):
        if self.pd.remote:
            logger.info("skipping AQ configuration for remote PD: %s" %(self.pd.GID()))
            return
        logger.info("Configuring %d AQs" % len(self.aqs))
        halapi.ConfigureAqs(self.aqs)
