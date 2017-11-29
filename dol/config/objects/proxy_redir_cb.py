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

class ProxyrCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('PROXYRCB'))
        return
        
    #def Init(self, spec_obj):
    def Init(self, qid):
        if halapi.IsHalDisabled(): qid = resmgr.ProxyrCbIdAllocator.get()
        self.id = qid
        gid = "ProxyrCb%04d" % qid
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
        #req_spec.meta.proxyrcb_id          = self.id
        req_spec.key_or_handle.proxyrcb_id  = self.id
        if req_spec.__class__.__name__ != 'ProxyrCbGetRequest':
           req_spec.proxyrcb_flags               = self.proxyrcb_flags
           req_spec.my_txq_base                  = self.my_txq_base
           req_spec.my_txq_ring_size_shift       = self.my_txq_ring_size_shift
           req_spec.my_txq_entry_size_shift      = self.my_txq_entry_size_shift

           req_spec.chain_rxq_base               = self.chain_rxq_base
           req_spec.chain_rxq_ring_indices_addr  = self.chain_rxq_ring_indices_addr
           req_spec.chain_rxq_ring_size_shift    = self.chain_rxq_ring_size_shift
           req_spec.chain_rxq_entry_size_shift   = self.chain_rxq_entry_size_shift
           req_spec.chain_rxq_ring_index_select  = self.chain_rxq_ring_index_select

           req_spec.ip_sa                        = self.ip_sa
           req_spec.ip_da                        = self.ip_da
           req_spec.sport                        = self.sport
           req_spec.dport                        = self.dport
           req_spec.vrf                          = self.vrf
           req_spec.af                           = self.af
           req_spec.ip_proto                     = self.ip_proto

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - ProxyrCb %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ != 'ProxyrCbResponse':
            self.proxyrcb_flags               = resp_spec.spec.proxyrcb_flags
            self.my_txq_base                  = resp_spec.spec.my_txq_base
            self.my_txq_ring_size_shift       = resp_spec.spec.my_txq_ring_size_shift
            self.my_txq_entry_size_shift      = resp_spec.spec.my_txq_entry_size_shift

            self.chain_rxq_base               = resp_spec.spec.chain_rxq_base
            self.chain_rxq_ring_indices_addr  = resp_spec.spec.chain_rxq_ring_indices_addr
            self.chain_rxq_ring_size_shift    = resp_spec.spec.chain_rxq_ring_size_shift
            self.chain_rxq_entry_size_shift   = resp_spec.spec.chain_rxq_entry_size_shift
            self.chain_rxq_ring_index_select  = resp_spec.spec.chain_rxq_ring_index_select
            self.pi                           = resp_spec.spec.pi
            self.ci                           = resp_spec.spec.ci

            self.ip_sa                        = resp_spec.spec.ip_sa
            self.ip_da                        = resp_spec.spec.ip_da
            self.sport                        = resp_spec.spec.sport
            self.dport                        = resp_spec.spec.dport
            self.vrf                          = resp_spec.spec.vrf
            self.af                           = resp_spec.spec.af
            self.ip_proto                     = resp_spec.spec.ip_proto

        return

    def GetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.GetProxyrCbs(lst)
        return

    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateProxyrCbs(lst)
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def Read(self):
        return

    def Write(self):
        return



# Helper Class to Generate/Configure/Manage ProxyrCb Objects.
class ProxyrCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(ProxyrCbObject)
        cfglogger.info("Configuring %d ProxyrCbs." % len(objlist)) 
        halapi.ConfigureProxyrCbs(objlist)
        return
        
    def __gen_one(self, qid):
        cfglogger.info("Creating ProxyrCb")
        proxyrcb_obj = ProxyrCbObject()
        proxyrcb_obj.Init(qid)
        Store.objects.Add(proxyrcb_obj)
        return proxyrcb_obj

    def Generate(self, qid):
        obj = self.__gen_one(qid)
        self.objlist.append(obj)
        lst = []
        lst.append(obj)
        self.Configure(lst)
        return self.objlist

    def main(self, qid):
        gid = "ProxyrCb%04d" % qid
        if Store.objects.IsKeyIn(gid):
            return Store.objects.Get(gid)
        objlist = self.Generate(qid)
        return objlist

ProxyrCbHelper = ProxyrCbObjectHelper()
