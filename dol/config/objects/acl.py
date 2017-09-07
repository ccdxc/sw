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

        if self.MatchOnIPv4():
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

        elif self.MatchOnIPv6():
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
        if self.MatchOnSIF():
            if self.fields.match.src_if:
                cfglogger.info("  Src IF     : %s" % self.fields.match.src_if.GID())
            else:
                cfglogger.info("  Src IF Not set")

        if self.MatchOnDIF():
            if self.fields.match.dst_if:
                cfglogger.info("  Dst IF     : %s" % self.fields.match.dst_if.GID())
            else:
                cfglogger.info("  Dst IF Not set")

        if self.MatchOnSegment():
            if self.fields.match.segment:
                cfglogger.info("  Segment     : %s" % self.fields.match.segment.GID())
            else:
                cfglogger.info("  Segment Not set")

        if self.MatchOnEth():
            cfglogger.info("  Eth:")
            cfglogger.info("  - Ethertype : %d" % self.fields.match.eth.ethertype.get())
            cfglogger.info("  - Src       : %s" % self.fields.match.eth.src.get())
            cfglogger.info("  - Dst       : %s" % self.fields.match.eth.dst.get())
        elif self.MatchOnIP():
            cfglogger.info("  IP:")
            cfglogger.info("  - Src   : %s/%d" %\
                            (self.fields.match.ip.src_ip.get(),
                             self.fields.match.ip.src_prefix_len.get()))
            cfglogger.info("  - Dst   : %s/%d" %\
                            (self.fields.match.ip.dst_ip.get(),
                             self.fields.match.ip.dst_prefix_len.get()))
        if self.MatchOnL4Proto():
            cfglogger.info("  - Proto : %d" % self.fields.match.l4.proto.get())
        if self.MatchOnTCP():
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
        elif self.MatchOnUDP():
            cfglogger.info("  UDP:")
            cfglogger.info("  - Src Port-Range  : %d-%d" %\
                           (self.fields.match.l4.udp.src_port_range.GetStart(),
                            self.fields.match.l4.udp.src_port_range.GetEnd()))
            cfglogger.info("  - Dst Port-Range  : %d-%d" %\
                           (self.fields.match.l4.udp.dst_port_range.GetStart(),
                            self.fields.match.l4.udp.dst_port_range.GetEnd()))
        elif self.MatchOnICMP():
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

    def Configure(self):
        cfglogger.info("Configuring Acl  : %s (id: %d)" % (self.GID(), self.id))
        halapi.ConfigureAcls([self])
        return

    def Delete(self):
        cfglogger.info("Deleting Acl  : %s (id: %d)" % (self.GID(), self.id))
        halapi.DeleteAcls([self])
        return

    def __getEnumValue(self, val):
        valstr = "ACL_ACTION_" + val.upper()
        return haldefs.acl.AclAction.Value(valstr)

    def PrepareHALRequestSpec(self, reqspec):
        reqspec.key_or_handle.acl_id = self.id
        reqspec.priority = self.fields.priority.get()
        if self.MatchOnSIF():
            reqspec.match.src_if_key_handle.if_handle =\
                    self.fields.match.src_if.hal_handle

        if self.MatchOnDIF():
            reqspec.match.dst_if_key_handle.if_handle =\
                    self.fields.match.dst_if.hal_handle

        if self.MatchOnSegment():
            reqspec.match.l2segment_key_handle.l2segment_handle=\
                    self.fields.match.segment.hal_handle

        if self.MatchOnEth():
            reqspec.match.eth_selector.eth_type = self.fields.match.eth.ethertype.get()
            reqspec.match.eth_selector.eth_type_mask = self.fields.match.eth.ethertype_mask.get()
            reqspec.match.eth_selector.src_mac = self.fields.match.eth.src.getnum()
            reqspec.match.eth_selector.src_mac_mask = self.fields.match.eth.src_mask.getnum()
            reqspec.match.eth_selector.dst_mac = self.fields.match.eth.dst.getnum()
            reqspec.match.eth_selector.dst_mac_mask = self.fields.match.eth.dst_mask.getnum()
        elif self.MatchOnIP():
            reqspec.match.ip_selector.ip_af = haldefs.common.IP_AF_NONE
            if self.MatchOnIPv4():
                reqspec.match.ip_selector.ip_af = haldefs.common.IP_AF_INET
                reqspec.match.ip_selector.src_prefix.address.ip_af = haldefs.common.IP_AF_INET
                reqspec.match.ip_selector.src_prefix.address.v4_addr = self.fields.match.ip.src_ip.getnum()
                reqspec.match.ip_selector.dst_prefix.address.ip_af = haldefs.common.IP_AF_INET
                reqspec.match.ip_selector.dst_prefix.address.v4_addr = self.fields.match.ip.dst_ip.getnum()
            elif self.MatchOnIPv6():
                reqspec.match.ip_selector.ip_af = haldefs.common.IP_AF_INET6
                reqspec.match.ip_selector.src_prefix.address.ip_af = haldefs.common.IP_AF_INET6
                reqspec.match.ip_selector.src_prefix.address.v6_addr = self.fields.match.ip.src_ip.getnum().to_bytes(16, 'big')
                reqspec.match.ip_selector.dst_prefix.address.ip_af = haldefs.common.IP_AF_INET6
                reqspec.match.ip_selector.dst_prefix.address.v6_addr = self.fields.match.ip.dst_ip.getnum().to_bytes(16, 'big')
            reqspec.match.ip_selector.src_prefix.prefix_len = self.fields.match.ip.src_prefix_len.get()
            reqspec.match.ip_selector.dst_prefix.prefix_len = self.fields.match.ip.dst_prefix_len.get()

        if self.MatchOnL4Proto():
            reqspec.match.ip_selector.ip_protocol = self.fields.match.l4.proto.get()
        if self.MatchOnTCP():
            reqspec.match.ip_selector.tcp_selector.src_port_range.port_low = \
                    self.fields.match.l4.tcp.src_port_range.GetStart()
            reqspec.match.ip_selector.tcp_selector.src_port_range.port_high = \
                    self.fields.match.l4.tcp.src_port_range.GetEnd()
            reqspec.match.ip_selector.tcp_selector.dst_port_range.port_low = \
                    self.fields.match.l4.tcp.dst_port_range.GetStart()
            reqspec.match.ip_selector.tcp_selector.dst_port_range.port_high = \
                    self.fields.match.l4.tcp.dst_port_range.GetEnd()
        elif self.MatchOnUDP():
            reqspec.match.ip_selector.udp_selector.src_port_range.port_low = \
                    self.fields.match.l4.udp.src_port_range.GetStart()
            reqspec.match.ip_selector.udp_selector.src_port_range.port_high = \
                    self.fields.match.l4.udp.src_port_range.GetEnd()
            reqspec.match.ip_selector.udp_selector.dst_port_range.port_low = \
                    self.fields.match.l4.udp.dst_port_range.GetStart()
            reqspec.match.ip_selector.udp_selector.dst_port_range.port_high = \
                    self.fields.match.l4.udp.dst_port_range.GetEnd()
        elif self.MatchOnICMP():
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
        self.hal_handle = resp_spec.status.acl_handle.handle
        cfglogger.info("  - Acl %s = %s handle: 0x%x" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status), 
                        self.hal_handle))
        return

    def PrepareHALDeleteRequestSpec(self, reqspec):
        reqspec.key_or_handle.acl_id = self.id
        return

    def ProcessHALDeleteResponse(self, req_spec, resp_spec):
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def MatchOnSIF(self):
        return self.fields.match.src_if_match

    def UpdateSIF(self, sif):
        if self.MatchOnSIF():
            self.fields.match.src_if = sif


    def MatchOnDIF(self):
        return self.fields.match.dst_if_match

    def UpdateDIF(self, dif):
        if self.MatchOnDIF():
            self.fields.match.dst_if = dif

    def MatchOnSegment(self):
        return self.fields.match.segment_match

    def UpdateSegment(self, segment):
        if self.MatchOnSegment():
            self.fields.match.segment = segment

    def MatchOnEth(self):
        return self.fields.match.type == 'eth'

    def MatchOnIP(self):
        return self.fields.match.type == 'ip'

    def MatchOnIPv4(self):
        return self.MatchOnIP() and self.fields.match.ip.type == 'v4'

    def MatchOnIPv6(self):
        return self.MatchOnIP() and self.fields.match.ip.type == 'v6'

    def MatchOnL4Proto(self):
        return self.MatchOnIP() and self.fields.match.l4.type == 'proto'

    def MatchOnTCP(self):
        return self.MatchOnIP() and self.fields.match.l4.type == 'tcp'

    def MatchOnUDP(self):
        return self.MatchOnIP() and self.fields.match.l4.type == 'udp'

    def MatchOnICMP(self):
        return self.MatchOnIP() and self.fields.match.l4.type == 'icmp'

    def MatchOnMacSA(self):
        if self.MatchOnEth() and self.fields.match.eth.src_mask.getnum():
            return True
        return False

    def MatchMacSA(self):
        if self.MatchOnEth() and self.fields.match.eth.src_mask.getnum():
            return self.fields.match.eth.src.get()
        return None 

    def MatchOnMacDA(self):
        if self.MatchOnEth() and self.fields.match.eth.dst_mask.getnum():
            return True
        return False

    def MatchMacDA(self):
        if self.MatchOnEth() and self.fields.match.eth.dst_mask.getnum():
            return self.fields.match.eth.dst.get()
        return None 

    def MatchOnEtherType(self):
        if self.MatchOnEth():
            return True
        return False

    def MatchEtherType(self):
        if self.MatchOnEth():
            return self.fields.match.eth.ethertype.get()
        return None

    def MatchOnIPv4SIP(self):
        if self.MatchOnIPv4() and self.fields.match.ip.src_prefix_len.get():
            return True
        return False

    def MatchIPv4SIP(self):
        if self.MatchOnIPv4() and self.fields.match.ip.src_prefix_len.get():
            return self.fields.match.ip.src_ip.get()
        return None

    def MatchOnIPv4DIP(self):
        if self.MatchOnIPv4() and self.fields.match.ip.dst_prefix_len.get():
            return True
        return False

    def MatchIPv4DIP(self):
        if self.MatchOnIPv4() and self.fields.match.ip.dst_prefix_len.get():
            return self.fields.match.ip.dst_ip.get()
        return None 

    def MatchOnIPv6SIP(self):
        if self.MatchOnIPv6() and self.fields.match.ip.src_prefix_len.get():
            return True
        return False

    def MatchIPv6SIP(self):
        if self.MatchOnIPv6() and self.fields.match.ip.src_prefix_len.get():
            return self.fields.match.ip.src_ip.get()
        return None

    def MatchOnIPv6DIP(self):
        if self.MatchOnIPv6() and self.fields.match.ip.dst_prefix_len.get():
            return True
        return False

    def MatchIPv6DIP(self):
        if self.MatchOnIPv6() and self.fields.match.ip.dst_prefix_len.get():
            return self.fields.match.ip.dst_ip.get()
        return None 

    def MatchOnProto(self):
        return self.MatchOnL4Proto()

    def MatchProto(self):
        if self.MatchOnL4Proto():
            return self.fields.match.l4.proto.get()

        return None

    def MatchOnUDPSport(self):
        if self.MatchOnUDP():
            return True
        return False

    def MatchUDPSport(self):
        if self.MatchOnUDP():
            return self.fields.match.l4.udp.src_port_range.get() 
        return None 

    def MatchOnUDPDport(self):
        if self.MatchOnUDP():
            return True
        return False

    def MatchUDPDport(self):
        if self.MatchOnUDP():
            return self.fields.match.l4.udp.dst_port_range.get() 
        return None 

    def MatchOnTCPSport(self):
        if self.MatchOnTCP():
            return True
        return False

    def MatchTCPSport(self):
        if self.MatchOnTCP():
            return self.fields.match.l4.tcp.src_port_range.get() 
        return None 

    def MatchOnTCPDport(self):
        if self.MatchOnTCP():
            return True
        return False

    def MatchTCPDport(self):
        if self.MatchOnTCP():
            return self.fields.match.l4.tcp.dst_port_range.get() 
        return None 

    def MatchOnICMPCode(self):
        if self.MatchOnICMP():
            return True
        return False

    def MatchICMPCode(self):
        if self.MatchOnICMP():
            return self.fields.match.l4.icmp.code.get()
        return None

    def MatchOnICMPType(self):
        if self.MatchOnICMP():
            return True
        return False

    def MatchICMPType(self):
        if self.MatchOnICMP():
            return self.fields.match.l4.icmp.type.get()
        return None

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
        return

AclHelper = AclObjectHelper()
