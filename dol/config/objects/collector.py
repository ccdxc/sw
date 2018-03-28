# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment
import config.objects.lif       as lif
import config.objects.tunnel    as tunnel
#import config.objects.session   as session

from config.store               import Store
from infra.common.logging       import logger

import config.hal.defs          as haldefs
import config.hal.api           as halapi
import telemetry_pb2            as telemetry_pb2

class CollectorObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('COLLECTOR'))
        return

    def Init(self, tenant, spec):
        self.tenant = tenant
        self.id = resmgr.CollectorIdAllocator.get()
        self.GID("Collector%04d" % self.id)
        self.spec = spec
        self.vlan = 0
        self.l2seg_handle = None
        self.dest_ip = None
        self.source_ip = None
        self.protocol = None
        self.dport = 0
        self.format = None
        self.template_id = 0
        self.src_ep = None
        self.dst_ep = None
        self.sess_spec = None
        return

    def Show(self):
        logger.info("Collector       : %s" % self.GID())
        logger.info("- Encap         : %s" % self.vlan)
        logger.info("- Destination   : %s" % self.dest_ip)
        logger.info("- Source        : %s" % self.source_ip)
        logger.info("- Protocol      : %s" % self.protocol)
        logger.info("- Port          : %s" % self.dport)
        logger.info("- Format        : %s" % self.format)
        logger.info("- Template ID   : %s" % self.template_id)
        return

    def Update(self, encap, dest, src, protocol, dport, format, template_id):
        self.vlan = encap
        self.dest_ip = dest
        self.source_ip = src
        self.protocol = protocol
        self.dport = dport
        self.format = format
        self.template_id = template_id
        objlist = []
        objlist.append(self)
        halapi.ConfigureCollectors(objlist)

    def PrepareHALRequestSpec(self, reqspec):
        #pdb.set_trace()
        reqspec.meta.vrf_id = self.tenant.id
        reqspec.export_controlId.Id = self.id
        reqspec.encap.encap_type = haldefs.common.ENCAP_TYPE_DOT1Q
        reqspec.encap.encap_value = self.vlan
        reqspec.l2seg_handle = self.l2seg_handle
        reqspec.dest_ip.ip_af = haldefs.common.IP_AF_INET
        reqspec.dest_ip.v4_addr = self.dest_ip.getnum()
        reqspec.src_ip.ip_af = haldefs.common.IP_AF_INET
        reqspec.src_ip.v4_addr = self.source_ip.getnum()
        reqspec.protocol = self.protocol
        reqspec.dest_port.port = self.dport
        reqspec.format = self.format
        reqspec.template_id = self.template_id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("  - Collector %s = %s" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        #pdb.set_trace()
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Tenant Objects.
class CollectorObjectHelper:
    def __init__(self):
        self.collectors = []
        return

    def Configure(self):
        logger.info("Configuring %d Collector." % len(self.collectors))
        halapi.ConfigureCollectors(self.collectors)
        return

    def Generate(self, tenant, topospec):
        count = 0
        eps = tenant.GetRemoteEps()
        leps = tenant.GetLocalEps()
        for entry in topospec.collectors:
            if count >= len(eps):
                break
            ep = eps[count]
            count = count + 1
            spec = entry.spec.Get(Store)
            collector = CollectorObject()
            collector.Init(tenant, spec)
            collector.vlan = ep.segment.vlan_id
            collector.l2seg_handle = ep.segment.hal_handle
            collector.dest_ip = ep.ipaddrs[0]
            collector.source_ip = leps[0].ipaddrs[0]
            collector.protocol = defs.ipprotos.id("UDP")
            collector.dport = 4739
            collector.format = telemetry_pb2.ExportFormat.Value("IPFIX")
            collector.template_id = 1
            # add the sessions.
            collector.sess_spec =  spec.session
            collector.dst_ep = ep
            collector.src_ep = leps[0]
            self.collectors.append(collector)
            collector.Show()
        Store.objects.SetAll(self.collectors)
        return

    def GetCollectorSessions(self):
        ret = []
        for c in self.collectors:
            ret.append((c.src_ep, c.dst_ep, c.sess_spec))
        return ret

    def main(self, tenant, topospec):
        self.Generate(tenant, topospec)
        self.Configure()
        return

CollectorHelper = CollectorObjectHelper()
