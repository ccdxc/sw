#! /usr/bin/python3
import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils
import config.resmgr            as resmgr
import infra.config.base        as base

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api            as halapi
import config.hal.defs           as haldefs

class SecurityProfileObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SECURITY_PROFILE'))
        self.id = resmgr.SecProfIdAllocator.get()
        return

    def Init(self, spec):
        if spec.fields:
            self.Clone(spec.fields) 
        self.GID(spec.id)
        self.Show()
        return

    def Show(self):
        cfglogger.info("- Security Profile  : %s (id: %d)" %\
                       (self.GID(), self.id))
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.profile_id = self.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.profile_status.profile_handle
        cfglogger.info("  - SecurityProfile %s = %s (HDL = 0x%x)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle))
        return

# Helper Class to Generate/Configure/Manage Security Profile Objects
class SecurityProfileObjectHelper:
    def __init__(self):
        self.sps = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d SecurityProfiles." % len(self.sps)) 
        halapi.ConfigureSecurityProfiles(self.sps)
        return
        
    def Generate(self):
        spec = Store.specs.Get('SECURITY_PROFILES')
        cfglogger.info("Creating %d SecurityProfiles." % len(spec.profiles))
        for p in spec.profiles:
            profile = SecurityProfileObject()
            profile.Init(p)
            self.sps.append(profile)
        Store.objects.SetAll(self.sps)
        return

    def main(self):
        self.Generate()
        self.Configure()
        return

SecurityProfileHelper = SecurityProfileObjectHelper()
