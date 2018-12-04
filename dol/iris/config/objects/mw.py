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

class MwObject(base.ConfigObjectBase):
    def __init__(self, pd, mw_type,
                 remote_wr=False, remote_rd=False,
                 remote_atomic=False):
        super().__init__()
        self.Clone(Store.templates.Get('RDMA_MW'))
        self.pd = pd
        self.lif = pd.ep.intf.lif
        self.rkey = pd.ep.intf.lif.GetMrKey() << 8
        if not mw_type:
            self.GID('MW-TYPE1_2-%s' %(self.rkey))
        else :
            self.GID('MW-TYPE%s-%s' %(mw_type, self.rkey))

        self.remote_wr = remote_wr
        self.remote_rd = remote_rd
        self.remote_atomic = remote_atomic
        self.mw_type = mw_type


        self.flags = self.GetAccessFlags()

        return

    def GetAccessFlags(self):
        flags = 0
        if self.remote_wr is True:
            flags |= 2
        if self.remote_rd is True:
            flags |= 4
        if self.remote_atomic is True:
            flags |= 8
        #inv_en
        if self.mw_type == 2:
            flags |= (128 << 8)
        #is_mw
        flags |= (64 << 8)

        return flags


    def PrepareAdminRequestSpec(self, req_spec):
        if (GlobalOptions.dryrun): return

        logger.info("MW: %s PD: %s LIF: %s rkey: %s remote_wr %s remote_rd %s" %\
                    (self.GID(), self.pd.GID(),
                     self.pd.ep.intf.lif.hw_lif_id, self.rkey,
                     self.remote_wr, self.remote_rd))

        # op = IONIC_V1_ADMIN_REG_MR
        req_spec.op = 3
        req_spec.dbid_flags = self.flags
        req_spec.id_ver = self.rkey
        req_spec.pd_id = self.pd.id
        #for alloc_lkey, set the following to 0. frpmr will provide them
        req_spec.tbl_index = 0
        req_spec.map_count = 0
        req_spec.dma_addr = 0
        req_spec.va = 0
        req_spec.length = 0
        req_spec.page_size_log2 = 0
        req_spec.host_addr = 0


    def PrepareHALRequestSpec(self, req_spec):
        logger.info("MW: %s PD: %s LIF: %s rkey: %s remote_wr %s remote_rd %s" %\
                    (self.GID(), self.pd.GID(),
                     self.pd.ep.intf.lif.hw_lif_id, self.rkey,
                     self.remote_wr, self.remote_rd))

        if (GlobalOptions.dryrun): return

        req_spec.hw_lif_id = self.pd.ep.intf.lif.hw_lif_id
        req_spec.pd = self.pd.id
        req_spec.ac_remote_wr = self.remote_wr
        req_spec.ac_remote_rd = self.remote_rd
        req_spec.ac_remote_atomic = self.remote_atomic
        req_spec.rkey = self.rkey
        req_spec.mw_type = self.mw_type
        self.Show()

    def ProcessHALResponse(self, req_spec, resp_spec):
        pass

    def Show(self):
        logger.info('MW: %s PD: %s ' %(self.GID(), self.pd.GID()))
        logger.info('RKey: %d ' %(self.rkey))

class MwObjectHelper:
    def __init__(self):
        self.mws = []
        self.useAdmin = False

    def Generate(self, pd, spec):
        self.pd = pd
        self.useAdmin = spec.useAdmin
        if self.pd.remote:
            logger.info("skipping MW generation for remote PD: %s" %(pd.GID()))
            return

        mw_count = spec.type1.count
        logger.info("Creating %d Type1 Mws. for PD:%s" %(mw_count, pd.GID()))
        for i in range(mw_count):
            mw = MwObject(pd, remote_wr=True, remote_rd=True, remote_atomic=True, mw_type=1)
            self.mws.append(mw)

        mw_count = spec.type2.count
        logger.info("Creating %d Type2 Mws. for PD:%s" %(mw_count, pd.GID()))
        for i in range(mw_count):
            mw = MwObject(pd, remote_wr=True, remote_rd=True, remote_atomic=True, mw_type=2)
            self.mws.append(mw)

        mw_count = spec.type1_2.count
        logger.info("Creating %d Type1_2 Mws. for PD:%s" %(mw_count, pd.GID()))
        for i in range(mw_count):
            mw = MwObject(pd, remote_wr=True, remote_rd=True, remote_atomic=True, mw_type=0)
            self.mws.append(mw)

    def AddMw(self, mw):
        self.mws.append(mw)

    def Configure(self):
        if self.pd.remote:
            logger.info("skipping MW configuration for remote PD: %s" %(self.pd.GID()))
            return

        if (GlobalOptions.dryrun): return

        logger.info("Configuring %d Mws." % len(self.mws))

        if self.useAdmin is True:
            adminapi.ConfigureMrs(self.mws[0].lif, self.mws)
        else:
            halapi.ConfigureMws(self.mws)
