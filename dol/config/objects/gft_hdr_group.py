#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment
import config.objects.tenant    as tenant

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import cfglogger
from config.store               import Store

class GftHeaderGroupObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('GFT_EXM_HEADER_GROUP'))
        return

    def Init(self, spec):
        self.GID(spec.name)
        self.headers = objects.MergeObjects(getattr(spec, 'headers', None),
                                            self.headers)
        self.fields = objects.MergeObjects(getattr(spec, 'fields', None),
                                           self.fields)
        bases = getattr(spec, 'inherit', [])
        for base in bases:
            self.__inherit_base(base)
        return

    def __inherit_base(self, baseref):
        base = baseref.Get(Store)
        for k,v in base.headers.__dict__.items():
            if v: self.headers.__dict__[k] = v
        for k,v in base.fields.__dict__.items():
            if v: self.fields.__dict__[k] = v
        return

    def PrepareGftFlowHALRequestSpec(self, req_spec, flow):
        self.PrepareHALRequestSpec(req_spec)
        if self.fields.dst_mac_addr:
            req_spec.eth_fields.dst_mac_addr = flow.dmac.getnum()
        if self.fields.src_mac_addr:
            req_spec.eth_fields.src_mac_addr = flow.smac.getnum()
        if self.fields.eth_type:
            req_spec.eth_fields.eth_type = flow.ethertype
        if self.fields.customer_vlan_id:
            req_spec.eth_fields.customer_vlan_id = flow.GetSrcSegmentVlanid()
        if self.fields.provider_vlan_id:
            req_spec.eth_fields.provider_vlan_id = 0 # TBD
        if self.fields.src_ip_addr:
            req_spec.src_ip_addr.ip_af = haldefs.common.IP_AF_INET
            req_spec.src_ip_addr.v4_addr = flow.sip.getnum()
        if self.fields.dst_ip_addr:
            req_spec.dst_ip_addr.ip_af = haldefs.common.IP_AF_INET
            req_spec.dst_ip_addr.v4_addr = flow.dip.getnum()
        if self.fields.ip_ttl:
            req_spec.ip_ttl = 0 # TBD
        if self.fields.ip_protocol:
            req_spec.ip_protocol = defs.ipprotos.id(flow.proto)

        if self.fields.src_port:
            if flow.IsTCP():
                req_spec.encap_or_transport.tcp_fields.sport = flow.sport
            elif flow.IsUDP():
                req_spec.encap_or_transport.udp_fields.sport = flow.sport
            else:
                assert(0)

        if self.fields.dst_port:
            if flow.IsTCP():
                req_spec.encap_or_transport.tcp_fields.dport = flow.dport
            elif flow.IsUDP():
                req_spec.encap_or_transport.udp_fields.dport = flow.dport
            else:
                assert(0)

        if self.fields.icmp_type:
            req_spec.encap_or_transport.icmp_fields.type = flow.icmptype
            req_spec.encap_or_transport.icmp_fields.code = flow.icmpcode
        return

    def PrepareHALRequestSpec(self, req_spec):
        # Set the headers.
        req_spec.headers.ethernet_header = self.headers.ethernet_header
        req_spec.headers.ipv4_header = self.headers.ipv4_header
        req_spec.headers.ipv6_header = self.headers.ipv6_header
        req_spec.headers.tcp_header = self.headers.tcp_header
        req_spec.headers.udp_header = self.headers.udp_header
        req_spec.headers.icmp_header = self.headers.icmp_header
        req_spec.headers.no_encap = self.headers.no_encap
        req_spec.headers.ip_in_ip_encap = self.headers.ip_in_ip_encap
        req_spec.headers.ip_in_gre_encap = self.headers.ip_in_gre_encap
        req_spec.headers.nvgre_encap = self.headers.nvgre_encap
        req_spec.headers.vxlan_encap = self.headers.vxlan_encap

        # Set the fields.
        req_spec.match_fields.dst_mac_addr = self.fields.dst_mac_addr
        req_spec.match_fields.src_mac_addr = self.fields.src_mac_addr
        req_spec.match_fields.eth_type = self.fields.eth_type
        req_spec.match_fields.customer_vlan_id = self.fields.customer_vlan_id
        req_spec.match_fields.provider_vlan_id = self.fields.provider_vlan_id
        req_spec.match_fields.dot1p_priority = self.fields.dot1p_priority
        req_spec.match_fields.src_ip_addr = self.fields.src_ip_addr
        req_spec.match_fields.dst_ip_addr = self.fields.dst_ip_addr
        req_spec.match_fields.ip_ttl = self.fields.ip_ttl
        req_spec.match_fields.ip_protocol = self.fields.ip_protocol
        req_spec.match_fields.ip_dscp = self.fields.ip_dscp
        req_spec.match_fields.src_port = self.fields.src_port
        req_spec.match_fields.dst_port = self.fields.dst_port
        req_spec.match_fields.tcp_flags = self.fields.tcp_flags
        req_spec.match_fields.tenant_id = self.fields.tenant_id
        req_spec.match_fields.icmp_type = self.fields.icmp_type
        req_spec.match_fields.icmp_code = self.fields.icmp_code
        req_spec.match_fields.oob_vlan = self.fields.oob_vlan
        req_spec.match_fields.oob_tenant_id = self.fields.oob_tenant_id
        req_spec.match_fields.gre_protocol = self.fields.gre_protocol
        return

    def Show(self):
        cfglogger.info("Header Group: %s" % (self.GID()))
        hdrs = ""
        if self.headers.ethernet_header: hdrs += "Eth,"
        if self.headers.ipv4_header: hdrs += "IPv4,"
        if self.headers.ipv6_header: hdrs += "IPv6,"
        if self.headers.tcp_header: hdrs += "TCP,"
        if self.headers.udp_header: hdrs += "UDP,"
        if self.headers.icmp_header: hdrs += "ICMP,"
        if self.headers.no_encap: hdrs += "NoEncap,"
        if self.headers.ip_in_ip_encap: hdrs += "IPinIP,"
        if self.headers.ip_in_gre_encap: hdrs += "IPinGRE,"
        if self.headers.nvgre_encap: hdrs += "NVGRE,"
        if self.headers.vxlan_encap: hdrs += "VXLAN,"
        cfglogger.info("- Headers: %s" % hdrs)
        fields = ""
        if self.fields.dst_mac_addr: fields += "Dmac,"
        if self.fields.src_mac_addr: fields += "Smac,"
        if self.fields.eth_type: fields += "Etype,"
        if self.fields.customer_vlan_id: fields += "CustVlanID,"
        if self.fields.provider_vlan_id: fields += "ProvVlanID,"
        if self.fields.dot1p_priority: fields += "Dot1P,"
        if self.fields.src_ip_addr: fields += "Sip,"
        if self.fields.dst_ip_addr: fields += "Dip,"
        if self.fields.ip_ttl: fields += "Ttl,"
        if self.fields.ip_protocol: fields += "Proto,"
        if self.fields.ip_dscp: fields += "Dscp,"
        if self.fields.src_port: fields += "Sport,"
        if self.fields.dst_port: fields += "Dport,"
        if self.fields.tcp_flags: fields += "TcpFlags,"
        if self.fields.tenant_id: fields += "TenantID,"
        if self.fields.icmp_type: fields += "IcmpType,"
        if self.fields.icmp_code: fields += "IcmpCode,"
        if self.fields.oob_vlan: fields += "OobVlan,"
        if self.fields.oob_tenant_id: fields += "OobTenantID,"
        if self.fields.gre_protocol: fields += "GREProto,"
        cfglogger.info("- Fields: %s" % fields)
        return

class GftHeaderGroupObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Generate(self, topospec):
        gftexm_spec = getattr(topospec, 'gftexm', None)
        if gftexm_spec is None: return

        spec = getattr(gftexm_spec, 'hdrgroups', None)
        hgs = spec.Get(Store)

        cfglogger.info("Adding GFT Header Groups to Store.")
        for entry in hgs.header_groups:
            hdrgroup = entry.group
            obj = GftHeaderGroupObject()
            obj.Init(hdrgroup)
            obj.Show()
            self.objlist.append(obj)
            Store.objects.Set(obj.GID(), obj)
        return

    def main(self, topospec):
        self.Generate(topospec)
        return

    def GetAll(self):
        return self.objlist

GftHeaderGroupHelper = GftHeaderGroupObjectHelper()
