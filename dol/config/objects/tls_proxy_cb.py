# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr

from config.store                      import Store
from infra.common.logging              import cfglogger
from config.objects.swdr               import SwDscrRingHelper
from config.objects.crypto_keys        import CryptoKeyHelper

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class TlsCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('TLSCB'))
        return
        
    # def Init(self, spec_obj):
    def Init(self ,tcpcb):
        self.id = resmgr.TlsCbIdAllocator.get()
        gid = "TlsCb%04d" % self.id
        self.GID(gid)
        cfglogger.info("  - %s" % self)
        self.tcpcb = tcpcb 
        self.serq = SwDscrRingHelper.main("SERQ", gid, self.id)
        self.bsq = SwDscrRingHelper.main("BSQ", gid, self.id)
        self.crypto_obj = CryptoKeyHelper.main() 
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.tlscb_id    = self.id
        if req_spec.__class__.__name__ != 'TlsCbGetRequest':
            req_spec.debug_dol                 = self.debug_dol
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - TlsCb %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ != 'TlsCbResponse':
            self.serq_pi = resp_spec.spec.serq_pi
            self.serq_ci = resp_spec.spec.serq_ci
            self.bsq_pi = resp_spec.spec.bsq_pi
            self.bsq_ci = resp_spec.spec.bsq_ci
            self.tnmdr_alloc = resp_spec.spec.tnmdr_alloc
            self.tnmpr_alloc = resp_spec.spec.tnmpr_alloc
            self.enc_requests = resp_spec.spec.enc_requests
            self.dec_requests = resp_spec.spec.dec_requests
            self.rnmdr_free = resp_spec.spec.rnmdr_free
            self.rnmpr_free = resp_spec.spec.rnmpr_free
            self.enc_completions = resp_spec.spec.enc_completions
            self.dec_completions = resp_spec.spec.dec_completions
            self.enc_failures = resp_spec.spec.enc_failures
            self.dec_failures = resp_spec.spec.dec_failures
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def GetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.GetTlsCbs(lst)
        return
    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateTlsCbs(lst)
        return

    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateTlsCbs(lst)
        return

    def Read(self):
        return

    def Write(self):
        return



# Helper Class to Generate/Configure/Manage TlsCb Objects.
class TlsCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(TlsCbObject)
        cfglogger.info("Configuring %d TlsCbs." % len(objlist)) 
        halapi.ConfigureTlsCbs(objlist)
        return
        
    def __gen_one(self, tcpcb):
        cfglogger.info("Creating TlsCb")
        tlscb_obj = TlsCbObject()
        tlscb_obj.Init(tcpcb)
        Store.objects.Add(tlscb_obj)
        return tlscb_obj

    def Generate(self, tcpcb):
        self.objlist.append(self.__gen_one(tcpcb))
        return self.objlist

    def main(self, tcpcb):
        objlist = self.Generate(tcpcb)
        self.Configure(self.objlist)
        return objlist

TlsCbHelper = TlsCbObjectHelper()
