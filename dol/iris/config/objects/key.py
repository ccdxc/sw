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
from infra.common.glopts import GlobalOptions

class KeyObject(base.ConfigObjectBase):
    def __init__(self, pd, remote_access=False):
        super().__init__()
        self.Clone(Store.templates.Get('RDMA_KEY'))
        self.pd = pd
        self.lkey = pd.ep.intf.lif.GetMrKey()
        self.rkey = self.lkey
        self.GID('KEY-%s' %(self.lkey))
        self.remote_access = remote_access
        # Hardcoding to 10 for now. This will go in to num-entries-rsvd in key table during alloc_lkey
        self.num_pt_entries = 10
        return

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
        self.Show()

    def ProcessHALResponse(self, req_spec, resp_spec):
        pass

    def Show(self):
        logger.info('Key: %s PD: %s ' %(self.GID(), self.pd.GID()))
        logger.info('LKey: %d ' %(self.lkey))

class KeyObjectHelper:
    def __init__(self):
        self.keys = []

    def Generate(self, pd, spec):
        self.pd = pd
        if self.pd.remote:
            logger.info("skipping Key generation for remote PD: %s" %(pd.GID()))
            return

        key_count = spec.count
        logger.info("Creating %d Keys. for PD:%s" %(key_count, pd.GID()))
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
        halapi.ConfigureKeys(self.keys)
