# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment
import config.objects.lif       as lif
import config.objects.tunnel    as tunnel
import config.objects.span      as span
import config.objects.l4lb      as l4lb

from config.store               import Store
from infra.common.logging       import cfglogger
from infra.common.glopts        import GlobalOptions

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class TenantObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('TENANT'))
        return
        
    def Init(self, spec, lifns, topospec):
        self.id = resmgr.TenIdAllocator.get()
        gid = "Ten%04d" % self.id
        self.GID(gid)

        self.hostpinned = GlobalOptions.hostpin
        self.spec = spec
        self.type = spec.type.upper()
        self.qos_enable = getattr(topospec, 'qos_enable', False)

        self.overlay = spec.overlay.upper()
        self.security_profile = None
        self.bhseg = None
        self.security_profile_handle = 0
        if spec.security_profile:
            self.security_profile = spec.security_profile.Get(Store)
        if self.IsInfra():
            self.subnet     = resmgr.TepIpSubnetAllocator.get()
            self.ipv4_pool  = resmgr.CreateIpv4AddrPool(self.subnet.get())
            self.local_tep  = self.ipv4_pool.get()
        self.Show()
        
        # Process LIFs
        self.lifns = lifns  
        self.obj_helper_lif = lif.LifObjectHelper()
        self.__create_lifs()
        # Process Segments 
        self.obj_helper_segment = segment.SegmentObjectHelper()
        self.__create_segments()
        # Process L4LbServices
        self.obj_helper_l4lb = l4lb.L4LbServiceObjectHelper()
        self.__create_l4lb_services()
        # Process Tunnels
        if self.IsInfra():
            self.obj_helper_tunnel = tunnel.TunnelObjectHelper()
            self.__create_tunnels()

        # Process Span Sessions
        if self.IsSpan():
            self.obj_helper_span = span.SpanSessionObjectHelper()
            self.__create_span_sessions()
        return


    def __copy__(self):
        ten = TenantObject()
        ten.id = self.id
        ten.security_profile_handle = self.security_profile_handle
        ten.security_profile = self.security_profile
        return ten


    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False
        fields = ["id", "security_profile_handle"]
        if not self.CompareObjectFields(other, fields, lgh):
            return False        
        return True
    
    def Show(self):
        cfglogger.info("Tenant  : %s" % self.GID())
        cfglogger.info("- Type      : %s" % self.type)
        cfglogger.info("- HostPinned: %s" % self.hostpinned)
        if self.IsInfra():
            cfglogger.info("- LocalTep  : %s" % self.local_tep.get())
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/Type:%s' % self.type
        if self.IsInfra():
            summary += '/LocTep:%s' % self.local_tep.get()
        return summary

    def IsInfra(self):
        return self.type == 'INFRA'
    def IsSpan(self):
        return self.type == 'SPAN'
    def IsTenant(self):
        return self.type == 'TENANT'
    def IsHostPinned(self):
        return self.hostpinned
    def IsQosEnabled(self):
        return self.qos_enable

    # Comment this before removing. Vxlan or Vlan is a segment level prop.
    #def IsOverlayVxlan(self):
    #    return self.overlay == 'VXLAN'
    #def IsOverlayVlan(self):
    #    return self.overlay == 'VLAN'

    def IsL4LbEnabled(self):
        return self.l4lb_enable == True
    def AllocL4LbBackend(self, remote):
        return self.obj_helper_segment.AllocL4LbBackend(remote)
   
    def __create_l4lb_services(self):
        if 'l4lb' not in self.spec.__dict__:
            self.l4lb_enable = False
            return

        self.l4lb_enable = True
        spec = self.spec.l4lb.Get(Store)
        self.obj_helper_l4lb.Generate(self, spec)
        return

    def __create_segments(self):
        for entry in self.spec.segments:
            spec = entry.spec.Get(Store)
            self.obj_helper_segment.Generate(self, spec, entry.count)
        self.obj_helper_segment.AddToStore()
        self.bhseg = self.obj_helper_segment.GetBlackholeSegment()
        return

    def __create_lifs(self):
        self.spec.lif = self.spec.lif.Get(Store)
        self.obj_helper_lif.Generate(self, self.spec.lif, self.lifns)
        self.obj_helper_lif.Configure()
        return

    def __create_tunnels(self):
        for entry in self.spec.tunnels:
            spec = entry.spec.Get(Store)
            self.obj_helper_tunnel.Generate(self, spec, self.GetEps())
        self.obj_helper_tunnel.AddToStore()
        return

    def __create_span_sessions(self):
        self.obj_helper_span.main(self, self.spec)
        return 

    def AllocLif(self):
        return self.obj_helper_lif.Alloc()

    def ConfigureSegments(self):
        return self.obj_helper_segment.Configure()

    def ConfigureSegmentsPhase2(self):
        return self.obj_helper_segment.ConfigurePhase2()

    def ConfigureTunnels(self):
        if self.IsInfra():
            self.obj_helper_tunnel.Configure()
        return
    
    def ConfigureL4LbServices(self):
        return self.obj_helper_l4lb.Configure()

    def GetSegments(self):
        return self.obj_helper_segment.segs

    def GetSpanSegment(self):
        for seg in self.obj_helper_segment.segs:
            if seg.IsSpanSegment():
                return seg
        return None

    def GetL4LbServices(self):
        return self.obj_helper_l4lb.svcs

    def GetEps(self, backend = False):
        return self.obj_helper_segment.GetEps(backend)

    def GetRemoteEps(self, backend = False):
        return self.obj_helper_segment.GetRemoteEps(backend)

    def GetLocalEps(self, backend = False):
        return self.obj_helper_segment.GetLocalEps(backend)

    def PrepareHALRequestSpec(self, reqspec):
        reqspec.meta.tenant_id          = self.id
        reqspec.key_or_handle.tenant_id = self.id
        if self.security_profile:
            reqspec.security_profile_handle = self.security_profile.hal_handle
            self.security_profile_handle = self.security_profile.hal_handle
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - Tenant %s = %s" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.meta.tenant_id = self.id
        get_req_spec.key_or_handle.tenant_id = self.id
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp_spec):
        if get_resp_spec.api_status == haldefs.common.ApiStatus.Value('API_STATUS_OK'):
            self.id = get_resp_spec.spec.meta.tenant_id
            self.security_profile_handle = get_resp_spec.spec.security_profile_handle
        else:
            self.security_profile_handle = None
        return

    def Get(self):
        halapi.GetTenants([self])

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Tenant Objects.
class TenantObjectHelper:
    def __init__(self):
        self.tens = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Tenants." % len(self.tens)) 
        halapi.ConfigureTenants(self.tens)
        for ten in self.tens:
            ten.ConfigureSegments()
            ten.ConfigureL4LbServices()
        return
        
    def ConfigurePhase2(self):
        for ten in self.tens:
            ten.ConfigureSegmentsPhase2()
            ten.ConfigureTunnels()
        return

    def Generate(self, topospec):
        tenspec = getattr(topospec, 'tenants', None)
        if tenspec is None:
            return
        for entry in topospec.tenants:
            spec = entry.spec.Get(Store)
            cfglogger.info("Creating %d Tenants" % entry.count)
            for c in range(entry.count):
                ten = TenantObject()
                ten.Init(spec, entry.lifns, topospec)
                self.tens.append(ten)
        Store.objects.SetAll(self.tens)
        return

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        return


TenantHelper = TenantObjectHelper()

def GetMatchingObjects(selectors):
    tenants =  Store.objects.GetAllByClass(TenantObject)
    return [ten for ten in tenants if ten.IsFilterMatch(selectors.tenant)]
