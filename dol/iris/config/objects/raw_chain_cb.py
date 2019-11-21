# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import iris.config.resmgr            as resmgr

from iris.config.store               import Store
from infra.common.logging       import logger
from iris.config.objects.swdr        import SwDscrRingHelper

import iris.config.hal.defs          as haldefs
import iris.config.hal.api           as halapi

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
        # logger.info("  - %s" % self)

        # self.uplinks = objects.ObjectDatabase()
        # for uplink_spec in self.spec.uplinks:
            # uplink_obj = uplink_spec.Get(Store)
            # self.uplinks.Set(uplink_obj.GID(), uplink_obj)

        # assert(len(self.uplinks) > 0)
        logger.info("  - %s" % self)

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
           req_spec.cpu_id                       = self.cpu_id
           req_spec.ascq_base                    = self.ascq_base
           req_spec.ascq_sem_inf_addr            = self.ascq_sem_inf_addr

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("  - RawcCb %s = %s" %\
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
            self.cpu_id                       = resp_spec.spec.cpu_id
            self.ascq_base                    = resp_spec.spec.ascq_base
            self.ascq_sem_inf_addr            = resp_spec.spec.ascq_sem_inf_addr
            self.pi                           = resp_spec.spec.pi
            self.ci                           = resp_spec.spec.ci

            self.chain_pkts                   = resp_spec.spec.chain_pkts
            self.cb_not_ready_discards        = resp_spec.spec.cb_not_ready_discards
            self.qstate_cfg_discards          = resp_spec.spec.qstate_cfg_discards
            self.aol_error_discards           = resp_spec.spec.aol_error_discards
            self.my_txq_empty_discards        = resp_spec.spec.my_txq_empty_discards
            self.txq_full_discards            = resp_spec.spec.txq_full_discards
            self.pkt_free_errors              = resp_spec.spec.pkt_free_errors

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
        logger.info("RAWCCB %d chain_pkts %d" % 
              (self.id, self.chain_pkts))
        logger.info("RAWCCB %d cb_not_ready_discards %d" % 
              (self.id, self.cb_not_ready_discards))
        logger.info("RAWCCB %d qstate_cfg_discards %d" % 
              (self.id, self.qstate_cfg_discards))
        logger.info("RAWCCB %d aol_error_discards %d" % 
              (self.id, self.aol_error_discards))
        logger.info("RAWCCB %d my_txq_empty_discards %d" % 
              (self.id, self.my_txq_empty_discards))
        logger.info("RAWCCB %d txq_full_discards %d" % 
              (self.id, self.txq_full_discards))
        logger.info("RAWCCB %d pkt_free_errors %d" % 
              (self.id, self.pkt_free_errors))
        return


# Helper Class to Generate/Configure/Manage RawcCb Objects.
class RawcCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(RawcCbObject)
        logger.info("Configuring %d RawcCbs." % len(objlist)) 
        halapi.ConfigureRawcCbs(objlist)
        return
        
    def __gen_one(self, qid):
        logger.info("Creating RawcCb")
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
