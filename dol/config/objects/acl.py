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

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class AclObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return
        
    def Init(self, spec):
        self.Clone(spec)

        self.id = resmgr.AclIdAllocator.get()
        gid = "Acl%04d" % self.id
        self.GID(gid)
        self.spec = spec
        if self.action.ingress_mirror and self.action.ingress_mirror.enable:
            session_ref = self.action.ingress_mirror.session
            self.action.ingress_mirror.session = session_ref.Get(Store)
        if self.action.egress_mirror and self.action.egress_mirror.enable:
            session_ref = self.action.egress_mirror.session
            self.action.egress_mirror.session = session_ref.Get(Store)
            
        self.Show()
        return

    def Show(self):
        cfglogger.info("Acl  : %s" % self.GID())
        cfglogger.info("- Match  : %s" % self.GID())
        if self.match.eth:
            cfglogger.info("  Eth:")
            cfglogger.info("  - Ethertype : %d" % self.match.eth.ethertype.get())
            cfglogger.info("  - Src       : %s" % self.match.eth.src.get())
            cfglogger.info("  - Dst       : %s" % self.match.eth.dst.get())
        elif self.match.ip:
            cfglogger.info("  IP:")
            cfglogger.info("  - Src   : %s" % self.match.ip.src.get())
            cfglogger.info("  - Dst   : %s" % self.match.ip.dst.get())
            cfglogger.info("  - Proto : %d" % self.match.ip.proto.get())
            if self.match.l4 and self.match.l4.tcp:
                cfglogger.info("  TCP:")
                cfglogger.info("  - Syn       : %d" % self.match.l4.tcp.syn.get())
                cfglogger.info("  - Ack       : %d" % self.match.l4.tcp.ack.get())
                cfglogger.info("  - Fin       : %d" % self.match.l4.tcp.fin.get())
                cfglogger.info("  - Rst       : %d" % self.match.l4.tcp.rst.get())
                cfglogger.info("  - Urg       : %d" % self.match.l4.tcp.urg.get())
                cfglogger.info("  - Port-Range: %d-%d" %\
                               (self.match.l4.tcp.port_range.GetStart(),
                                self.match.l4.tcp.port_range.GetEnd()))
            elif self.match.l4 and self.match.l4.udp:
                cfglogger.info("  UDP:")
                cfglogger.info("  - Port-Range: %d-%d" %\
                               (self.match.l4.udp.port_range.GetStart(),
                                self.match.l4.udp.port_range.GetEnd()))
            elif self.match.l4 and self.match.l4.icmp:
                cfglogger.info("  ICMP:")
                cfglogger.info("  - Code: %d" % self.match.l4.icmp.code.get())
                cfglogger.info("  - Type: %d" % self.match.l4.icmp.type.get())
         
        cfglogger.info("- Action    : %s" % self.action.action)
        if self.action.ingress_mirror:
            cfglogger.info("  Ing. Mirror   : En=%s, Session=%s" %\
                           (self.action.ingress_mirror.enable,
                            self.action.ingress_mirror.session.GID()))
        if self.action.egress_mirror:
            cfglogger.info("  Egr. Mirror   : En=%s, Session=%s" %\
                           (self.action.egress_mirror.enable,
                            self.action.egress_mirror.session.GID()))
        return

    def PrepareHALRequestSpec(self, reqspec):
        reqspec.key_or_handle.acl_id = self.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - Acl %s = %s" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Acl Objects.
class AclObjectHelper:
    def __init__(self):
        self.acls = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Acls." % len(self.acls)) 
        halapi.ConfigureAcls(self.acls)
        return
        
    def Generate(self, topospec):
        spec = topospec.acls.Get(Store)
        cfglogger.info("Creating Acls")
        for e in spec.entries:
            acl = AclObject()
            acl.Init(e.entry)
            self.acls.append(acl)
        Store.objects.SetAll(self.acls)
        return

    def main(self, topospec):
        self.Generate(topospec)
        #self.Configure()
        return

AclHelper = AclObjectHelper()
