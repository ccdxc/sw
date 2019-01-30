#! /usr/bin/python3
import pdb

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
from factory.objects.rdma.descriptor import RdmaCqDescriptorRecv
from factory.objects.rdma.descriptor import RdmaCqDescriptorSend
from factory.objects.rdma.descriptor import RdmaCqDescriptorAdmin
from factory.objects.rdma.descriptor import RdmaSge
from infra.common.glopts import GlobalOptions

class CqObject(base.ConfigObjectBase):
    def __init__(self, ep, cq_id, num_wqes, pg_sz, privileged = False, lif = None):
        super().__init__()
        self.Clone(Store.templates.Get('QSTATE_RDMA_CQ'))
        self.privileged = privileged

        self.num_cq_wqes = self.__roundup_to_pow_2(num_wqes)
        self.cqwqe_size = self.__get_cqwqe_size()
        self.cq_size = self.num_cq_wqes * self.cqwqe_size

        if privileged is True:
            self.lif = lif
            self.ep = None
            self.remote = False
            self.hostmem_pg_size = self.cq_size
        else:
            self.lif = ep.intf.lif
            self.ep = ep
            self.remote = ep.remote
            self.hostmem_pg_size = pg_sz
        self.id = cq_id
        self.GID("CQ%04d" % self.id)
        # Every 8 CQs share a EQ
        self.eq_id = int((self.id + 7) / 8) + 1

        if not self.remote:
            self.cq = self.lif.GetQ('RDMA_CQ', self.id)
            self.eq = self.lif.GetQ('RDMA_EQ', self.eq_id)
    
            if (self.cq is None or self.eq is None):
                assert(0)

            # create cq slab
            if self.privileged is True:
                self.cq_slab = slab.SlabObject(self.lif, self.cq_size, True)
                self.lif.AddSlab(self.cq_slab)
            else:
                self.cq_slab = slab.SlabObject(self.ep.intf.lif, self.cq_size)
                self.ep.AddSlab(self.cq_slab)

        self.Show()

        return

    def __roundup_to_pow_2(self, x):
        power = 1
        while power < x : 
            power *= 2
        return power
 
    def __get_cqwqe_size(self):
        #all CQ WQEs (Send, Recv, Admin) are of same size
        return  self.__roundup_to_pow_2(len(RdmaCqDescriptorRecv()))

    def Show(self):
        if self.privileged is True:
            logger.info('CQ: %s LIF: %s Remote: %s' %(self.GID(), self.lif.GID(), self.remote))
        else:
            logger.info('CQ: %s EP: %s Remote: %s' %(self.GID(), self.ep.GID(), self.remote))
        logger.info('CQ num_wqes: %d wqe_size: %d, EQ-ID: %d' 
                       %(self.num_cq_wqes, self.cqwqe_size, self.eq_id)) 

    def PrepareAdminRequestSpec(self, req_spec):
        self.tbl_pos = self.lif.GetRdmaTblPos(len(self.cq_slab.phy_address))
        if self.privileged is True:
            logger.info("CQ: %s LIF: %s TBL_POS: %d" %\
                            (self.GID(), self.lif.GID(), self.tbl_pos))
        else:
            logger.info("CQ: %s EP: %s Remote: %s LIF: %d TBL_POS: %d" %\
                            (self.GID(), self.ep.GID(), self.remote,
                            self.lif.hw_lif_id, self.tbl_pos))
        if (GlobalOptions.dryrun): return

        # op = IONIC_V1_ADMIN_CREATE_CQ
        req_spec.op = 1
        req_spec.dbid_flags = self.lif.hw_lif_id
        req_spec.id_ver = self.id
        req_spec.eq_id = self.eq_id
        req_spec.depth_log2 = self.__get_log_size(self.num_cq_wqes)
        req_spec.stride_log2 = self.__get_log_size(self.cqwqe_size)
        req_spec.page_size_log2 = self.__get_log_size(self.hostmem_pg_size)
        req_spec.tbl_index = self.tbl_pos
        pt_size = len(self.cq_slab.phy_address)
        req_spec.map_count = pt_size
        dma_addr = self.cq_slab.GetDMATableSlab() if pt_size > 1 else self.cq_slab.phy_address[0]
        req_spec.dma_addr = dma_addr

    def PrepareHALRequestSpec(self, req_spec):
        self.tbl_pos = self.lif.GetRdmaTblPos(len(self.cq_slab.phy_address))
        if self.privileged is True:
            logger.info("CQ: %s LIF: %s TBL_POS: %d" %\
                            (self.GID(), self.lif.GID(), self.tbl_pos))
        else:
            logger.info("CQ: %s EP: %s Remote: %s LIF: %d TBL_POS: %d" %\
                            (self.GID(), self.ep.GID(), self.remote,
                            self.lif.hw_lif_id, self.tbl_pos))
        if (GlobalOptions.dryrun): return

        req_spec.cq_num = self.id
        req_spec.eq_id = self.eq_id
        req_spec.hw_lif_id = self.lif.hw_lif_id
        req_spec.cq_wqe_size = self.cqwqe_size
        req_spec.num_cq_wqes = self.num_cq_wqes
        req_spec.host_addr = 1
        req_spec.hostmem_pg_size = self.hostmem_pg_size
        req_spec.cq_va_pages_phy_addr[:] = self.cq_slab.phy_address
        req_spec.table_index = self.tbl_pos

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.SetupRings()

    def SetupRings(self):
        self.cq.SetRingParams('CQ', 0, True, False,
                              self.cq_slab.mem_handle,
                              self.cq_slab.address, 
                              self.num_cq_wqes, 
                              self.cqwqe_size)
        #repeat same mem_handle, and other params for these rings,
        #but they are never used.
        #TODO: find a way to create LiteRings, where there is no
        #associated FIFO, but just CI/PI
        self.cq.SetRingParams('CQ_ARM', 1, True, False,
                              self.cq_slab.mem_handle,
                              self.cq_slab.address, 
                              0,
                              0)
        self.cq.SetRingParams('CQ_SARM', 2, True, False,
                              self.cq_slab.mem_handle,
                              self.cq_slab.address, 
                              0,
                              0)
        if self.privileged is True:
            logger.info("CQ: %s LIF: %s Remote: %s cqcb_addr: 0x%x cq_base_addr: 0x%x" %\
                            (self.GID(), self.lif.hw_lif_id, self.remote,
                            self.cq.GetQstateAddr(), self.cq_slab.address))
        else:
            logger.info("CQ: %s EP: %s Remote: %s LIF: %d cqcb_addr: 0x%x cq_base_addr: 0x%x" %\
                            (self.GID(), self.ep.GID(), self.remote,
                            self.lif.hw_lif_id,
                            self.cq.GetQstateAddr(), self.cq_slab.address))
        #logger.ShowScapyObject(self.cq.qstate.data)
         
    def __get_log_size(self, x):
        power = 1
        log = 0
        while power < x:
            power *= 2
            log += 1
        return log

class CqObjectHelper:
    def __init__(self):
        self.cqs = []
        self.useAdmin = False

    def Generate(self, ep, spec):
        self.ep = ep
        if self.ep.remote:
            logger.info("skipping CQ generation for remote EP: %s" %(ep.GID()))
            return

        count = spec.count
        self.useAdmin = spec.useAdmin
        logger.info("Creating %d Cqs. for EP:%s useAdmin: %d" %\
                       (count, ep.GID(), self.useAdmin))
        for i in range(count):
            cq_id = i if ep.remote else ep.intf.lif.GetCqid()
            cq = CqObject(ep, cq_id, spec.num_wqes, spec.hostmem_pg_size)
            self.cqs.append(cq)

    def Configure(self):
        if self.ep.remote:
            logger.info("skipping CQ configuration for remote EP: %s" %(self.ep.GID()))
            return
        logger.info("Configuring %d CQs." % len(self.cqs))

        if (GlobalOptions.dryrun): return

        if self.useAdmin is True:
            adminapi.ConfigureCqs(self.cqs[0].lif, self.cqs)
        else:
            halapi.ConfigureCqs(self.cqs)
