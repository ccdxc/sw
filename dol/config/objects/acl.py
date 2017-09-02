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
        self.Clone(Store.templates.Get('ACL'))
        self.id = resmgr.AclIdAllocator.get()
        return
        
    def Init(self, spec):
        self.fields = objects.MergeObjects(spec, self.fields)

        self.GID(spec.id)
#        self.spec = spec
        if self.IsIPv4():
            if self.fields.match.ip.alloc_src_ip:
                self.fields.match.ip.src_ip = resmgr.AclIPv4Allocator.get()
                self.fields.match.ip.src_prefix_len = objects.TemplateFieldObject("const/32")
            elif self.fields.match.ip.alloc_src_prefix:
                self.fields.match.ip.src_ip = resmgr.AclIPv4SubnetAllocator.get()

            if self.fields.match.ip.alloc_dst_ip:
                self.fields.match.ip.dst_ip = resmgr.AclIPv4Allocator.get()
                self.fields.match.ip.dst_prefix_len = objects.TemplateFieldObject("const/32")
            elif self.fields.match.ip.alloc_dst_prefix:
                self.fields.match.ip.dst_ip = resmgr.AclIPv4SubnetAllocator.get()

        elif self.IsIPv6():
            if self.fields.match.ip.alloc_src_ip:
                self.fields.match.ip.src_ip = resmgr.AclIPv6Allocator.get()
                self.fields.match.ip.src_prefix_len = objects.TemplateFieldObject("const/128")
            elif self.fields.match.ip.alloc_src_prefix:
                self.fields.match.ip.src_ip = resmgr.AclIPv6SubnetAllocator.get()
            else:
                self.fields.match.ip.src_ip = objects.TemplateFieldObject("ipv6addr/0::0")

            if self.fields.match.ip.alloc_dst_ip:
                self.fields.match.ip.dst_ip = resmgr.AclIPv6Allocator.get()
                self.fields.match.ip.dst_prefix_len = objects.TemplateFieldObject("const/128")
            elif self.fields.match.ip.alloc_dst_prefix:
                self.fields.match.ip.dst_ip = resmgr.AclIPv6SubnetAllocator.get()
            else:
                self.fields.match.ip.dst_ip = objects.TemplateFieldObject("ipv6addr/0::0")

        if self.fields.action.ingress_mirror and self.fields.action.ingress_mirror.enable:
            session_ref = self.fields.action.ingress_mirror.session
            self.fields.action.ingress_mirror.session = session_ref.Get(Store)
        if self.fields.action.egress_mirror and self.fields.action.egress_mirror.enable:
            session_ref = self.fields.action.egress_mirror.session
            self.fields.action.egress_mirror.session = session_ref.Get(Store)
            
        self.Show()
        return

    def Show(self):
        cfglogger.info("Acl  : %s (id: %d)" % (self.GID(), self.id))
        cfglogger.info("- Match  : %s" % self.GID())
        if self.IsEth():
            cfglogger.info("  Eth:")
            cfglogger.info("  - Ethertype : %d" % self.fields.match.eth.ethertype.get())
            cfglogger.info("  - Src       : %s" % self.fields.match.eth.src.get())
            cfglogger.info("  - Dst       : %s" % self.fields.match.eth.dst.get())
        elif self.IsIP():
            cfglogger.info("  IP:")
            cfglogger.info("  - Src   : %s/%d" %\
                            (self.fields.match.ip.src_ip.get(),
                             self.fields.match.ip.src_prefix_len.get()))
            cfglogger.info("  - Dst   : %s/%d" %\
                            (self.fields.match.ip.dst_ip.get(),
                             self.fields.match.ip.dst_prefix_len.get()))
        if self.IsL4Proto():
            cfglogger.info("  - Proto : %d" % self.fields.match.l4.proto.get())
        if self.IsTCP():
            cfglogger.info("  TCP:")
#            cfglogger.info("  - Syn             : %d" % self.fields.match.l4.tcp.syn.get())
#            cfglogger.info("  - Ack             : %d" % self.fields.match.l4.tcp.ack.get())
#            cfglogger.info("  - Fin             : %d" % self.fields.match.l4.tcp.fin.get())
#            cfglogger.info("  - Rst             : %d" % self.fields.match.l4.tcp.rst.get())
#            cfglogger.info("  - Urg             : %d" % self.fields.match.l4.tcp.urg.get())
            cfglogger.info("  - Src Port-Range  : %d-%d" %\
                           (self.fields.match.l4.tcp.src_port_range.GetStart(),
                            self.fields.match.l4.tcp.src_port_range.GetEnd()))
            cfglogger.info("  - Dst Port-Range  : %d-%d" %\
                           (self.fields.match.l4.tcp.dst_port_range.GetStart(),
                            self.fields.match.l4.tcp.dst_port_range.GetEnd()))
        elif self.IsUDP():
            cfglogger.info("  UDP:")
            cfglogger.info("  - Src Port-Range  : %d-%d" %\
                           (self.fields.match.l4.udp.src_port_range.GetStart(),
                            self.fields.match.l4.udp.src_port_range.GetEnd()))
            cfglogger.info("  - Dst Port-Range  : %d-%d" %\
                           (self.fields.match.l4.udp.dst_port_range.GetStart(),
                            self.fields.match.l4.udp.dst_port_range.GetEnd()))
        elif self.IsICMP():
            cfglogger.info("  ICMP:")
            cfglogger.info("  - Code: %d" % self.fields.match.l4.icmp.code.get())
            cfglogger.info("  - Type: %d" % self.fields.match.l4.icmp.type.get())
         
        cfglogger.info("- Action    : %s" % self.fields.action.action)
        if self.fields.action.ingress_mirror:
            cfglogger.info("  Ing. Mirror   : En=%s, Session=%s" %\
                           (self.fields.action.ingress_mirror.enable,
                            self.fields.action.ingress_mirror.session.GID()))
        if self.fields.action.egress_mirror:
            cfglogger.info("  Egr. Mirror   : En=%s, Session=%s" %\
                           (self.fields.action.egress_mirror.enable,
                            self.fields.action.egress_mirror.session.GID()))
        return

    def IsEth(self):
        return self.fields.match.type == 'eth'

    def IsIP(self):
        return self.fields.match.type == 'ip'

    def IsIPv4(self):
        return self.IsIP() and self.fields.match.ip.type == 'v4'

    def IsIPv6(self):
        return self.IsIP() and self.fields.match.ip.type == 'v6'

    def IsL4Proto(self):
        return self.IsIP() and self.fields.match.l4.type == 'proto'

    def IsTCP(self):
        return self.IsIP() and self.fields.match.l4.type == 'tcp'

    def IsUDP(self):
        return self.IsIP() and self.fields.match.l4.type == 'udp'

    def IsICMP(self):
        return self.IsIP() and self.fields.match.l4.type == 'icmp'

    def __getEnumValue(self, val):
        valstr = "ACL_ACTION_" + val.upper()
        return haldefs.acl.AclAction.Value(valstr)

    def PrepareHALRequestSpec(self, reqspec):
        reqspec.key_or_handle.acl_id = self.id
        reqspec.priority = self.fields.priority.get()
        if self.IsEth():
            reqspec.match.eth_selector.eth_type = self.fields.match.eth.ethertype.get()
            reqspec.match.eth_selector.eth_type_mask = self.fields.match.eth.ethertype_mask.get()
            reqspec.match.eth_selector.src_mac = self.fields.match.eth.src.getnum()
            reqspec.match.eth_selector.src_mac_mask = self.fields.match.eth.src_mask.getnum()
            reqspec.match.eth_selector.dst_mac = self.fields.match.eth.dst.getnum()
            reqspec.match.eth_selector.dst_mac_mask = self.fields.match.eth.dst_mask.getnum()
        elif self.IsIP():
            reqspec.match.ip_selector.ip_af = haldefs.common.IP_AF_NONE
            if self.IsIPv4():
                reqspec.match.ip_selector.ip_af = haldefs.common.IP_AF_INET
                reqspec.match.ip_selector.src_prefix.address.ip_af = haldefs.common.IP_AF_INET
                reqspec.match.ip_selector.src_prefix.address.v4_addr = self.fields.match.ip.src_ip.getnum()
                reqspec.match.ip_selector.dst_prefix.address.ip_af = haldefs.common.IP_AF_INET
                reqspec.match.ip_selector.dst_prefix.address.v4_addr = self.fields.match.ip.dst_ip.getnum()
            elif self.IsIPv6():
                reqspec.match.ip_selector.ip_af = haldefs.common.IP_AF_INET6
                reqspec.match.ip_selector.src_prefix.address.ip_af = haldefs.common.IP_AF_INET6
                reqspec.match.ip_selector.src_prefix.address.v6_addr = self.fields.match.ip.src_ip.getnum().to_bytes(16, 'big')
                reqspec.match.ip_selector.dst_prefix.address.ip_af = haldefs.common.IP_AF_INET6
                reqspec.match.ip_selector.dst_prefix.address.v6_addr = self.fields.match.ip.dst_ip.getnum().to_bytes(16, 'big')
            reqspec.match.ip_selector.src_prefix.prefix_len = self.fields.match.ip.src_prefix_len.get()
            reqspec.match.ip_selector.dst_prefix.prefix_len = self.fields.match.ip.dst_prefix_len.get()

        if self.IsL4Proto():
            reqspec.match.ip_selector.ip_protocol = self.fields.match.l4.proto.get()
        if self.IsTCP():
            reqspec.match.ip_selector.tcp_selector.src_port_range.port_low = \
                    self.fields.match.l4.tcp.src_port_range.GetStart()
            reqspec.match.ip_selector.tcp_selector.src_port_range.port_high = \
                    self.fields.match.l4.tcp.src_port_range.GetEnd()
            reqspec.match.ip_selector.tcp_selector.dst_port_range.port_low = \
                    self.fields.match.l4.tcp.dst_port_range.GetStart()
            reqspec.match.ip_selector.tcp_selector.dst_port_range.port_high = \
                    self.fields.match.l4.tcp.dst_port_range.GetEnd()
        elif self.IsUDP():
            reqspec.match.ip_selector.udp_selector.src_port_range.port_low = \
                    self.fields.match.l4.udp.src_port_range.GetStart()
            reqspec.match.ip_selector.udp_selector.src_port_range.port_high = \
                    self.fields.match.l4.udp.src_port_range.GetEnd()
            reqspec.match.ip_selector.udp_selector.dst_port_range.port_low = \
                    self.fields.match.l4.udp.dst_port_range.GetStart()
            reqspec.match.ip_selector.udp_selector.dst_port_range.port_high = \
                    self.fields.match.l4.udp.dst_port_range.GetEnd()
        elif self.IsICMP():
            reqspec.match.ip_selector.icmp_selector.icmp_code = \
                    self.fields.match.l4.icmp.code.get()
            reqspec.match.ip_selector.icmp_selector.icmp_code_mask = \
                    self.fields.match.l4.icmp.code_mask.get()
            reqspec.match.ip_selector.icmp_selector.icmp_type = \
                    self.fields.match.l4.icmp.type.get()
            reqspec.match.ip_selector.icmp_selector.icmp_type_mask = \
                    self.fields.match.l4.icmp.type_mask.get()
    
        reqspec.action.action = self.__getEnumValue(self.fields.action.action)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.acl_handle
        cfglogger.info("  - Acl %s = %s" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def MatchOnMacSA(self):
        if self.IsEth() and self.fields.match.eth.src_mask.getnum():
            return True
        return False

    def MatchMacSA(self):
        if self.IsEth() and self.fields.match.eth.src_mask.getnum():
            return self.fields.match.eth.src.get()
        return None 

    def MatchOnMacDA(self):
        if self.IsEth() and self.fields.match.eth.dst_mask.getnum():
            return True
        return False

    def MatchMacDA(self):
        if self.IsEth() and self.fields.match.eth.dst_mask.getnum():
            return self.fields.match.eth.dst.get()
        return None 

    def MatchOnEtherType(self):
        if self.IsEth():
            return True
        return False

    def MatchEtherType(self):
        if self.IsEth():
            return self.fields.match.eth.ethertype.get()
        return None

    def MatchOnVlan(self):
        return False

    def MatchVlan(self):
        return None

    def MatchOnIPv4SIP(self):
        if self.IsIPv4() and self.fields.match.ip.src_prefix_len:
            return True
        return False

    def MatchIPv4SIP(self):
        if self.IsIPv4() and self.fields.match.ip.src_prefix_len:
            return self.fields.match.ip.src_ip.get()
        return None

    def MatchOnIPv4DIP(self):
        if self.IsIPv4() and self.fields.match.ip.dst_prefix_len:
            return True
        return False

    def MatchIPv4DIP(self):
        if self.IsIPv4() and self.fields.match.ip.dst_prefix_len:
            return self.fields.match.ip.dst_ip.get()
        return None 

    def MatchOnIPv6SIP(self):
        if self.IsIPv6() and self.fields.match.ip.src_prefix_len:
            return True
        return False

    def MatchIPv6SIP(self):
        if self.IsIPv6() and self.fields.match.ip.src_prefix_len:
            return self.fields.match.ip.src_ip.get()
        return None

    def MatchOnIPv6DIP(self):
        if self.IsIPv6() and self.fields.match.ip.dst_prefix_len:
            return True
        return False

    def MatchIPv6DIP(self):
        if self.IsIPv6() and self.fields.match.ip.dst_prefix_len:
            return self.fields.match.ip.dst_ip.get()
        return None 

    def MatchOnUDPSport(self):
        if self.IsUDP():
            return True
        return False

    def MatchUDPSport(self):
        if self.IsUDP():
            return self.fields.match.l4.udp.src_port_range.get() 
        return None 

    def MatchOnUDPDport(self):
        if self.IsUDP():
            return True
        return False

    def MatchUDPDport(self):
        if self.IsUDP():
            return self.fields.match.l4.udp.dst_port_range.get() 
        return None 

    def MatchOnTCPSport(self):
        if self.IsTCP():
            return True
        return False

    def MatchTCPSport(self):
        if self.IsTCP():
            return self.fields.match.l4.tcp.src_port_range.get() 
        return None 

    def MatchOnTCPDport(self):
        if self.IsTCP():
            return True
        return False

    def MatchTCPDport(self):
        if self.IsTCP():
            return self.fields.match.l4.tcp.dst_port_range.get() 
        return None 

    def MatchOnICMPCode(self):
        if self.IsICMP():
            return True
        return False

    def MatchICMPCode(self):
        if self.IsICMP():
            return self.fields.match.l4.icmp.code
        return None

    def MatchOnICMPType(self):
        if self.IsICMP():
            return True
        return False

    def MatchICMPType(self):
        if self.IsICMP():
            return self.fields.match.l4.icmp.type
        return None

    def IsFlowMatch(self, flow):
        return False

    def GetAction(self):
        return self.fields.action.action.upper()

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
        if topospec.acls == None: return
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
        self.Configure()
        return

    def GetMatchingAclAction(self, flow):
        # Go over all the acls according to priority and return the action of
        # the matching acl
        for acl in self.acls:
            if acl.IsFlowMatch(flow):
                return acl.GetAction()
        return 'PERMIT'

AclHelper = AclObjectHelper()
