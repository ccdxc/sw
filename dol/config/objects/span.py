# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment
import config.objects.lif       as lif
import config.objects.tunnel    as tunnel

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class SpanSessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return
        
    def Init(self, tenant, spec):
        self.tenant = tenant
        self.segment = tenant.GetSpanSegment()
        self.id = resmgr.SpanSessionIdAllocator.get()
        self.GID("SpanSession%04d" % self.id)
        self.spec = spec
        self.type = spec.type.upper()
        self.dscp = None

        if self.IsLocal():
            local_eps = tenant.GetLocalEps()
            self.ep   = local_eps[0]
        elif self.IsRspan():
            eps = tenant.GetRemoteEps()
            self.ep = eps[0]
        elif self.IsErspan():
            eps = tenant.GetRemoteEps()
            self.ep = eps[1]
            self.dscp = spec.dscp.get()
        else:
            assert(0)
       
        self.intf = self.ep.intf
        self.Show()
        return

    def Show(self):
        cfglogger.info("Span Session: %s" % self.GID())
        cfglogger.info("- Type      : %s" % self.type)
        if self.IsLocal():
            cfglogger.info("- Interface : %s" % self.intf.GID())
        elif self.IsRspan():
            cfglogger.info("- Interface : %s" % self.intf.GID())
            if self.tenant.IsOverlayVxlan():
                cfglogger.info("- EncapType : VXLAN")
                cfglogger.info("- EncapVal  : %s" % self.segment.vxlan_id)
            else:
                cfglogger.info("- EncapType : VLAN")
                cfglogger.info("- EncapVal  : %s" % self.segment.vlan_id)
        elif self.IsErspan():
            cfglogger.info("- Dest IP   : %s" % self.ep.GetIpAddress().get())
            cfglogger.info("- DSCP      : %s" % self.dscp)
        return

    def IsLocal(self):
        return self.type == 'LOCAL'
    def IsRspan(self):
        return self.type == 'RSPAN'
    def IsErspan(self):
        return self.type == 'ERSPAN'

    def PrepareHALRequestSpec(self, reqspec):
        #reqspec.meta.tenant_id          = self.id
        #reqspec.key_or_handle.tenant_id = self.id
        #reqspec.security_profile_handle = self.security_profile.hal_handle
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - SpanSession %s = %s" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Tenant Objects.
class SpanSessionObjectHelper:
    def __init__(self):
        self.span_ssns = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Span Sessions." % len(self.span_ssns)) 
        halapi.ConfigureSpanSessions(self.span_ssns)
        return
        
    def Generate(self, tenant, topospec):
        for entry in topospec.span_sessions:
            spec = entry.spec.Get(Store)
            span_ssn = SpanSessionObject()
            span_ssn.Init(tenant, spec)
            self.span_ssns.append(span_ssn)
        Store.objects.SetAll(self.span_ssns)
        return

    def main(self, tenant, topospec):
        self.Generate(tenant, topospec)
        #self.Configure()
        return
