# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
import iris.config.objects.segment   as segment
import iris.config.objects.lif       as lif
import iris.config.objects.tunnel    as tunnel

from iris.config.store               import Store
from infra.common.logging       import logger

import iris.config.hal.defs          as haldefs
import iris.config.hal.api           as halapi
import telemetry_pb2            as telemetry_pb2

class SpanSessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SPAN_SESSION'))
        return

    def Init(self, tenant, spec):
        self.tenant = tenant
        self.segment = tenant.GetSpanSegment()
        self.id = resmgr.SpanSessionIdAllocator.get()
        self.GID("SpanSession%04d" % self.id)
        self.spec = spec
        self.snaplen = spec.snaplen.get()
        self.type = spec.type.upper()
        self.dscp = None
        self.erspan_dest = None
        self.erspan_src = None
        eps = tenant.GetRemoteEps()
        self.ep = eps[(self.id -1) %len(eps)]
        self.intf = self.ep.intf
        self.Show()
        return

    def Show(self):
        logger.info("Span Session: %s" % self.GID())
        logger.info("- Type      : %s" % self.type)
        if self.IsLocal():
            logger.info("- Interface : %s" % self.intf.GID())
        elif self.IsRspan():
            logger.info("- Interface : %s" % self.intf.GID())
            if self.segment.IsFabEncapVxlan():
                logger.info("- EncapType : VXLAN")
                logger.info("- EncapVal  : %s" % self.segment.vxlan_id)
            else:
                logger.info("- EncapType : VLAN")
                logger.info("- EncapVal  : %s" % self.segment.vlan_id)
        elif self.IsErspan():
            logger.info("- Dest IP   : %s" % self.ep.GetIpAddress().get())
            logger.info("- DSCP      : %s" % self.dscp)
        return

    def IsLocal(self):
        return self.type == 'LOCAL'
    def IsRspan(self):
        return self.type == 'RSPAN'
    def IsErspan(self):
        return self.type == 'ERSPAN'

    def Update(self, snaplen, spantype, intf):
        self.snaplen = snaplen
        self.type = spantype
        self.intf = intf
        objlist = []
        objlist.append(self)
        halapi.ConfigureSpanSessions(objlist)

    def Clear(self):
        self.type = 'INACTIVE'
        objlist = []
        objlist.append(self)
        halapi.DeleteSpanSessions(objlist)

    def PrepareHALRequestSpec(self, reqspec):
        if isinstance(reqspec, telemetry_pb2.MirrorSessionSpec):
            reqspec.vrf_key_handle.vrf_id = self.tenant.id
            reqspec.key_or_handle.mirrorsession_id = self.id
            reqspec.snaplen = self.snaplen
            if self.IsLocal():
                reqspec.local_span_if.if_handle = self.intf.hal_handle
            if self.IsRspan():
                reqspec.rspan_spec.intf.if_handle = self.intf.hal_handle
                if self.segment.IsFabEncapVxlan():
                    reqspec.rspan_spec.rspan_encap.encap_type = haldefs.common.ENCAP_TYPE_VXLAN
                    reqspec.rspan_spec.rspan_encap.encap_value = self.segment.vxlan_id
                else:
                    reqspec.rspan_spec.rspan_encap.encap_type = haldefs.common.ENCAP_TYPE_DOT1Q
                    reqspec.rspan_spec.rspan_encap.encap_value = self.segment.vlan_id
            if self.IsErspan():
                reqspec.erspan_spec.src_ip.ip_af = haldefs.common.IP_AF_INET
                reqspec.erspan_spec.src_ip.v4_addr = self.erspan_src.getnum()
                reqspec.erspan_spec.dest_ip.ip_af = haldefs.common.IP_AF_INET
                reqspec.erspan_spec.dest_ip.v4_addr = self.erspan_dest.getnum()
                reqspec.erspan_spec.type = telemetry_pb2.ERSpanType.ERSPAN_TYPE_3
                reqspec.erspan_spec.span_id = self.id
        else:
            reqspec.key_or_handle.mirrorsession_id = self.id

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("  - SpanSession %s = %s" %\
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
        logger.info("Configuring %d Span Sessions." % len(self.span_ssns))
        halapi.ConfigureSpanSessions(self.span_ssns)
        return

    def Generate(self, tenant, topospec):
        tnls = []
        for tnl in Store.objects.GetAllByClass(tunnel.TunnelObject):
            if tnl.IsGRE():
                tnls.append(tnl)
        tnlid = 0
        for entry in topospec.span_sessions:
            spec = entry.spec.Get(Store)
            span_ssn = SpanSessionObject()
            span_ssn.Init(tenant, spec)
            if span_ssn.IsErspan():
                tnl = tnls[tnlid % len(tnls)]
                tnlid = tnlid + 1
                span_ssn.erspan_dest = tnl.GetDestIp()
                span_ssn.erspan_src = tnl.GetSrcIp()
            self.span_ssns.append(span_ssn)
        Store.objects.SetAll(self.span_ssns)
        return

    def main(self, tenant, topospec):
        self.Generate(tenant, topospec)
        self.Configure()
        return
