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

class CpuCbObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('CPUCB'))
        return
        
    #def Init(self, spec_obj):
    def Init(self, qid):
        #if halapi.IsHalDisabled(): qid = resmgr.CpuCbIdAllocator.get()
        self.id = qid
        gid = "CpuCb%04d" % qid
        self.GID(gid)
        # self.spec = spec_obj
        # cfglogger.info("  - %s" % self)

        # self.uplinks = objects.ObjectDatabase()
        # for uplink_spec in self.spec.uplinks:
            # uplink_obj = uplink_spec.Get(Store)
            # self.uplinks.Set(uplink_obj.GID(), uplink_obj)

        # assert(len(self.uplinks) > 0)
        cfglogger.info("  - %s" % self)
        self.arq = SwDscrRingHelper.main("ARQ", gid, self.id)

        return


    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.cpucb_id    = self.id
        if req_spec.__class__.__name__ != 'CpuCbGetRequest':
           req_spec.debug_dol                 = self.debug_dol
           if hasattr(self, 'header_template'):
               req_spec.header_template           = self.header_template
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - CpuCb %s = %s" %\
                       (self.id, \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        if resp_spec.__class__.__name__ != 'CpuCbResponse':
            self.debug_dol = resp_spec.spec.debug_dol
        return

    def GetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.GetCpuCbs(lst)
        return

    def SetObjValPd(self):
        lst = []
        lst.append(self)
        halapi.UpdateCpuCbs(lst)
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def Read(self):
        return

    def Write(self):
        return



# Helper Class to Generate/Configure/Manage CpuCb Objects.
class CpuCbObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self, objlist = None):
        if objlist == None:
            objlist = Store.objects.GetAllByClass(CpuCbObject)
        cfglogger.info("Configuring %d CpuCbs." % len(objlist)) 
        halapi.ConfigureCpuCbs(objlist)
        return
        
    def __gen_one(self, qid):
        cfglogger.info("Creating CpuCb")
        cpucb_obj = CpuCbObject()
        cpucb_obj.Init(qid)
        Store.objects.Add(cpucb_obj)
        return cpucb_obj

    def Generate(self, qid):
        obj = self.__gen_one(qid)
        self.objlist.append(obj)
        lst = []
        lst.append(obj)
        self.Configure(lst)
        return self.objlist

    def main(self, qid):
        gid = "CpuCb%04d" % qid
        if Store.objects.IsKeyIn(gid):
            return Store.objects.Get(gid)
        objlist = self.Generate(qid)
        return objlist

CpuCbHelper = CpuCbObjectHelper()
