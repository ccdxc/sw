#! /usr/bin/python3

import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils
import config.resmgr            as resmgr
import infra.config.base        as base

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api            as halapi
import config.hal.defs           as haldefs

class FlowObject(base.ConfigObjectBase):
    def __init__(self, session, sfep, dfep, direction):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        
        self.ID(resmgr.FlowIdAllocator.get())
        assert(sfep.type == dfep.type)
        self.session = session
        self.direction = direction
        self.type   = sfep.type
        self.sep    = sfep.ep
        self.dep    = dfep.ep
        self.sfep   = sfep
        self.dfep   = dfep
        self.sseg   = sfep.ep.segment
        self.dseg   = dfep.ep.segment
        self.sten   = self.sseg.tenant
        self.dten   = self.dseg.tenant

        if self.sseg == self.dseg:
            self.fwtype = 'L2'
        else:
            self.fwtype = 'L3'

        # Initialize Externally Visible Fields
        self.domid = sfep.dom
        self.__init_key()
        return

    def __init_key(self):
        if self.IsIP():
            self.sip = self.sfep.addr
            self.dip = self.dfep.addr
            assert(self.sfep.proto == self.dfep.proto)
            self.proto = self.sfep.proto
            if self.HasL4Ports():
                self.sport = self.sfep.port
                self.dport = self.dfep.port
            elif self.IsICMP():
                self.icmpid     = self.sfep.icmp_id
                self.icmptype   = self.sfep.icmp_type
                self.icmpcode   = self.sfep.icmp_code
        elif self.IsMAC():
            self.smac = self.sfep.addr
            self.dmac = self.dfep.addr
            self.ethertype = self.sfep.ethertype
        else:
            assert(0)

        return

    def IsIPV4(self):
        return self.type == 'IPV4'

    def IsIPV6(self):
        return self.type == 'IPV6'

    def IsIP(self):
        return self.IsIPV4() or self.IsIPV6()

    def IsMAC(self):
        return self.type == 'MAC'

    def IsICMP(self):
        return self.proto == 'ICMP' or self.proto == 'ICMPV6'

    def IsTCP(self):
        return self.proto == 'TCP'

    def IsUDP(self):
        return self.proto == 'UDP'

    def HasL4Ports(self):
        return self.IsTCP() or self.IsUDP()

    def GetSrcTenant(self):
        return self.sten

    def GetDstTenant(self):
        return self.dten

    def GetSrcSegment(self):
        return self.sseg

    def GetDstSegment(self):
        return self.dseg

    def GetSrcEndpoint(self):
        return self.sep

    def GetDstEndpoint(self):
        return self.dep

    def __configure_l4_info(self, l4_info):
        if self.HasL4Ports():
            l4_info.tcp_udp.sport = self.sport
            l4_info.tcp_udp.dport = self.dport
        elif self.IsICMP():
            l4_info.icmp.type = self.icmptype
            l4_info.icmp.code = self.icmpcode
            l4_info.icmp.id = self.icmpid
        else:
            assert(0)
        return

    def __get_hal_ipproto(self):
        hal_ipproto_str = 'IP_PROTO_' + self.proto
        hal_ipproto = haldefs.common.IPProtocol.Value(hal_ipproto_str)
        return hal_ipproto

    def PrepareHALRequestSpec(self, req_spec):
        if self.IsMAC():
            req_spec.flow_key.l2_key.smac = self.smac.getnum()
            req_spec.flow_key.l2_key.dmac = self.dmac.getnum()
            req_spec.flow_key.l2_key.l2_segment_id = self.sseg.id
            req_spec.flow_key.l2_key.ether_type = self.ethertype
        elif self.IsIPV4():
            req_spec.flow_key.v4_key.sip = self.sip.getnum()
            req_spec.flow_key.v4_key.dip = self.dip.getnum()
            req_spec.flow_key.v4_key.ip_proto = self.__get_hal_ipproto()
            self.__configure_l4_info(req_spec.flow_key.v4_key)
        elif self.IsIPV6():
            req_spec.flow_key.v6_key.sip.ip_af = haldefs.common.IP_AF_INET6
            req_spec.flow_key.v6_key.sip.v6_addr = self.sip.getnum().to_bytes(16, 'big')
            req_spec.flow_key.v6_key.dip.ip_af = haldefs.common.IP_AF_INET6
            req_spec.flow_key.v6_key.dip.v6_addr = self.dip.getnum().to_bytes(16, 'big')
            req_spec.flow_key.v6_key.ip_proto = self.__get_hal_ipproto()
            self.__configure_l4_info(req_spec.flow_key.v6_key)
        else:
            assert(0)

        req_spec.flow_data.flow_info.flow_action = haldefs.session.FLOW_ACTION_ALLOW
        req_spec.flow_data.flow_info.nat_type = haldefs.session.NAT_TYPE_NONE
        req_spec.flow_data.flow_info.tcp_state = haldefs.session.FLOW_TCP_STATE_ESTABLISHED

        return

    def __str__(self):
        string = "%s:%s:" % (self.session.GID(), self.direction)
        string += self.type + '/'
        if self.IsIP():
            string += "%d/%s/%s/" % (self.domid, self.sip.get(), self.dip.get())
        else:
            string += "%d/%s/%s/" % (self.domid, self.smac.get(), self.dmac.get())

        if self.proto: string += "%s/" % self.proto
        if self.IsIP():
            if self.HasL4Ports():
                string += "%d/%d" % (self.sport, self.dport)
            elif self.IsICMP():
                string += "%d/%d/%d" % (self.icmptype, self.icmpcode, self.icmpid)
        elif self.IsMAC():
            string += "%04x" % (self.ethertype)
        else:
            assert(0)
        return string

    def IsFilterMatch(self, config_filter):
        # Match Source Tenant
        match = self.sten.IsFilterMatch(config_filter.src.tenant)
        if match == False: return match
        # Match Destination Tenant
        match = self.dten.IsFilterMatch(config_filter.dst.tenant)
        if match == False: return match
        # Match Source Segment
        match = self.sseg.IsFilterMatch(config_filter.src.segment)
        if match == False: return match
        # Match Destination Segment
        match = self.dseg.IsFilterMatch(config_filter.dst.segment)
        if match == False: return match
        # Match Source Endpoint
        match = self.sep.IsFilterMatch(config_filter.src.endpoint)
        if match == False: return match
        # Match Destination Endpoint
        match = self.dep.IsFilterMatch(config_filter.dst.endpoint)
        if match == False: return match
        # Match Source Interface
        match = self.sep.intf.IsFilterMatch(config_filter.src.interface)
        if match == False: return match
        # Match Destination Interface
        match = self.dep.intf.IsFilterMatch(config_filter.dst.interface)
        if match == False: return match
        # Match Flow
        match = super().IsFilterMatch(config_filter.flow.filters)
        if match == False: return match
        return True
