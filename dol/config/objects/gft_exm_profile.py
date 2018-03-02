#! /usr/bin/python3

import infra.config.base        as base
import config.resmgr            as resmgr

import config.hal.api           as halapi
import config.hal.defs          as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import cfglogger
from config.store               import Store

class GftExmProfileObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('GFT_EXM_PROFILE'))
        self.LockAttributes()
        return

    def Init(self, spec):
        self.GID(spec.name)
        self.id = resmgr.GftExmProfileIdAllocator.get()
        self.spec = spec
        self.rdma_flow = getattr(spec, 'rdma', False)
        self.table_type  = spec.table
        self.groups = []

        self.__process_groups()
        self.Show()
        return

    def __process_groups(self):
        for g in self.spec.groups:
            group = g.Get(Store)
            self.groups.append(group)
        return

    def Show(self):
        cfglogger.info("Creating GFT EXM Profile Object = %s ID=%d" %\
                       (self.GID(), self.id))
        cfglogger.info("- rdma_flow     : ", self.rdma_flow)
        cfglogger.info("- table_type    : ", self.table_type)
        cfglogger.info("- Header Groups:")
        for g in self.groups:
            cfglogger.info("  - Group: %s" % g.GID())
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/ID:%d' % self.id
        summary += '/Groups:%d' + len(self.layers)
        return summary

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.profile_id = self.id
        req_spec.rdma_flow = self.rdma_flow
        tts = 'GFT_TABLE_TYPE_' + self.table_type
        req_spec.table_type = haldefs.gft.GftTableType.Value(tts)

        for group in self.groups:
            grp_req_spec = req_spec.exact_match_profiles.add()
            group.PrepareHALRequestSpec(grp_req_spec)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)


class GftExmProfileObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d GFT EXM Profiles." % len(self.objlist))
        halapi.ConfigureGftExmProfiles(self.objlist)
        return

    def ReConfigure(self):
        cfglogger.info("Updating %d GFT EXM Profiles." % len(self.objlist))
        #halapi.ConfigureGftExmProfiles(self.objlist, update = True)
        return

    def Generate(self, topospec):
        gftexm_spec = getattr(topospec, 'gftexm', None)
        if gftexm_spec is None: return

        spec = getattr(gftexm_spec, 'profiles', None)
        pfs = spec.Get(Store)

        cfglogger.info("Adding GFT EXM Profiles to Store.")

        for entry in pfs.profiles:
            profile = entry.profile
            obj = GftExmProfileObject()
            obj.Init(profile)
            self.objlist.append(obj)
        return

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        cfglogger.info("Adding %d GFT EXM Profiles to Store." % len(self.objlist))
        if len(self.objlist) == 0: return
        Store.objects.SetAll(self.objlist)
        return

    def GetAll(self):
        return self.objlist

GftExmProfileHelper = GftExmProfileObjectHelper()
