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
    def __init__(self, session, sfep, dfep, direction, label, span):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.ID(resmgr.FlowIdAllocator.get())
        assert(sfep.type == dfep.type)

        # Private members (Not available for Filters)
        self.__domid    = sfep.dom
        self.__session  = session
        self.__sep      = sfep.ep
        self.__dep      = dfep.ep
        self.__sfep     = sfep
        self.__dfep     = dfep
        self.__sseg     = sfep.ep.segment
        self.__dseg     = dfep.ep.segment
        self.__sten     = self.__sseg.tenant
        self.__dten     = self.__dseg.tenant
        self.__span     = span.Get(Store) if span else None

        # Initialize Externally Visible Fields
        # Attributes available for Filters
        self.label      = label
        self.direction  = direction
        self.type       = sfep.type
        self.fwtype     = 'L2' if self.__sseg == self.__dseg else 'L3'
        self.sip        = None
        self.dip        = None
        self.proto      = None
        self.sport      = None
        self.dport      = None
        self.icmpid     = None
        self.icmptype   = None
        self.icmpcode   = None
        self.smac       = None
        self.dmac       = None
        self.state      = self.__sfep.flow_info.state.upper()
        self.action     = self.__sfep.flow_info.action.upper()
        self.nat_type   = self.__sfep.flow_info.nat_type.upper()
       
        self.__init_key()
        self.__init_info()
        return

    def __init_key(self):
        if self.IsIP():
            self.sip = self.__sfep.addr
            self.dip = self.__dfep.addr
            assert(self.__sfep.proto == self.__dfep.proto)
            self.proto = self.__sfep.proto
            if self.IsTCP():
                self.sport = self.__sfep.port
                self.dport = self.__dfep.port
            elif self.IsUDP():
                self.sport = self.__sfep.port
                self.dport = self.__dfep.port
            elif self.IsICMP():
                self.icmpid     = self.__sfep.icmp_id
                self.icmptype   = self.__sfep.icmp_type
                self.icmpcode   = self.__sfep.icmp_code
        elif self.IsMAC():
            self.smac = self.__sfep.addr
            self.dmac = self.__dfep.addr
            self.ethertype = self.__sfep.ethertype
        else:
            assert(0)
        return

    def __init_info(self):
        self.info = self.__sfep.flow_info
        self.tracking = self.__sfep.tracking_info
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

    def __configure_l4_key(self, l4_info):
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
            req_spec.flow_key.l2_key.l2_segment_id = self.__sseg.id
            req_spec.flow_key.l2_key.ether_type = self.ethertype
        elif self.IsIPV4():
            req_spec.flow_key.v4_key.sip = self.sip.getnum()
            req_spec.flow_key.v4_key.dip = self.dip.getnum()
            req_spec.flow_key.v4_key.ip_proto = self.__get_hal_ipproto()
            self.__configure_l4_key(req_spec.flow_key.v4_key)
        elif self.IsIPV6():
            req_spec.flow_key.v6_key.sip.ip_af = haldefs.common.IP_AF_INET6
            req_spec.flow_key.v6_key.sip.v6_addr = self.sip.getnum().to_bytes(16, 'big')
            req_spec.flow_key.v6_key.dip.ip_af = haldefs.common.IP_AF_INET6
            req_spec.flow_key.v6_key.dip.v6_addr = self.dip.getnum().to_bytes(16, 'big')
            req_spec.flow_key.v6_key.ip_proto = self.__get_hal_ipproto()
            self.__configure_l4_key(req_spec.flow_key.v6_key)
        else:
            assert(0)

        action = "FLOW_ACTION_" + self.action
        req_spec.flow_data.flow_info.flow_action = haldefs.session.FlowAction.Value(action)
        nat_type = "NAT_TYPE_" + self.nat_type
        req_spec.flow_data.flow_info.nat_type = haldefs.session.NatType.Value(nat_type)

        if self.IsIPV4():
            req_spec.flow_data.flow_info.nat_sip.ip_af = haldefs.common.IP_AF_INET
            req_spec.flow_data.flow_info.nat_sip.v4_addr = self.__sfep.flow_info.nat_sip.getnum()
            req_spec.flow_data.flow_info.nat_dip.ip_af = haldefs.common.IP_AF_INET
            req_spec.flow_data.flow_info.nat_dip.v4_addr = self.__sfep.flow_info.nat_dip.getnum()
        elif self.IsIPV6():
            req_spec.flow_data.flow_info.nat_sip.ip_af = haldefs.common.IP_AF_INET6
            req_spec.flow_data.flow_info.nat_sip.v6_addr = self.__sfep.flow_info.nat_sip.getnum().to_bytes(16, 'big')
            req_spec.flow_data.flow_info.nat_dip.ip_af = haldefs.common.IP_AF_INET6
            req_spec.flow_data.flow_info.nat_dip.v6_addr = self.__sfep.flow_info.nat_dip.getnum().to_bytes(16, 'big')

        req_spec.flow_data.flow_info.nat_sport = self.__sfep.flow_info.nat_sport.get()
        req_spec.flow_data.flow_info.nat_dport = self.__sfep.flow_info.nat_dport.get()

        if self.IsTCP():
            tcp_state = "FLOW_TCP_STATE_" + self.state
            req_spec.flow_data.flow_info.tcp_state = haldefs.session.FlowTCPState.Value(tcp_state)
            req_spec.flow_data.conn_track_info.tcp_seq_num = self.__sfep.tracking_info.tcp_seq_num.get()
            req_spec.flow_data.conn_track_info.tcp_ack_num = self.__sfep.tracking_info.tcp_ack_num.get()
            req_spec.flow_data.conn_track_info.tcp_win_sz = self.__sfep.tracking_info.tcp_win_sz.get()
            req_spec.flow_data.conn_track_info.tcp_win_scale = self.__sfep.tracking_info.tcp_win_scale.get()
            req_spec.flow_data.conn_track_info.tcp_mss = self.__sfep.tracking_info.tcp_mss.get()

        #req_spec.flow_data.flow_info.egress_mirror_session = self.__span.hal_handle

        return

    def __str__(self):
        string = "%s:%s:" % (self.__session.GID(), self.direction)
        string += self.type + '/'
        if self.IsIP():
            string += "%d/%s/%s/" % (self.__domid, self.sip.get(), self.dip.get())
        else:
            string += "%d/%s/%s/" % (self.__domid, self.smac.get(), self.dmac.get())

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

        string += " --> %s" % self.action
        if self.IsTCP():
            string += "/%s" % self.state
        return string

    def IsFilterMatch(self, config_filter):
        cfglogger.debug("Matching Flow %s" % self.GID())
        # Match Source Tenant
        match = self.__sten.IsFilterMatch(config_filter.src.tenant)
        cfglogger.debug("- Source Tenant Filter Match =", match)
        if match == False: return match
        # Match Destination Tenant
        match = self.__dten.IsFilterMatch(config_filter.dst.tenant)
        cfglogger.debug("- Destination Tenant Filter Match =", match)
        if match == False: return match
        # Match Source Segment
        match = self.__sseg.IsFilterMatch(config_filter.src.segment)
        cfglogger.debug("- Source Segment Filter Match =", match)
        if match == False: return match
        # Match Destination Segment
        match = self.__dseg.IsFilterMatch(config_filter.dst.segment)
        cfglogger.debug("- Destination Segment Filter Match =", match)
        if match == False: return match
        # Match Source Endpoint
        match = self.__sep.IsFilterMatch(config_filter.src.endpoint)
        cfglogger.debug("- Source Endpoint Filter Match =", match)
        if match == False: return match
        # Match Destination Endpoint
        match = self.__dep.IsFilterMatch(config_filter.dst.endpoint)
        cfglogger.debug("- Destination Endpoint Filter Match =", match)
        if match == False: return match
        # Match Source Interface
        match = self.__sep.intf.IsFilterMatch(config_filter.src.interface)
        cfglogger.debug("- Source Interface Filter Match =", match)
        if match == False: return match
        # Match Destination Interface
        match = self.__dep.intf.IsFilterMatch(config_filter.dst.interface)
        cfglogger.debug("- Destination Interface Filter Match =", match)
        if match == False: return match
        # Match Flow
        match = super().IsFilterMatch(config_filter.flow.filters)
        cfglogger.debug("- Flow Filter Match =", match)
        if match == False: return match
        return True

    def SetupTestcaseConfig(self, obj):
        obj.flow = self
        obj.src.tenant = self.__sten 
        obj.dst.tenant = self.__dten
        obj.src.segment = self.__sseg
        obj.dst.segment = self.__dseg
        obj.src.endpoint = self.__sep
        obj.dst.endpoint = self.__dep
        return

    def ShowTestcaseConfig(self, obj, logger):
        logger.info("Config Objects for %s %s" %\
                  (self.direction, self.GID()))
        logger.info("- Src EP: %s" % obj.src.endpoint.GID())
        logger.info("- Src IF: %s" % obj.src.endpoint.intf.GID())
        logger.info("- Dst EP: %s" % obj.dst.endpoint.GID())
        logger.info("- Dst IF: %s" % obj.dst.endpoint.intf.GID())
        logger.info("- Flow  : %s" % obj.flow)

