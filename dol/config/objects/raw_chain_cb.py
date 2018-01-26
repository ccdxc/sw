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

class RawcCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('RAWCCB'))
        return
        
    #def Init(self, spec_obj):
    def Init(self, qid):
        if halapi.IsHalDisabled(): qid = resmgr.RawcCbIdAllocator.get()
        self.id = qid
        gid = "RawcCb%04d" % qid
        self.GID(gid)
        # self.spec = spec_obj
        # cfglogger.info("  - %s" % self)

        # self.uplinks = objects.ObjectDatabase()
        # for uplink_spec in self.spec.uplinks:
            # uplink_obj = uplink_spec.Get(Store)
            # self.uplinks.Set(uplink_obj.GID(), uplink_obj)

        # assert(len(self.uplinks) > 0)
        cfglogger.info("  - %s" % self)

        self.rawccbq = SwDscrRingHelper.main("RAWCCBQ", gid, self.id)
        return


    def PrepareHALRequestSpec(self, req_spec):
        #req_spec.meta.rawccb_id             = self.id
        req_spec.key_or_handle.rawccb_id    = self.id
        if req_spec.__class__.__name__ != 'RawcCbGetRequest':
           req_spec.rawccb_flags                 = self.rawccb_flags
           req_spec.my_txq_base                  = self.my_txq_base
           req_spec.my_txq_ring_size_shift       = self.my_txq_ring_size_shift
           req_spec.my_txq_entry_size_shift      = self.my_txq_entry_size_shift

           req_spec.chain_txq_base               = self.chain_txq_base
           req_spec.chain_txq_ring_indices_addr  = self.chain_txq_ring_indices_addr
           req_spec.chain_txq_ring_size_shift    = self.chain_txq_ring_size_shift
           req_spec.chain_txq_entry_size_shift   = self.chain_txq_entry_size_shift
           req_spec.chain_txq_lif                = self.chain_txq_lif
           req_spec.chain_txq_qtype              = self.chain_txq_qtype
           req_spec.chain_txq_qid                = self.chain_txq_qid
           req_spec.chain_txq_ring               = self.chain_txq_ring

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - RawcCb %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ != 'RawcCbResponse':
            self.rawccb_flags                 = resp_spec.spec.rawccb_flags
            self.my_txq_base                  = resp_spec.spec.my_txq_base
            self.my_txq_ring_size_shift       = resp_spec.spec.my_txq_ring_size_shift
            self.my_txq_entry_size_shift      = resp_spec.spec.my_txq_entry_size_shift

            self.chain_txq_base               = resp_spec.spec.chain_txq_base
            self.chain_txq_ring_indices_addr  = resp_spec.spec.chain_txq_ring_indices_addr
            self.chain_txq_ring_size_shift    = resp_spec.spec.chain_txq_ring_size_shift
            self.chain_txq_entry_size_shift   = resp_spec.spec.chain_txq_entry_size_shift
            self.chain_txq_lif                = resp_spec.spec.chain_txq_lif
            self.chain_txq_qtype              = resp_spec.spec.chain_txq_qtype
            self.chain_txq_qid                = resp_spec.spec.chain_txq_qid
            self.chain_txq_ring               = resp_spec.spec.chain_txq_ring
            self.pi                           = resp_spec.spec.pi
            self.ci                           = resp_spec.spec.ci

            self.stat_pkts_chain              = resp_spec.spec.stat_pkts_chain
            self.stat_pkts_discard            = resp_spec.spec.stat_pkts_discard
            self.stat_cb_not_ready            = resp_spec.spec.stat_cb_not_ready
            self.stat_my_txq_empty            = resp_spec.spec.stat_my_txq_empty
            self.stat_aol_err                 = resp_spec.spec.stat_aol_err
            self.stat_txq_full                = resp_spec.spec.stat_txq_full
            self.stat_desc_sem_free_full      = resp_spec.spec.stat_desc_sem_free_full
            self.stat_page_sem_free_full      = resp_spec.spec.stat_page_sem_free_full

        return

    def GetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.GetRawcCbs(lst)
        return

    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateRawcCbs(lst)
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def Read(self):
        return

    def Write(self):
        return

    #
    # Print rawccb statistics
    #
    def StatsPrint(self):
        print("RAWCCB %d stat_pkts_chain %d" % 
              (self.id, self.stat_pkts_chain))
        print("RAWCCB %d stat_pkts_discard %d" % 
              (self.id, self.stat_pkts_discard))
        print("RAWCCB %d stat_cb_not_ready %d" % 
              (self.id, self.stat_cb_not_ready))
        print("RAWCCB %d stat_my_txq_empty %d" % 
              (self.id, self.stat_my_txq_empty))
        print("RAWCCB %d stat_aol_err %d" % 
              (self.id, self.stat_aol_err))
        print("RAWCCB %d stat_txq_full %d" % 
              (self.id, self.stat_txq_full))
        print("RAWCCB %d stat_desc_sem_free_full %d" % 
              (self.id, self.stat_desc_sem_free_full))
        print("RAWCCB %d stat_page_sem_free_full %d" % 
              (self.id, self.stat_page_sem_free_full))
        return


# Helper Class to Generate/Configure/Manage RawcCb Objects.
class RawcCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(RawcCbObject)
        cfglogger.info("Configuring %d RawcCbs." % len(objlist)) 
        halapi.ConfigureRawcCbs(objlist)
        return
        
    def __gen_one(self, qid):
        cfglogger.info("Creating RawcCb")
        rawccb_obj = RawcCbObject()
        rawccb_obj.Init(qid)
        Store.objects.Add(rawccb_obj)
        return rawccb_obj

    def Generate(self, qid):
        obj = self.__gen_one(qid)
        self.objlist.append(obj)
        lst = []
        lst.append(obj)
        self.Configure(lst)
        return self.objlist

    def main(self, qid):
        gid = "RawcCb%04d" % qid
        if Store.objects.IsKeyIn(gid):
            return Store.objects.Get(gid)
        objlist = self.Generate(qid)
        return objlist

RawcCbHelper = RawcCbObjectHelper()
