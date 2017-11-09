# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.tenant    as tenant
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

        if self.MatchOnFlowMiss():
            self.fields.config_flow_miss = True

        self.ing_mirror_sessions = []
        self.egr_mirror_sessions = []
        for s in self.fields.action.mirror.ingress:
            ispan   = s.Get(Store)
            self.ing_mirror_sessions.append(ispan)
        for s in self.fields.action.mirror.egress:
            espan   = s.Get(Store)
            self.egr_mirror_sessions.append(espan)
            
        if self.ActionSupRedirect():
            self.fields.action.redirect_if = Store.objects.Get('Cpu1')
        #self.Show()
        return

    def Show(self):
        cfglogger.info("Acl  : %s (id: %d)" % (self.GID(), self.id))
        cfglogger.info("- Match  : %s" % self.GID())
        if self.MatchOnFlowMiss():
            cfglogger.info("  Flow Miss" )

        if self.MatchOnDirection():
            cfglogger.info("  Direction     : %s" % self.fields.match.direction)

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

        if self.MatchOnTenant():
            if self.fields.match.tenant:
                cfglogger.info("  Tenant     : %s" % self.fields.match.tenant.GID())
            else:
                cfglogger.info("  Tenant Not set")

        if len(self.fields.match.dropmask):
            cfglogger.info("  DropMask: " )
        for dm in self.fields.match.dropmask:
            cfglogger.info("    - %s" % dm)

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
            cfglogger.info("  - Options : ", self.fields.match.ip.options)
        if self.MatchOnL4Proto():
            cfglogger.info("  - Proto : %d" % self.fields.match.l4.proto.get())
        if self.MatchOnTCP():
            cfglogger.info("  TCP:")
            if self.MatchOnTCPFlags(['syn']):
                cfglogger.info("  - Syn             : %d" % self.fields.match.l4.tcp.syn)
            if self.MatchOnTCPFlags(['ack']):
                cfglogger.info("  - Ack             : %d" % self.fields.match.l4.tcp.ack)
            if self.MatchOnTCPFlags(['fin']):
                cfglogger.info("  - Fin             : %d" % self.fields.match.l4.tcp.fin)
            if self.MatchOnTCPFlags(['rst']):
                cfglogger.info("  - Rst             : %d" % self.fields.match.l4.tcp.rst)
            if self.MatchOnTCPFlags(['urg']):
                cfglogger.info("  - Urg             : %d" % self.fields.match.l4.tcp.urg)
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
            cfglogger.info("  - Code/Mask: %02x/%02x" % \
                    (self.fields.match.l4.icmp.code.get(), self.fields.match.l4.icmp.code_mask.get()))
            cfglogger.info("  - Type/Mask: %02x/%02x" % \
                    (self.fields.match.l4.icmp.type.get(), self.fields.match.l4.icmp.type_mask.get()))

        cfglogger.info("- Action    : %s" % self.fields.action.action)
        if self.ActionUplinkRedirect():
            cfglogger.info("  Uplink redirect : %s" % self.fields.action.redirect_if.GID())
        elif self.ActionTunnelRedirect():
            cfglogger.info("  Tunnel redirect : %s" % self.fields.action.redirect_if.GID())
        elif self.ActionSupRedirect():
            cfglogger.info("  Sup redirect" )

        for ssn in self.ing_mirror_sessions:
            cfglogger.info("  Ing. Mirror   : Session=%s" % ssn.GID())
        for ssn in self.egr_mirror_sessions:
            cfglogger.info("  Egr. Mirror   : Session=%s" % ssn.GID())
        return

    def Configure(self):
        cfglogger.info("Configuring Acl  : %s (id: %d)" % (self.GID(), self.id))
        if self.GID() == 'ACL_TEP_MISS_ACTION_DROP':
            # TEP miss is handled in input_mapping table. Skip adding ACL
            return
        self.Show()
        halapi.ConfigureAcls([self])
        return

    def Delete(self):
        cfglogger.info("Deleting Acl  : %s (id: %d)" % (self.GID(), self.id))
        if self.GID() == 'ACL_TEP_MISS_ACTION_DROP':
            # TEP miss is handled in input_mapping table. Skip deleting ACL
            return
        halapi.DeleteAcls([self])
        return

    def __getEnumValue(self, val):
        valstr = "ACL_ACTION_" + val.upper()
        return haldefs.acl.AclAction.Value(valstr)

    def __getDirection(self, val):
        vals = {'from_enic':0, 'from_uplink':1}
        return vals[val]

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
            reqspec.match.l2segment_key_handle.l2segment_handle =\
                    self.fields.match.segment.hal_handle

        if self.MatchOnTenant():
            reqspec.match.tenant_key_handle.tenant_id =\
                    self.fields.match.tenant.id

        for dm in self.fields.match.dropmask:
            en = haldefs.acl.DropReason.Value(dm.upper() + "__DROP")
            reqspec.match.internal_key.drop_reason.append(en)
            reqspec.match.internal_mask.drop_reason.append(en)

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
            if self.MatchOnTCPFlags(['syn']):
                if self.fields.match.l4.tcp.syn:
                    reqspec.match.ip_selector.tcp_selector.tcp_syn_set = True
                else:
                    reqspec.match.ip_selector.tcp_selector.tcp_syn_clear = True

            if self.MatchOnTCPFlags(['ack']):
                if self.fields.match.l4.tcp.ack:
                    reqspec.match.ip_selector.tcp_selector.tcp_ack_set = True
                else:
                    reqspec.match.ip_selector.tcp_selector.tcp_ack_clear = True

            if self.MatchOnTCPFlags(['fin']):
                if self.fields.match.l4.tcp.fin:
                    reqspec.match.ip_selector.tcp_selector.tcp_fin_set = True
                else:
                    reqspec.match.ip_selector.tcp_selector.tcp_fin_clear = True

            if self.MatchOnTCPFlags(['rst']):
                if self.fields.match.l4.tcp.rst:
                    reqspec.match.ip_selector.tcp_selector.tcp_rst_set = True
                else:
                    reqspec.match.ip_selector.tcp_selector.tcp_rst_clear = True

            if self.MatchOnTCPFlags(['urg']):
                if self.fields.match.l4.tcp.urg:
                    reqspec.match.ip_selector.tcp_selector.tcp_urg_set = True
                else:
                    reqspec.match.ip_selector.tcp_selector.tcp_urg_clear = True

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

        if self.MatchOnIpOptions():
            reqspec.match.internal_key.ip_options = True
            reqspec.match.internal_mask.ip_options = True
    
        if self.MatchOnFlowMiss():
            reqspec.match.internal_key.flow_miss = True
            reqspec.match.internal_mask.flow_miss = True

        if self.MatchOnDirection():
            reqspec.match.internal_key.direction = \
                    self.__getDirection(self.fields.match.direction)
            reqspec.match.internal_mask.direction = True

        reqspec.action.action = self.__getEnumValue(self.fields.action.action)
        if self.ActionRedirect():
            reqspec.action.redirect_if_key_handle.if_handle = \
                    self.fields.action.redirect_if.hal_handle
        if self.ActionTunnelRedirect() or self.ActionUplinkRedirect():
            reqspec.action.internal_actions.mac_sa_rewrite_en = True
            reqspec.action.internal_actions.mac_sa = self.fields.action.macsa.getnum()
            reqspec.action.internal_actions.mac_da_rewrite_en = True
            reqspec.action.internal_actions.mac_da = self.fields.action.macda.getnum()
            reqspec.action.internal_actions.ttl_dec_en = True
            reqspec.action.internal_actions.encap_info.encap_value = self.fields.action.encap_id

        for ssn in self.ing_mirror_sessions:
            ssn_spec = reqspec.action.ing_mirror_sessions.add()
            ssn_spec.session_id = ssn.id
        for ssn in self.egr_mirror_sessions:
            ssn_spec = reqspec.action.egr_mirror_sessions.add()
            ssn_spec.session_id = ssn.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.acl_handle.handle
        cfglogger.info("  - Acl %s = %s handle: 0x%x" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status), 
                        self.hal_handle))
        return

    def PrepareHALDeleteRequestSpec(self, reqspec):
        reqspec.key_or_handle.acl_handle.handle = self.hal_handle
        return

    def ProcessHALDeleteResponse(self, req_spec, resp_spec):
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def ConfigFlowMiss(self):
        # Do we need to send pkt such that it hits flow miss
        return self.fields.config_flow_miss

    def MatchOnTEPMiss(self):
        return self.fields.match.tep_miss

    def MatchTEPMissDIP(self):
        return resmgr.AclIPv4Allocator.get()

    def MatchOnFlowMiss(self):
        # Is the ACL added matching on flow miss bit
        return self.fields.match.flow_miss

    def MatchOnDirection(self):
        if self.fields.match.direction is not None:
            return True
        return False

    def MatchOnSIF(self):
        return self.fields.match.src_if_match

    def MatchOnDIF(self):
        return self.fields.match.dst_if_match

    def MatchOnTenant(self):
        return self.fields.match.tenant_match

    def MatchOnSegment(self):
        return self.fields.match.segment_match

    def MatchOnEth(self):
        return self.fields.match.type == 'eth'

    def MatchOnIP(self):
        return self.fields.match.type == 'ip'

    def MatchOnIPv4(self):
        return self.MatchOnIP() and self.fields.match.ip.type == 'v4'

    def MatchOnIPv6(self):
        return self.MatchOnIP() and self.fields.match.ip.type == 'v6'

    def MatchOnIpOptions(self):
        return self.MatchOnIP() and self.fields.match.ip.options

    def MatchOnL4Proto(self):
        return self.MatchOnIP() and self.fields.match.l4.type == 'proto'

    def MatchOnTCP(self):
        return self.MatchOnIP() and self.fields.match.l4.type == 'tcp'

    def MatchOnTCPFlags(self, flags=['syn','ack','fin','rst','urg']):
        if not self.MatchOnTCP():
            return False

        for flag in flags:
            if getattr(self.fields.match.l4.tcp, flag) is not None:
                return True

    def MatchTCPFlags(self):
        flags=['syn','ack','fin','rst','urg']
        f = []
        for flag in flags:
            if getattr(self.fields.match.l4.tcp, flag):
                f.append(flag)

        return ','.join(f)

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
        if self.MatchOnEth() and self.fields.match.eth.ethertype_mask.get():
            return True
        return False

    def MatchEtherType(self):
        if self.MatchOnEth():
            return self.fields.match.eth.ethertype.get()
        return None

    def MatchOnSIP(self):
        if self.MatchOnIP() and self.fields.match.ip.src_prefix_len.get():
            return True
        return False

    def MatchOnDIP(self):
        if self.MatchOnIP() and self.fields.match.ip.dst_prefix_len.get():
            return True
        return False

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

    def MatchOnUDPSport(self):
        if self.MatchOnUDP() and self.fields.match.l4.udp.src_port:
            return True
        return False

    def MatchUDPSport(self):
        if self.MatchOnUDP():
            return self.fields.match.l4.udp.src_port_range.get() 
        return None 

    def MatchOnUDPDport(self):
        if self.MatchOnUDP() and self.fields.match.l4.udp.dst_port:
            return True
        return False

    def MatchUDPDport(self):
        if self.MatchOnUDP():
            return self.fields.match.l4.udp.dst_port_range.get() 
        return None 

    def MatchOnTCPSport(self):
        if self.MatchOnTCP() and self.fields.match.l4.tcp.src_port:
            return True
        return False

    def MatchTCPSport(self):
        if self.MatchOnTCP():
            return self.fields.match.l4.tcp.src_port_range.get() 
        return None 

    def MatchOnTCPDport(self):
        if self.MatchOnTCP() and self.fields.match.l4.tcp.dst_port:
            return True
        return False

    def MatchTCPDport(self):
        if self.MatchOnTCP():
            return self.fields.match.l4.tcp.dst_port_range.get() 
        return None 

    def MatchOnICMPCode(self):
        if self.MatchOnICMP() and self.fields.match.l4.icmp.code_mask.get() != 0:
            return True
        return False

    def MatchICMPCode(self):
        if self.MatchOnICMP():
            return self.fields.match.l4.icmp.code.get()
        return None

    def MatchOnICMPType(self):
        if self.MatchOnICMP() and self.fields.match.l4.icmp.type_mask.get() != 0:
            return True
        return False

    def MatchICMPType(self):
        if self.MatchOnICMP():
            return self.fields.match.l4.icmp.type.get()
        return None

    def ActionRedirect(self):
        if self.fields.action.action == 'redirect':
            return True
        return False

    def ActionUplinkRedirect(self):
        if self.fields.action.action == 'redirect' and self.fields.action.intf == 'uplink':
            return True
        return False

    def ActionTunnelRedirect(self):
        if self.fields.action.action == 'redirect' and self.fields.action.intf == 'tunnel':
            return True
        return False

    def ActionSupRedirect(self):
        if self.fields.action.action == 'redirect' and self.fields.action.intf == 'cpu':
            return True
        return False

    def ActionMirror(self):
        if len(self.ing_mirror_sessions) or len(self.egr_mirror_sessions):
            return True
        return False

    def GetIngressMirrorSession(self, idx = 0):
        if idx > len(self.ing_mirror_sessions):
            return None
        return self.ing_mirror_sessions[idx - 1]

    def GetEgressMirrorSession(self, idx = 0):
        if idx > len(self.egr_mirror_sessions):
            return None
        return self.egr_mirror_sessions[idx - 1]

    def UpdateFromTCConfig(self, flow, sep, dep, segment, tenant):

        if self.ConfigFlowMiss():
            # Generate configs with locally generated values
            smac = resmgr.AclMacAllocator.get()
            dmac = resmgr.AclMacAllocator.get()
            etype = resmgr.AclEtypeAllocator.get()
            if flow.IsIPV4():
                sip = resmgr.AclIPv4Allocator.get()
                dip = resmgr.AclIPv4Allocator.get()
            elif flow.IsIPV6():
                sip = resmgr.AclIPv6Allocator.get()
                dip = resmgr.AclIPv6Allocator.get()
            sport = resmgr.AclL4PortAllocator.get()
            dport = resmgr.AclL4PortAllocator.get()
            icmpcode = resmgr.AclICMPTypeCodeAllocator.get()
            icmptype = resmgr.AclICMPTypeCodeAllocator.get()
        else:
            # Update the config with values from the flow
            if flow.IsMAC():
                smac = sep.macaddr
                dmac = dep.macaddr
                etype = flow.ethertype
            if flow.IsIP():
                sip = flow.sip
                dip = flow.dip
            if flow.IsTCP() or flow.IsUDP():
                sport = flow.sport
                dport = flow.dport
            if flow.IsICMP():
                icmpcode = flow.icmpcode
                icmptype = flow.icmptype

        if self.MatchOnSIF():
            self.fields.match.src_if = sep.intf

        if self.MatchOnDIF():
            self.fields.match.dst_if = dep.intf

        if self.MatchOnTenant():
            self.fields.match.tenant = tenant

        if self.MatchOnSegment():
            self.fields.match.segment = segment

        if self.MatchOnEtherType():
            self.fields.match.eth.ethertype = \
                    objects.TemplateFieldObject("const/%d" % etype)
        if self.MatchOnMacSA():
            self.fields.match.eth.src = smac
        if self.MatchOnMacDA():
            self.fields.match.eth.dst = dmac

        if self.MatchOnSIP():
            self.fields.match.ip.src_ip = sip
        if self.MatchOnDIP():
            self.fields.match.ip.dst_ip = dip
        if self.MatchOnTCPSport():
            self.fields.match.l4.tcp.src_port_range = \
                    objects.TemplateFieldObject("range/%d/%d" % (sport, sport))
        if self.MatchOnTCPDport():
            self.fields.match.l4.tcp.dst_port_range = \
                    objects.TemplateFieldObject("range/%d/%d" % (dport, dport))
        if self.MatchOnUDPSport():
            self.fields.match.l4.udp.src_port_range = \
                    objects.TemplateFieldObject("range/%d/%d" % (sport, sport))
        if self.MatchOnUDPDport():
            self.fields.match.l4.udp.dst_port_range = \
                    objects.TemplateFieldObject("range/%d/%d" % (dport, dport))
        if self.MatchOnICMPCode():
            self.fields.match.l4.icmp.code = \
                    objects.TemplateFieldObject("const/%d" % icmpcode)
        if self.MatchOnICMPType():
            self.fields.match.l4.icmp.type = \
                    objects.TemplateFieldObject("const/%d" % icmptype)

        if self.ActionSupRedirect():
            self.fields.action.redirect_if = Store.objects.Get('Cpu1')
        elif self.ActionTunnelRedirect():
            self.fields.action.redirect_if = \
                    (set(Store.GetTunnelsVxlan()) - set([dep.intf])).pop()
            self.fields.action.encap_id = 30
            self.fields.action.macsa = resmgr.AclMacAllocator.get()
            self.fields.action.macda = resmgr.AclMacAllocator.get()
        elif self.ActionUplinkRedirect():
            self.fields.action.redirect_if = \
                    (set(Store.GetTrunkingUplinks()) - set([dep.intf])).pop()
            self.fields.action.encap_id = 20
            self.fields.action.macsa = resmgr.AclMacAllocator.get()
            self.fields.action.macda = resmgr.AclMacAllocator.get()

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
        aclspec = getattr(topospec, 'acls', None)
        if aclspec is None:
            return
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
