#! /usr/bin/python3
import pdb
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.endpoint  as endpoint
import config.objects.enic      as enic
import config.objects.network   as nw

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.logging       import cfglogger
from config.store               import Store

class SegmentObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SEGMENT'))
        return

    def Init(self, tenant, spec):
        self.id = resmgr.SegIdAllocator.get()
        self.GID("Seg%04d" % self.id)

        self.spec   = spec
        self.tenant = tenant
        self.type   = spec.type.upper()
        self.native = spec.native

        self.vlan_id    = resmgr.SegVlanAllocator.get()
        self.vxlan_id   = resmgr.SegVxlanAllocator.get()
        self.macaddr    = resmgr.RouterMacAllocator.get()
        if self.IsInfraSegment():
            self.subnet     = resmgr.TepIpSubnetAllocator.get()
            self.subnet6    = resmgr.TepIpv6SubnetAllocator.get()
        elif self.IsTenantSegment() or self.IsSpanSegment():
            self.subnet     = resmgr.IpSubnetAllocator.get()
            self.subnet6    = resmgr.Ipv6SubnetAllocator.get()
        else:
            assert(0)

        self.ip_addr_allocator      = resmgr.CreateIpAddrAllocator(self.subnet.get())
        self.ipv6_addr_allocator    = resmgr.CreateIpv6AddrAllocator(self.subnet6.get())

        policy = "BROADCAST_FWD_POLICY_" + spec.broadcast.upper()
        self.broadcast_policy = haldefs.segment.BroadcastFwdPolicy.Value(policy)

        policy = "MULTICAST_FWD_POLICY_" + spec.multicast.upper()
        self.multicast_policy = haldefs.segment.MulticastFwdPolicy.Value(policy)
        
        self.obj_helper_ep = endpoint.EndpointObjectHelper()
        self.obj_helper_enic = enic.EnicObjectHelper()
        self.obj_helper_nw = nw.NetworkObjectHelper()   

        self.hal_handle = None

        self.Show()
        self.obj_helper_nw.Generate(self)
        self.obj_helper_nw.Configure()

        self.obj_helper_enic.Generate(self, self.spec.endpoints)
        self.obj_helper_ep.Generate(self, self.spec.endpoints)
        self.Show(detail = True)
        return

    def IsTenantSegment(self):
        return self.type == 'TENANT'
    def IsInfraSegment(self):
        return self.type == 'INFRA'
    def IsSpanSegment(self):
        return self.type == 'SPAN'

    def Show(self, detail = False):
        cfglogger.info("- Segment = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("  - VLAN       = %d" % self.vlan_id)
        if not self.IsInfraSegment():
            cfglogger.info("  - VXLAN      = %s" % self.vxlan_id)
        cfglogger.info("  - MAC        = %s" % self.macaddr.get())
        cfglogger.info("  - Subnet     = %s" % self.subnet.get())
        cfglogger.info("  - Subnet6    = %s" % self.subnet6.get())
        cfglogger.info("  - Broadcast  = %s" % self.spec.broadcast)
        cfglogger.info("  - Multicast  = %s" % self.spec.multicast)

        if detail == False: return
        self.obj_helper_ep.Show()
        self.obj_helper_enic.Show()
        return

    def GetDirectEnics(self):
        return self.obj_helper_enic.direct
    def GetPvlanEnics(self):
        return self.obj_helper_enic.pvlan
    def GetUsegEnics(self):
        return self.obj_helper_enic.useg
    def GetEps(self):
        return self.obj_helper_ep.eps
    def GetLocalEps(self):
        return self.obj_helper_ep.local
    def GetRemoteEps(self):
        return self.obj_helper_ep.remote

    def ConfigureEndpoints(self):
        return self.obj_helper_ep.Configure()

    def ConfigureEnics(self):
        return self.obj_helper_enic.Configure()

    def ConfigureChildren(self):
        self.ConfigureEnics()
        self.ConfigureEndpoints()
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.tenant_id = self.tenant.id
        req_spec.key_or_handle.segment_id = self.id
        if self.IsTenantSegment() or self.IsSpanSegment():
            req_spec.segment_type = haldefs.common.L2_SEGMENT_TYPE_TENANT
        elif self.IsInfraSegment():
            req_spec.segment_type = haldefs.common.L2_SEGMENT_TYPE_INFRA
        else:
            assert(0)
        req_spec.access_encap.encap_type    = haldefs.common.ENCAP_TYPE_DOT1Q
        req_spec.access_encap.encap_value   = self.vlan_id
        if self.tenant.IsOverlayVxlan():
            req_spec.fabric_encap.encap_type    = haldefs.common.ENCAP_TYPE_VXLAN
            req_spec.fabric_encap.encap_value   = self.vxlan_id
        else:
            req_spec.fabric_encap.encap_type    = haldefs.common.ENCAP_TYPE_DOT1Q
            req_spec.fabric_encap.encap_value   = self.vlan_id
        req_spec.mcast_fwd_policy = self.multicast_policy
        req_spec.bcast_fwd_policy = self.broadcast_policy

        for nw in self.obj_helper_nw.nws:
            if nw.hal_handle:
                req_spec.network_handle.append(nw.hal_handle)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("- Segment %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.hal_handle = resp_spec.l2segment_status.l2segment_handle
        return
    
    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Segment Objects.
class SegmentObjectHelper:
    def __init__(self):
        self.segs = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Segments." % len(self.segs)) 
        halapi.ConfigureSegments(self.segs)
        for seg in self.segs:
            seg.ConfigureChildren()
        return

    def Generate(self, tenant, spec, count):
        cfglogger.info("Creating %d Segments for Tenant = %s" %\
                       (count, tenant.GID()))
        for s in range(count):
            seg = SegmentObject()
            seg.Init(tenant, spec)
            self.segs.append(seg)
        return

    def AddToStore(self):
        Store.objects.SetAll(self.segs)
        return

    def GetEps(self):
        eps = []
        for seg in self.segs:
            eps += seg.GetEps()
        return eps

    def GetLocalEps(self):
        eps = []
        for seg in self.segs:
            eps += seg.GetLocalEps()
        return eps

    def GetRemoteEps(self):
        eps = []
        for seg in self.segs:
            eps += seg.GetRemoteEps()
        return eps
