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

class KeyObject(base.ConfigObjectBase):
    def __init__(self, pd, remote_access=False):
        super().__init__()
        self.Clone(Store.templates.Get('RDMA_KEY'))
        self.pd = pd
        self.lif = pd.ep.intf.lif
        self.lkey = self.lif.GetMrKey() << 8
        self.rkey = self.lkey
        self.GID('KEY-%s' %(self.lkey))
        self.remote_access = remote_access
        # Hardcoding to 10 for now. This will go in to num-entries-rsvd in key table during alloc_lkey
        self.num_pt_entries = 10
        self.flags = self.GetAccessFlags()
        self.flags |= self.GetFuncFlags() << 8

        return

    def PrepareAdminRequestSpec(self, req_spec):
        if (GlobalOptions.dryrun): return

        self.tbl_pos = self.pd.ep.intf.lif.GetRdmaTblPos(self.num_pt_entries)
        logger.info("MR: %s PD: %s LIF: %s lkey: %s TBL_POS: %d" %\
                    (self.GID(), self.pd.GID(), 
                     self.pd.ep.intf.lif.hw_lif_id, self.lkey, self.tbl_pos))
        # op = IONIC_V1_ADMIN_REG_MR
        req_spec.op = 3
        req_spec.dbid_flags = self.flags
        req_spec.id_ver = self.lkey
        req_spec.pd_id = self.pd.id
        req_spec.tbl_index = self.tbl_pos
        #for alloc_lkey, set the following to 0. frpmr will provide them
        req_spec.map_count = 0
        req_spec.dma_addr = 0
        req_spec.va = 0
        req_spec.length = 0
        req_spec.page_size_log2 = 0
        req_spec.host_addr = 1


    def PrepareHALRequestSpec(self, req_spec):
        logger.info("Key: %s PD: %s LIF: %s lkey: %s remote_access %s" %\
                    (self.GID(), self.pd.GID(),
                     self.pd.ep.intf.lif.hw_lif_id, self.lkey,
                     self.remote_access))

        if (GlobalOptions.dryrun): return

        req_spec.hw_lif_id = self.pd.ep.intf.lif.hw_lif_id
        req_spec.pd = self.pd.id
        req_spec.remote_access = self.remote_access
        req_spec.lkey = self.lkey
        req_spec.rkey = self.rkey
        req_spec.num_pt_entries_rsvd = self.num_pt_entries
        req_spec.host_addr = 1
        self.Show()

    def ProcessHALResponse(self, req_spec, resp_spec):
        pass

    def Show(self):
        logger.info('Key: %s PD: %s ' %(self.GID(), self.pd.GID()))
        logger.info('LKey: %d ' %(self.lkey))

    def GetFuncFlags(self):
        flags = 0
        #INV_EN
        flags |= 128
        #UKEY_EN
        flags |= 32

        return flags

    def GetAccessFlags(self):
        return 0

class KeyObjectHelper:
    def __init__(self):
        self.keys = []
        self.useAdmin = False

    def Generate(self, pd, spec):
        self.pd = pd
        self.useAdmin = spec.useAdmin
        if self.pd.remote:
            logger.info("skipping Key generation for remote PD: %s" %(pd.GID()))
            return

        key_count = spec.count
        logger.info("Creating %d Keys. for PD:%s, useAdmin: %d" %\
                    (key_count, pd.GID(), self.useAdmin))
        for i in range(key_count):
            key = KeyObject(pd, remote_access=True)
            self.keys.append(key)

    def AddKey(self, key):
        self.keys.append(key)

    def Configure(self):
        if self.pd.remote:
            logger.info("skipping Key configuration for remote PD: %s" %(self.pd.GID()))
            return

        if (GlobalOptions.dryrun): return

        logger.info("Configuring %d Keys." % len(self.keys))
        if self.useAdmin is True:
            adminapi.ConfigureMrs(self.keys[0].lif, self.keys)
        else:
            halapi.ConfigureKeys(self.keys)
