#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
from iris.config.store               import Store
from infra.common.logging       import logger

import iris.config.objects.rdma.adminapi    as adminapi

import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2
from infra.common.glopts import GlobalOptions

class MrObject(base.ConfigObjectBase):
    def __init__(self, pd, slab, id_substr="",
                 local_wr=False, remote_wr=False, 
                 remote_rd=False, remote_atomic=False):
        super().__init__()
        self.Clone(Store.templates.Get('RDMA_MR'))
        self.pd = pd
        self.lif = pd.ep.intf.lif
        self.slab = slab
        self.lkey = pd.ep.intf.lif.GetMrKey() << 8
        #self.rkey = self.lkey + 4096
        self.rkey = self.lkey
        self.id = slab.id
        if id_substr:
            mr_id = ("MR-%s-%s" % (id_substr, self.slab.GID()))
        else:
            mr_id = ("MR-%s" % (self.slab.GID()))
        self.GID(mr_id)
        self.local_wr = local_wr
        self.remote_wr = remote_wr
        self.remote_rd = remote_rd
        self.remote_atomic = remote_atomic
        self.flags = self.GetAccessFlags()
        return

    def PrepareAdminRequestSpec(self, req_spec):
        if (GlobalOptions.dryrun): return

        self.tbl_pos = self.pd.ep.intf.lif.GetRdmaTblPos(len(self.slab.phy_address))
        logger.info("MR: %s PD: %s SLAB: %s LIF: %s lkey: %s local_wr %s TBL_POS: %d" %\
                    (self.GID(), self.pd.GID(), self.slab.GID(),
                     self.pd.ep.intf.lif.hw_lif_id, self.lkey, self.local_wr, self.tbl_pos))
        # op = IONIC_V1_ADMIN_REG_MR
        req_spec.op = 3
        req_spec.dbid_flags = self.flags
        req_spec.id_ver = self.lkey
        req_spec.va = self.slab.address
        req_spec.length = self.slab.size
        req_spec.pd_id = self.pd.id
        req_spec.page_size_log2 = self.__get_log_size(self.slab.page_size)
        req_spec.tbl_index = self.tbl_pos
        pt_size = len(self.slab.phy_address)
        req_spec.map_count = pt_size
        dma_addr = self.slab.GetDMATableSlab() if pt_size > 1 else self.slab.phy_address[0]
        req_spec.dma_addr = self.slab.GetDMATableSlab()

    def PrepareHALRequestSpec(self, req_spec):
        if (GlobalOptions.dryrun): return

        self.tbl_pos = self.pd.ep.intf.lif.GetRdmaTblPos(len(self.slab.phy_address))
        logger.info("MR: %s PD: %s SLAB: %s LIF: %s lkey: %s local_wr %s TBL_POS: %d" %\
                    (self.GID(), self.pd.GID(), self.slab.GID(),
                     self.pd.ep.intf.lif.hw_lif_id, self.lkey, self.local_wr, self.tbl_pos))

        req_spec.hw_lif_id = self.pd.ep.intf.lif.hw_lif_id
        req_spec.pd = self.pd.id
        req_spec.va = self.slab.address
        req_spec.len = self.slab.size
        req_spec.ac_local_wr = self.local_wr
        req_spec.ac_remote_wr = self.remote_wr
        req_spec.ac_remote_rd = self.remote_rd
        req_spec.ac_remote_atomic = self.remote_atomic
        req_spec.lkey = self.lkey
        req_spec.rkey = self.rkey
        req_spec.hostmem_pg_size = self.slab.page_size
        req_spec.host_addr = 1
        req_spec.va_pages_phy_addr[:] = self.slab.phy_address
        req_spec.table_index = self.tbl_pos
        self.Show()

    def ProcessHALResponse(self, req_spec, resp_spec):
        pass

    def Show(self):
        logger.info('MR: %s PD: %s ' %(self.GID(), self.pd.GID()))
        logger.info('LKey: %d RKey: %d address: 0x%x size: %d '\
                        %(self.lkey, self.rkey, self.slab.address, self.slab.size))

    def GetAccessFlags(self):
        flags = 0
        if self.local_wr is True:
            flags |= 1
        if self.remote_wr is True:
            flags |= 2
        if self.remote_rd is True:
            flags |= 4
        if self.remote_atomic is True:
            flags |= 8

        return flags

    def __get_log_size(self, x):
        power = 1
        log = 0
        while power < x:
            power *= 2
            log += 1
        return log

class MrObjectHelper:
    def __init__(self):
        self.mrs = []
        self.useAdmin = False

    def Generate(self, pd, spec):
        self.pd = pd
        if self.pd.remote:
            logger.info("skipping MR generation for remote PD: %s" %(pd.GID()))
            return

        mr_count = spec.count
        self.useAdmin = spec.useAdmin
        slab_count = len(pd.ep.slabs)
        logger.info('mr_count: %d slab_count: %d' \
                        %(mr_count, slab_count))
        count = min(mr_count, slab_count)
        logger.info("Creating %d Mrs. for PD:%s useAdmin: %d" %\
                       (count, pd.GID(), self.useAdmin))
        for i in range(count):
            slab = pd.ep.slabs.Get('SLAB%04d' % i)
            mr = MrObject(pd, slab, id_substr=None, local_wr=True, remote_wr=True, remote_rd=True, remote_atomic=True)
            self.mrs.append(mr)
            mr = MrObject(pd, slab, id_substr="RONLY", local_wr=False, remote_wr=False, remote_rd=True, remote_atomic=True)
            self.mrs.append(mr)

    def AddMr(self, mr):
        self.mrs.append(mr)

    def Configure(self):
        if self.pd.remote:
            logger.info("skipping MR configuration for remote PD: %s" %(self.pd.GID()))
            return

        logger.info("Configuring %d Mrs." % len(self.mrs))

        if (GlobalOptions.dryrun): return

        if self.useAdmin is True:
            adminapi.ConfigureMrs(self.mrs[0].lif, self.mrs)
        else:
            halapi.ConfigureMrs(self.mrs)
