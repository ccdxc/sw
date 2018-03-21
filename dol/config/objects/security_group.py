#! /usr/bin/python3
import pdb
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from infra.common.logging       import logger
from config.store               import Store

class SecurityGroupObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('NETWORK'))
        return

    def Init(self, tenant, local, eps, label):
        self.id = resmgr.SecurityGroupAllocator.get()
        self.GID("SG%04d" % self.id)
        self.label = label
        self.tenant = tenant
        self.local = local
        self.eps = eps
        self.Show()

        self.__update_eps()
        return

    def GetLabel(self):
        return self.label

    def __update_eps(self):
        for ep in self.eps:
            logger.info("- Adding SecurityGroup:%s to Ep:%s" % (self.GID(), ep.GID()))
            ep.AddSecurityGroup(self)
            ep.SetLabel(self.label)
        return

    def IsRemote(self):
        return self.local is False

    def IsLocal(self):
        return self.local is True

    def Show(self, detail = False):
        logger.info("SecurityGroup = %s(%d)" % (self.GID(), self.id))
        logger.info("- Tenant    = %s" % self.tenant.GID())
        logger.info("- Local     = %s" % self.local)
        logger.info("- EPs:")
        for e in self.eps:
            logger.info("  - %s" % e.GID())
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.vrf_id = self.tenant.id
        req_spec.key_or_handle.security_group_id = self.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("- SecurityGroup %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.hal_handle = resp_spec.status.sg_handle
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp_spec):
        return

    def Get(self):
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Network Objects.
class SecurityGroupObjectHelper:
    def __init__(self):
        self.sgs = []
        self.lsgs = []
        self.rsgs = []
        return

    def GetLocal(self):
        return self.lsgs

    def GetRemote(self):
        return self.rsgs

    def Configure(self):
        logger.info("Configuring %d Security Groups" % len(self.sgs))
        halapi.ConfigureSecurityGroups(self.sgs)
        return

    def __create(self, tenant, local, eps, label):
        sg = SecurityGroupObject()
        sg.Init(tenant, local, eps, label)
        Store.objects.Set(sg.GID(), sg)
        self.sgs.append(sg)
        if local is True:
            self.lsgs.append(sg)
        else:
            self.rsgs.append(sg)
        return

    def Generate(self, tenant):
        logger.info("Creating Security Group for Tenant = %s" %\
                       (tenant.GID()))
        leps = tenant.GetLocalEps()
        reps = tenant.GetRemoteEps()
        split = 2
        
        self.__create(tenant, False, reps, 'DOS_SRC_DST')
        if len(leps) == 0:
            return
        self.__create(tenant, True, leps[:split], 'DOS_TO_WL')
        self.__create(tenant, True, leps[split:], 'DOS_FROM_WL')
        return

    def AddToSegments(self, tenant):
        if len(self.sgs) == 0: return
        segs = tenant.GetSegments()
        sgidx = 0
        for seg in segs:
            for sg in self.sgs:
                #sg = self.sgs[sgidx]
                seg.AddSecurityGroup(sg)
                #sgidx = (sgidx + 1) % len(self.sgs)
        return

    def main(self, tenant):
        self.Generate(tenant)
        self.Configure()
        self.AddToSegments(tenant)
        return

def GetMatchingObjects(selectors):
    sgs =  Store.objects.GetAllByClass(SecurityGroupObject)
    return [sg for sg in sgs if sg.IsFilterMatch(selectors.security_group)]
