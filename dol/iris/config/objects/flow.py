#! /usr/bin/python3

import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils
import iris.config.resmgr            as resmgr
import infra.config.base        as base

from iris.config.store                       import Store
from infra.common.logging               import logger
from iris.config.objects.security_policy     import SecurityGroupPolicyHelper
import iris.config.hal.api            as halapi
import iris.config.hal.defs           as haldefs

from iris.config.hashgen             import TcpUdpHashGen

FLOW_COLLISION_LABEL = 'FLOW-COLLISION'
class FlowObject(base.ConfigObjectBase):
    def __init__(self, session, sfep, dfep, direction, label, span):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.ID(resmgr.FlowIdAllocator.get())
        assert(sfep.type == dfep.type)

        sfep.SetFlow(self)
        dfep.SetFlow(self)

        # Private members (Not available for Filters)
        self.__domid    = sfep.dom
        self.__session  = session
        self.__sep      = sfep.ep
        self.__dep      = dfep.ep
        self.__sfep     = sfep
        self.__dfep     = dfep
        self.__sseg     = sfep.GetSegment()
        self.__dseg     = dfep.GetSegment()
        self.__sten     = self.__sseg.tenant
        self.__dten     = self.__dseg.tenant
        self.__span     = span
        self.__flowhash = None
        self.__gft_flow = None
        self.multicast  = False
        self.ing_mirror_sessions = []
        self.egr_mirror_sessions = []
        if span:
            if 'ingress' in span.__dict__:
                for s in span.ingress:
                    ispan   = s.Get(Store)
                    self.ing_mirror_sessions.append(ispan)
            if 'egress' in span.__dict__:
                for s in span.egress:
                    espan   = s.Get(Store)
                    self.egr_mirror_sessions.append(espan)

        # Initialize Externally Visible Fields
        # Attributes available for Filters
        self.label      = label
        self.direction  = direction
        self.type       = sfep.type
        self.fwtype     = 'L2' if self.__sseg == self.__dseg else 'L3'
        self.sseg_id    = self.__sseg.id
        self.dseg_id    = self.__dseg.id
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
        self.espspi     = None
        self.nat_dmac   = None
        self.state      = self.__sfep.flow_info.state.upper()
        self.action     = self.__sfep.flow_info.action.upper()
        self.fte        = False

        self.__init_key()
        self.__init_info()
        self.__init_nat()
        self.__init_qos()
        self.__init_sp()
        self.__init_distlabel()

        if self.__session.IsFteEnabled():
            self.fte = True
        return

    def SetGftFlow(self, gft_flow):
        self.__gft_flow = gft_flow
        return

    def GetGftFlow(self):
        return self.__gft_flow

    def IsRetryEnabled(self):
        return self.fte

    def SetMulticast(self, mc):
        self.multicast = mc
        return

    def IsMulticast(self):
        return self.multicast

    def GetMulticastL3Type(self):
        return self.__dfep.group.l3type

    def GetMulticastEnicOifList(self):
        return self.__dfep.group.GetEnicOifList()

    def GetMulticastGroup(self):
        return self.__dfep.group

    def __init_sp(self):
        action = SecurityGroupPolicyHelper.GetAction(self, self.__sep, self.__dep)
        if action is not None:
            self.action = action
        self.sgtype = SecurityGroupPolicyHelper.GetSGType(self, self.__sep, self.__dep)

    def __init_hashgen(self):
        if self.IsIflow() and (self.IsTCP() or self.IsUDP()):
            if self.__sep.IsRemote():
                direction = 'FROM_UPLINK'
            else:
                direction = 'FROM_ENIC'

            ret = TcpUdpHashGen.Process(self.sip.getnum(),
                                        self.dip.getnum(),
                                        self.proto,
                                        self.type,
                                        direction,
                                        self.__sseg.vrf_id)

            self.__flowhash, skip, drop, sport, dport  = ret
            if skip:
                self.__session.SetFteEnabled()
            if drop:
                self.action = 'DROP'

            self.__sfep.SetFlowSport(sport)
            self.__dfep.SetFlowDport(dport)

        else:
            # Hash gen is not supported for non-tcp/udp flows
            assert 0
        return

    def __init_qos(self):
        self.txqos.cos  = self.__sfep.GetTxQosCos()
        self.txqos.dscp = self.__sfep.GetTxQosDscp()
        self.rxqos.cos  = self.__dfep.GetRxQosCos()
        self.rxqos.dscp = self.__dfep.GetRxQosDscp()
        return

    def __init_distlabel(self):
        src_intf = self.__sfep.GetInterface()
        dst_intf = self.__dfep.GetInterface()
        src_label = "None"
        dst_label = "None"
        if src_intf:
            src_label = src_intf.GetDistLabel()
        if dst_intf:
            dst_label = dst_intf.GetDistLabel()
        self.distlabel = "%s_to_%s" % (src_label, dst_label)
        return

    def __init_nat(self):
        # No-OP for DSR backend
        if self.__sfep.IsL4Lb() and self.__sfep.l4lb.IsNATDSR():
            self.nat_type = 'NONE'
            self.nat_rw = 'NONE'
            return

        if self.__sfep.IsL4Lb():
            if self.__sfep.l4lb.IsTwiceNAT():
                # Twice NAT
                self.nat_type = 'TWICE_NAT'
                if self.__sfep.l4lb.IsServiceSNatPortValid():
                    self.nat_rw = 'IP_PORT'
                else:
                    self.nat_rw = 'IP_ONLY'
            else:
                # SNAT
                self.nat_type = 'SNAT'
                if self.__sfep.l4lb.IsBackendPortValid():
                    self.nat_rw = 'IP_PORT'
                else:
                    self.nat_rw = 'IP_ONLY'
        elif self.__dfep.IsL4Lb():
            if self.__dfep.l4lb.IsTwiceNAT():
                # Twice NAT
                self.nat_type = 'TWICE_NAT'
                if self.__dfep.l4lb.IsServiceSNatPortValid():
                    self.nat_rw = 'IP_PORT'
                else:
                    self.nat_rw = 'IP_ONLY'
            else:
                self.nat_type = 'DNAT'
                # DSR Case
                if self.__dfep.IsNATDSR():
                    if self.__dfep.l4lb.IsBackendPortValid():
                        self.nat_rw = 'MAC_PORT'
                    else:
                        self.nat_rw = 'MAC_ONLY'
                else:
                    if self.__dfep.l4lb.IsBackendPortValid():
                        self.nat_rw = 'IP_PORT'
                    else:
                        self.nat_rw = 'IP_ONLY'
        else:
            self.nat_type = 'NONE'
            self.nat_rw = 'NONE'

        if self.IsIflow() and self.__dfep.l4lb.IsTwiceNAT():
            if self.IsIPV4():
                self.nat_sip   = self.__dfep.l4lb.GetServiceSNatIpAddress()
            else:
                self.nat_sip   = self.__dfep.l4lb.GetServiceSNatIpv6Address()
            if self.nat_rw == 'IP_ONLY':
                self.nat_sport = 0
            else:
                self.nat_sport = self.__dfep.l4lb.GetServiceSNatPort()
        else:
            self.nat_sip   = self.__sfep.GetNatSip()
            if self.nat_rw == 'IP_ONLY':
                self.nat_sport = 0
            else:
                self.nat_sport = self.__sfep.GetNatSport()

        if not self.IsIflow() and self.__sfep.l4lb.IsTwiceNAT():
            self.nat_dip   = self.__dfep.GetFlowSip()
            if self.nat_rw == 'IP_ONLY':
                self.nat_dport = 0
            else:
                self.nat_dport = self.__dfep.GetFlowSport()
        else:
            if self.__dfep.l4lb.IsNATDSR():
                self.nat_dip = objects.IpAddress(integer=0)
                self.nat_dmac = self.__dfep.ep.macaddr
                if self.nat_rw == 'MAC_ONLY':
                    self.nat_dport = 0
                else:
                    self.nat_dport = self.__dfep.GetNatDport()
            else:
                self.nat_dip   = self.__dfep.GetNatDip()
                if self.nat_rw == 'IP_ONLY':
                    self.nat_dport = 0
                else:
                    self.nat_dport = self.__dfep.GetNatDport()
        return


    def __init_ip_flow_key(self):
        self.sip = self.__sfep.GetFlowSip()
        # Twice NAT - Rflow
        if not self.IsIflow() and self.__sfep.l4lb.IsTwiceNAT():
            if self.IsIPV4():
                self.dip = self.__sfep.l4lb.GetServiceSNatIpAddress()
            else:
                self.dip = self.__sfep.l4lb.GetServiceSNatIpv6Address()
        else:
            self.dip = self.__dfep.GetFlowDip()
        assert(self.__sfep.proto == self.__dfep.proto)
        self.proto = self.__sfep.proto
        if self.__sfep.hashgen:
            self.__init_hashgen()

        if self.IsTCP() or self.IsUDP():
            self.sport = self.__sfep.GetFlowSport()
            # Twice NAT - Rflow
            if not self.IsIflow() and self.__sfep.l4lb.IsTwiceNAT():
                self.dport = self.__sfep.l4lb.GetServiceSNatPort()
            else:
                self.dport = self.__dfep.GetFlowDport()
        elif self.IsICMP():
            self.icmpid     = self.__sfep.icmp_id
            self.icmptype   = self.__sfep.icmp_type
            self.icmpcode   = self.__sfep.icmp_code
        elif self.IsESP():
            self.espspi     = self.__sfep.esp_spi

        self.smac = self.__sfep.ep.macaddr
        self.dmac = self.__dfep.ep.macaddr
        if self.IsIPV4():
            self.ethertype = 0x800
        elif self.IsIPV6():
            self.ethertype = 0x86DD
        else:
            assert(0)

        return

    def __init_mac_flow_key(self):
        self.smac = self.__sfep.addr
        self.dmac = self.__dfep.addr
        self.ethertype = self.__sfep.ethertype
        return

    def __init_key(self):
        if self.IsIP():
            self.__init_ip_flow_key()
        elif self.IsMAC():
            self.__init_mac_flow_key()
        else:
            assert(0)
        return

    def __init_info(self):
        self.info = self.__sfep.flow_info
        self.tracking = self.__sfep.tracking_info
        return

    def IsFteEnabled(self):
        return self.__session.IsFteEnabled()

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

    def IsL2(self):
        return self.fwtype == 'L2'

    def IsDrop(self):
        if self.action == 'DROP':
            return True
        if self.__sten.IsHostPinned() is False:
            return False
        # Host Pinned mode
        if self.__sep.remote and self.__dep.remote:
            return True
        return False

    def SepIsRemote(self):
        return self.__sep.IsRemote()

    def IsESP(self):
        return self.proto == 'ESP'

    def HasL4Ports(self):
        return self.IsTCP() or self.IsUDP()

    def IsCollisionFlow(self):
        return self.label == FLOW_COLLISION_LABEL

    def SetLabel(self, label):
        logger.info("Updating %s Label to %s" % (self.GID(), label))
        self.label = label
        return

    def GetSession(self):
        return self.__session

    def GetStats(self):
        return self.__session.GetStats()

    def GetOpposingFlow(self):
        if self.IsIflow():
            return self.__session.GetRflow()
        else:
            return self.__session.GetIflow()

    def GetIngressMirrorSession(self, idx = 0):
        if idx > len(self.ing_mirror_sessions):
            return None
        return self.ing_mirror_sessions[idx - 1]

    def GetEgressMirrorSession(self, idx = 0):
        if idx > len(self.egr_mirror_sessions):
            return None
        return self.egr_mirror_sessions[idx - 1]

    def IsSnat(self):
        return self.nat_type == 'SNAT'

    def IsDnat(self):
        return self.nat_type == 'DNAT'

    def IsTwiceNAT(self):
        return self.nat_type == 'TWICE_NAT'

    def IsNat(self):
        return self.nat_type != 'NONE'

    def IsIflow(self):
        return self.direction == 'IFLOW'

    def GetSrcSegmentVlanid(self):
        return self.__sseg.vlan_id
    def GetDstSegmentVlanid(self):
        return self.__dseg.vlan_id

    def GetSrcSegmentGftExmProfile(self):
        if self.IsIP():
            profstr = self.type + '_' + self.proto
        else:
            profstr = 'ETH'
        profile = self.__sseg.GetExmProfile(profstr)
        return profile

    def GetGftTranspositionProfile(self):
        trp_name = "GFT_TRSPN_"
        if self.__sseg.IsFabEncapVlan() == self.__dseg.IsFabEncapVlan():
            trp_name += "VLAN_"
            if (self.__sseg.IsNative() and self.__dseg.IsNative()) or self.label == "RDMA":
                # Both are untagged.
                trp_name += "NONE"
            elif self.__sseg.IsNative() == self.__dseg.IsNative():
                # Both are tagged.
                trp_name += "MODIFY"
            elif self.__sseg.IsNative():
                # SrcSegment is untagged and DstSegment is tagged.
                trp_name += "PUSH"
            else:
                # SrcSegment is tagged and DstSegment is untagged.
                trp_name += "POP"
        return Store.objects.Get(trp_name)

    def __configure_l4_key(self, l4_info):
        if self.HasL4Ports():
            l4_info.tcp_udp.sport = self.sport
            l4_info.tcp_udp.dport = self.dport
        elif self.IsICMP():
            l4_info.icmp.type = self.icmptype
            l4_info.icmp.code = self.icmpcode
            l4_info.icmp.id = self.icmpid
        elif self.IsESP():
            l4_info.esp.spi = self.espspi
        return

    def __get_hal_ipproto(self):
        #hal_ipproto_str = 'IPPROTO_' + self.proto
        #hal_ipproto = haldefs.common.IPProtocol.Value(hal_ipproto_str)
        return defs.ipprotos.id(self.proto)

    def __getStringValue(self, field, val):
        return field.Name(val).split("_")[-1]

    def __copy__(self):
        flow = FlowObject(self.__session, self.__sfep, self.__dfep, self.direction,
                           self.label, self.__span)
        flow.smac = self.smac
        flow.dmac = self.dmac
        if self.IsMAC():
            flow.ethertype = self.ethertype
        else:
            flow.proto = self.proto
            flow.sip = self.sip
            flow.dip = self.dip
        flow.sseg_id = self.sseg_id
        flow.dseg_id = self.dseg_id
        flow.type = self.type
        return flow

    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False

        fields = ["type"]
        if not self.CompareObjectFields(other, fields, lgh):
            return False

        if self.IsMAC():
            fields.extend(["smac", "dmac", "ethertype"])
        elif self.IsIP():
            fields.extend(["sip", "dip", "proto"])
        else:
            assert 0

        if not self.CompareObjectFields(other, fields, lgh):
            return False

        if self.IsTCP() or self.IsUDP():
            fields.extend(["sport", "dport"])
        elif self.IsICMP():
            fields.extend(["icmptype", "icmpcode", "icmpid"])
        elif self.IsESP():
            #TODO
            pass

        if self.IsNat():
            fields.extend(["nat_sip", "nat_dip", "nat_sport", "nat_dport"])

        #This has to be generalized.
        #if not utils.CompareObjectFields(self.txqos, other.txqos, ["cos", "dscp"], lgh):
        #    return False

        #if not utils.CompareObjectFields(self.rxqos, other.rxqos, ["cos", "dscp"], lgh):
        #    return False

        return True

    def PrepareHALRequestFlowKeySpec(self, req_spec):
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


    def PrepareHALRequestSpec(self, req_spec):
        #prepare the flow key
        self.PrepareHALRequestFlowKeySpec(req_spec)

        #prepare the flow data
        action = "FLOW_ACTION_" + self.action
        if action == "FLOW_ACTION_DENY":
            action = "FLOW_ACTION_DROP"
        req_spec.flow_data.flow_info.flow_action = haldefs.session.FlowAction.Value(action)
        nat_type = "NAT_TYPE_" + self.nat_type
        req_spec.flow_data.flow_info.nat_type = haldefs.session.NatType.Value(nat_type)

        if self.IsNat():
            if self.IsIPV4():
                req_spec.flow_data.flow_info.nat_sip.ip_af = haldefs.common.IP_AF_INET
                req_spec.flow_data.flow_info.nat_sip.v4_addr = self.nat_sip.getnum()
                if not self.__dfep.IsNATDSR():
                    req_spec.flow_data.flow_info.nat_dip.ip_af = haldefs.common.IP_AF_INET
                    req_spec.flow_data.flow_info.nat_dip.v4_addr = self.nat_dip.getnum()
            elif self.IsIPV6():
                req_spec.flow_data.flow_info.nat_sip.ip_af = haldefs.common.IP_AF_INET6
                req_spec.flow_data.flow_info.nat_sip.v6_addr = self.nat_sip.getnum().to_bytes(16, 'big')
                if not self.__dfep.IsNATDSR():
                    req_spec.flow_data.flow_info.nat_dip.ip_af = haldefs.common.IP_AF_INET6
                    req_spec.flow_data.flow_info.nat_dip.v6_addr = self.nat_dip.getnum().to_bytes(16, 'big')

            if self.__dfep.IsNATDSR():
                req_spec.flow_data.flow_info.nat_dmac = self.nat_dmac.getnum()

            # Send NAT ports to HAL, only if port rewrite is required.
            if self.__sfep.IsNatPortValid() or self.__dfep.IsNatPortValid():
                req_spec.flow_data.flow_info.nat_sport = self.nat_sport
                req_spec.flow_data.flow_info.nat_dport = self.nat_dport

        if self.IsTCP():
            tcp_state = "FLOW_TCP_STATE_" + self.state
            req_spec.flow_data.flow_info.tcp_state = haldefs.session.FlowTCPState.Value(tcp_state)
            req_spec.flow_data.conn_track_info.tcp_seq_num = self.__sfep.tracking_info.tcp_seq_num.get()
            req_spec.flow_data.conn_track_info.tcp_ack_num = self.__sfep.tracking_info.tcp_ack_num.get()
            req_spec.flow_data.conn_track_info.tcp_win_sz = self.__sfep.tracking_info.tcp_win_sz.get()
            req_spec.flow_data.conn_track_info.tcp_win_scale = self.__sfep.tracking_info.tcp_win_scale.get()
            req_spec.flow_data.conn_track_info.tcp_mss = self.__sfep.tracking_info.tcp_mss.get()

        # QOS stuff
        #if self.__sten.IsQosEnabled():
        #if self.txqos.cos is not None:
        #    req_spec.flow_data.flow_info.eg_qos_actions.marking_spec.pcp_rewrite_en = True
        #    req_spec.flow_data.flow_info.eg_qos_actions.marking_spec.pcp = self.txqos.cos
        #if self.txqos.dscp is not None:
        #    req_spec.flow_data.flow_info.eg_qos_actions.marking_spec.dscp_rewrite_en = True
        #    req_spec.flow_data.flow_info.eg_qos_actions.marking_spec.dscp = self.txqos.dscp

        #req_spec.flow_data.flow_info.egress_mirror_session = self.__span.hal_handle
        for ssn in self.ing_mirror_sessions:
            ssn_spec = req_spec.flow_data.flow_info.ing_mirror_sessions.add()
            ssn_spec.mirrorsession_id = ssn.id - 1
        for ssn in self.egr_mirror_sessions:
            ssn_spec = req_spec.flow_data.flow_info.egr_mirror_sessions.add()
            ssn_spec.mirrorsession_id = ssn.id - 1
        return

    def Summary(self):
        string = "%s:%s:%s:" % (self.__session.GID(), self.direction, self.fwtype)
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
            elif self.IsESP():
                string += "%d" % self.espspi
        elif self.IsMAC():
            string += "%04x" % (self.ethertype)
        else:
            assert(0)
        return string

    def Show(self):
        string = self.Summary()
        logger.info("- %s    : %s" % (self.direction, string))
        string = "%s" % self.action
        if self.IsTCP():
            string += "/%s" % self.state

        if self.__flowhash is not None:
            logger.info("  - flowhash : 0x%08x" % self.__flowhash)
        logger.info("  - label  : %s" % self.label)
        logger.info("  - distlabel: %s" % self.distlabel)
        if self.IsSnat():
            string += '/%s/%s/%d/%s' %\
                      (self.nat_type, self.nat_sip.get(),
                       self.nat_sport, self.nat_rw)
        if self.IsDnat():
            if self.__dfep.IsNATDSR():
                string += '/%s/%s/%d/%s/%s' %\
                          (self.nat_type, self.nat_dip.get(),
                           self.nat_dport,self.nat_dmac,
                           self.nat_rw)
            else:
                string += '/%s/%s/%d/%s' %\
                          (self.nat_type, self.nat_dip.get(),
                           self.nat_dport,
                           self.nat_rw)

        if self.IsTwiceNAT():
            string += '/%s/%s/%d/%s/%d/%s' %\
                      (self.nat_type, self.nat_sip.get(),
                       self.nat_sport, self.nat_dip.get(),
                       self.nat_dport, self.nat_rw)

        logger.info("  - info   : %s" % string)

        logger.info('  - txqos: Cos:%s/Dscp:%s' %\
                       (str(self.txqos.cos), str(self.txqos.dscp)))
        logger.info('  - rxqos: Cos:%s/Dscp:%s' %\
                       (str(self.rxqos.cos), str(self.rxqos.dscp)))

        if len(self.ing_mirror_sessions):
            string = ''
            for ssn in self.ing_mirror_sessions:
                string += ssn.GID() + ' '
            logger.info("  - IngSpan: %s" % string)

        if len(self.egr_mirror_sessions):
            string = ''
            for ssn in self.egr_mirror_sessions:
                string += ssn.GID() + ' '
            logger.info("  - EgrSpan: %s" % string)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.flow_handle
        if self.__flowhash is not None and resp_spec.flow_hash != self.__flowhash:
            assert 0, '%s: Recirc hash 0x%x does not match expected 0x%x' %\
                    (self.GID(), resp_spec.flow_hash, self.__flowhash)

        logger.info("- %s %s = (HDL = %x), Label = %s" %\
                       (self.direction, self.GID(),
                        self.hal_handle, self.label))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        #Should never be called.
        assert 0
        return


    def ProcessHALGetResponse(self, get_req_spec, get_resp):

        def set_flow_l4_info(flow_key):
            if flow_key.HasField("tcp_udp"):
                self.sport = flow_key.tcp_udp.sport
                self.dport = flow_key.tcp_udp.dport
            elif flow_key.HasField("icmp"):
                self.icmpcode = flow_key.icmp.code
                self.icmptype = flow_key.icmp.type
                self.icmpid = flow_key.icmp.id
            else:
                self.sport = self.dport = self.icmpcode = self.icmptype = self.icmpid = None

        if get_resp.flow_key.HasField("l2_key"):
            self.smac = objects.MacAddressBase(integer=get_resp.flow_key.l2_key.smac)
            self.dmac = objects.MacAddressBase(integer=get_resp.flow_key.l2_key.dmac)
            self.ethertype = get_resp.flow_key.l2_key.ether_type
            self.sseg_id = get_resp.flow_key.l2_key.l2_segment_id
            self.type = 'MAC'
        elif get_resp.flow_key.HasField("v4_key"):
            self.sip = objects.IpAddress(integer=get_resp.flow_key.v4_key.sip)
            self.dip = objects.IpAddress(integer=get_resp.flow_key.v4_key.dip)
            self.proto = self.__getStringValue(haldefs.common.IPProtocol,
                get_resp.flow_key.v4_key.ip_proto)
            self.type = 'IPV4'
            set_flow_l4_info(get_resp.flow_key.v4_key)
        elif get_resp.flow_key.HasField("v6_key"):
           self.sip = objects.Ipv6Address(integer=get_resp.flow_key.v6_key.sip.v6_addr)
           self.dip = objects.Ipv6Address(integer=get_resp.flow_key.v6_key.dip.v6_addr)
           self.proto = self.__getStringValue(haldefs.common.IPProtocol,
               get_resp.flow_key.v6_key.ip_proto)
           self.type = 'IPV6'
           set_flow_l4_info(get_resp.flow_key.v6_key)
        else:
            assert 0

        flow_info = get_resp.flow_data.flow_info
        self.action = self.__getStringValue(haldefs.session.FlowAction,
                                            flow_info.flow_action)
        self.nat_type = self.__getStringValue(haldefs.session.NatType,
                                              flow_info.nat_type)

        if self.IsNat():
            if self.IsIPV4():
                self.nat_sip = objects.IpAddress(integer=flow_info.nat_sip.v4_addr)
                self.nat_dip = objects.IpAddress(integer=flow_info.nat_dip.v4_addr)
            elif self.IsIPV6():
                set.nat_sip = objects.Ipv6Address(integer=flow_info.nat_sip.v6_addr)
                set.nat_dip = objects.Ipv6Address(integer=flow_info.nat_dip.v6_addr)
            self.nat_sport = flow_info.nat_sport
            self.nat_dport = flow_info.nat_dport

#        if flow_info.eg_qos_actions.marking_spec.pcp_rewrite_en:
#            self.txqos.cos  = flow_info.eg_qos_actions.marking_spec.pcp
#        else:
#            self.txqos.cos = None
#        if flow_info.eg_qos_actions.marking_spec.dscp_rewrite_en:
#            self.txqos.dscp  = flow_info.eg_qos_actions.marking_spec.dscp
#        else:
#            self.txqos.dscp = None

    def IsFilterMatch(self, selectors):
        logger.debug("Matching %s Flow:%s, Session:%s" %\
                        (self.direction, self.GID(), self.__session.GID()))
        # Match Source Tenant
        match = self.__sten.IsFilterMatch(selectors.src.tenant)
        logger.debug("- Source Tenant Filter Match =", match)
        if match == False: return match
        # Match Destination Tenant
        match = self.__dten.IsFilterMatch(selectors.dst.tenant)
        logger.debug("- Destination Tenant Filter Match =", match)
        if match == False: return match
        # Match Source Segment
        match = self.__sseg.IsFilterMatch(selectors.src.segment)
        logger.debug("- Source Segment Filter Match =", match)
        if match == False: return match
        # Match Destination Segment
        match = self.__dseg.IsFilterMatch(selectors.dst.segment)
        logger.debug("- Destination Segment Filter Match =", match)
        if match == False: return match
        # Match Source Endpoint
        if self.__sep:
            match = self.__sep.IsFilterMatch(selectors.src.endpoint)
            logger.debug("- Source Endpoint Filter Match =", match)
            if match == False: return match
        # Match Destination Endpoint
        if self.__dep:
            match = self.__dep.IsFilterMatch(selectors.dst.endpoint)
            logger.debug("- Destination Endpoint Filter Match =", match)
            if match == False: return match
        # Match Source Interface
        if self.__sep:
            match = self.__sep.intf.IsFilterMatch(selectors.src.interface)
            logger.debug("- Source Interface Filter Match =", match)
            if match == False: return match
        # Match Destination Interface
        if self.__dep:
            match = self.__dep.intf.IsFilterMatch(selectors.dst.interface)
            logger.debug("- Destination Interface Filter Match =", match)
            if match == False: return match
        # Match Source Lif
        if self.__sep and self.__sep.remote == False:
            match = self.__sep.intf.lif.IsFilterMatch(selectors.src.lif)
            logger.debug("- Source Lif Filter Match =", match)
            if match == False: return match
        # Match Destination Lif
        if self.__dep and self.__dep.remote == False:
            match = self.__dep.intf.lif.IsFilterMatch(selectors.dst.lif)
            logger.debug("- Destination Lif Filter Match =", match)
            if match == False: return match
        # Match Flow
        match = super().IsFilterMatch(selectors.flow.filters)
        logger.debug("- Flow Filter Match =", match)
        if match == False: return match
        return True

    def SetupTestcaseConfig(self, obj):
        obj.flow = self
        obj.gft_flow = self.__gft_flow
        obj.src.tenant = self.__sten
        obj.dst.tenant = self.__dten
        obj.src.segment = self.__sseg
        obj.dst.segment = self.__dseg
        obj.src.endpoint = self.__sep
        if self.__sep:
            obj.src.intf = self.__sep.GetInterface()
            if self.__sep.remote == False:
                obj.src.lif = obj.src.intf.lif
        obj.src.l4lb = self.__sfep.l4lb
        obj.dst.endpoint = self.__dep
        if self.__dep:
            obj.dst.intf = self.__dep.GetInterface()
            if self.__dep.remote == False:
                obj.dst.lif = obj.dst.intf.lif
        obj.dst.l4lb = self.__dfep.l4lb
        obj.ingress_mirror.session1 = self.GetIngressMirrorSession(idx = 1)
        obj.ingress_mirror.session2 = self.GetIngressMirrorSession(idx = 2)
        obj.ingress_mirror.session3 = self.GetIngressMirrorSession(idx = 3)
        obj.egress_mirror.session1 = self.GetEgressMirrorSession(idx = 1)
        obj.egress_mirror.session2 = self.GetEgressMirrorSession(idx = 2)
        obj.egress_mirror.session3 = self.GetEgressMirrorSession(idx = 3)
        return

    def __show_testcase_config(self, obj):
        logger.info("  - Ten : %s" % obj.tenant.Summary())
        logger.info("  - Seg : %s" % obj.segment.Summary())
        if obj.endpoint:
            logger.info("  - EP  : %s" % obj.endpoint.Summary())
            logger.info("  - Intf: %s" % obj.endpoint.intf.Summary())
        if obj.l4lb.IsEnabled():
            logger.info("  - Srvc: %s" % obj.l4lb.service.Summary())
            logger.info("  - Bknd: %s" % obj.l4lb.backend.Summary())
        return

    def ShowTestcaseConfig(self, obj):
        logger.info("Testcase Config Objects:")
        logger.info("- Flow  : %s" % obj.flow.Summary())
        if obj.gft_flow:
            logger.info("- GftFlow: %s" % obj.gft_flow.Summary())
        logger.info("- Source:")
        self.__show_testcase_config(obj.src)
        logger.info("- Destination:")
        self.__show_testcase_config(obj.dst)
        if self.IsMulticast():
            self.__dfep.ShowTestcaseConfig()
        return
