# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr

from config.store               import Store
from infra.common.logging       import cfglogger
from config.objects.swdr        import SwDscrRingHelper

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class RawrCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('RAWRCB'))
        return
        
    #def Init(self, spec_obj):
    def Init(self, qid):
        if halapi.IsHalDisabled(): qid = resmgr.RawrCbIdAllocator.get()
        self.id = qid
        gid = "RawrCb%04d" % qid
        self.GID(gid)
        # self.spec = spec_obj
        # cfglogger.info("  - %s" % self)

        # self.uplinks = objects.ObjectDatabase()
        # for uplink_spec in self.spec.uplinks:
            # uplink_obj = uplink_spec.Get(Store)
            # self.uplinks.Set(uplink_obj.GID(), uplink_obj)

        # assert(len(self.uplinks) > 0)
        cfglogger.info("  - %s" % self)

        return


    def PrepareHALRequestSpec(self, req_spec):
        #req_spec.meta.rawrcb_id             = self.id
        req_spec.key_or_handle.rawrcb_id    = self.id
        if req_spec.__class__.__name__ != 'RawrCbGetRequest':
           req_spec.rawrcb_flags                 = self.rawrcb_flags
           req_spec.chain_rxq_base               = self.chain_rxq_base
           req_spec.chain_rxq_ring_indices_addr  = self.chain_rxq_ring_indices_addr
           req_spec.chain_rxq_ring_size_shift    = self.chain_rxq_ring_size_shift
           req_spec.chain_rxq_entry_size_shift   = self.chain_rxq_entry_size_shift
           req_spec.chain_rxq_ring_index_select  = self.chain_rxq_ring_index_select

           req_spec.chain_txq_base               = self.chain_txq_base
           req_spec.chain_txq_ring_indices_addr  = self.chain_txq_ring_indices_addr
           req_spec.chain_txq_ring_size_shift    = self.chain_txq_ring_size_shift
           req_spec.chain_txq_entry_size_shift   = self.chain_txq_entry_size_shift
           req_spec.chain_txq_ring_index_select  = self.chain_txq_ring_index_select
           req_spec.chain_txq_lif                = self.chain_txq_lif
           req_spec.chain_txq_qtype              = self.chain_txq_qtype
           req_spec.chain_txq_qid                = self.chain_txq_qid

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - RawrCb %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ != 'RawrCbResponse':
            self.rawrcb_flags                 = resp_spec.spec.rawrcb_flags
            self.chain_rxq_base               = resp_spec.spec.chain_rxq_base
            self.chain_rxq_ring_indices_addr  = resp_spec.spec.chain_rxq_ring_indices_addr
            self.chain_rxq_ring_size_shift    = resp_spec.spec.chain_rxq_ring_size_shift
            self.chain_rxq_entry_size_shift   = resp_spec.spec.chain_rxq_entry_size_shift
            self.chain_rxq_ring_index_select  = resp_spec.spec.chain_rxq_ring_index_select

            self.chain_txq_base               = resp_spec.spec.chain_txq_base
            self.chain_txq_ring_indices_addr  = resp_spec.spec.chain_txq_ring_indices_addr
            self.chain_txq_ring_size_shift    = resp_spec.spec.chain_txq_ring_size_shift
            self.chain_txq_entry_size_shift   = resp_spec.spec.chain_txq_entry_size_shift
            self.chain_txq_ring_index_select  = resp_spec.spec.chain_txq_ring_index_select
            self.chain_txq_lif                = resp_spec.spec.chain_txq_lif
            self.chain_txq_qtype              = resp_spec.spec.chain_txq_qtype
            self.chain_txq_qid                = resp_spec.spec.chain_txq_qid

        return

    def GetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.GetRawrCbs(lst)
        return

    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateRawrCbs(lst)
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def Read(self):
        return

    def Write(self):
        return



# Helper Class to Generate/Configure/Manage RawrCb Objects.
class RawrCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(RawrCbObject)
        cfglogger.info("Configuring %d RawrCbs." % len(objlist)) 
        halapi.ConfigureRawrCbs(objlist)
        return
        
    def __gen_one(self, qid):
        cfglogger.info("Creating RawrCb")
        rawrcb_obj = RawrCbObject()
        rawrcb_obj.Init(qid)
        Store.objects.Add(rawrcb_obj)
        return rawrcb_obj

    def Generate(self, qid):
        obj = self.__gen_one(qid)
        self.objlist.append(obj)
        lst = []
        lst.append(obj)
        self.Configure(lst)
        return self.objlist

    def main(self, qid):
        gid = "RawrCb%04d" % qid
        if Store.objects.IsKeyIn(gid):
            return Store.objects.Get(gid)
        objlist = self.Generate(qid)
        return objlist

RawrCbHelper = RawrCbObjectHelper()
