#! /usr/bin/python3

import pdb
import copy

import infra.common.defs            as defs
import infra.common.objects         as objects
import infra.common.utils           as utils
import infra.common.timeprofiler    as timeprofiler
import infra.config.base            as base

import config.resmgr                  as resmgr
import config.objects.flow            as flow
import config.objects.flow_endpoint   as flowep
import config.objects.endpoint        as endpoint
import config.objects.tenant          as tenant
import config.objects.collector       as collector
import config.objects.l4lb            as l4lb
import config.objects.multicast_group as multicast_group

from config.store                       import Store
from infra.common.logging               import cfglogger
from infra.common.glopts                import GlobalOptions as GlobalOptions
from config.objects.proxycb_service     import ProxyCbServiceHelper

import config.hal.api            as halapi
import config.hal.defs           as haldefs

class SessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SESSION'))
        return

    def Init(self, initiator, responder, spec):
        self.id = resmgr.SessionIdAllocator.get()
        self.GID("Session%d" % self.id)
        self.__pre_process_spec(spec)
        self.conn_track_en = getattr(self.spec, 'tracking', False)
        self.tcp_ts_option = getattr(self.spec, 'timestamp', False)
        self.tcp_sack_perm_option = getattr(self.spec, 'sack_perm', False)

        self.fte = getattr(spec, 'fte', False)
        self.multicast = getattr(spec, 'multicast', False)
        self.initiator = flowep.FlowEndpointObject(srcobj = initiator)
        self.__process_initiator_spec()
        
        self.responder = flowep.FlowEndpointObject(srcobj = responder)
        self.__process_responder_spec()

        self.type = self.initiator.type
        self.label = getattr(self.spec, 'label', '')
        self.label = self.label.upper()

        assert(initiator.proto == responder.proto)
        self.proto = initiator.proto

        self.iflow = flow.FlowObject(self, self.initiator, self.responder,
                                     'IFLOW', self.label,
                                     getattr(self.ispec, 'span', None))
        self.iflow.SetMulticast(self.multicast)
        
        self.rflow = None
        if self.multicast is False:
            self.rflow = flow.FlowObject(self, self.responder, self.initiator,
                                         'RFLOW', self.label,
                                         getattr(self.rspec, 'span', None))
            self.rflow.SetMulticast(self.multicast)
        self.Show()
        return defs.status.SUCCESS

    def __pre_process_spec(self, spec):
        self.spec = spec
        self.ispec = getattr(self.spec, 'initiator', None)
        self.rspec = getattr(self.spec, 'responder', None)
        return

    def __process_initiator_spec(self):
        if self.ispec is None: return
        self.initiator.SetInfo(self.ispec)
        return

    def __process_responder_spec(self):
        if self.rspec is None: return
        self.responder.SetInfo(self.rspec)
        return

    def SetFteEnabled(self):
        self.fte = True

    def IsFteEnabled(self):
        return self.fte

    def SetLabel(self, label, update_iflow=True, update_rflow=True):
        cfglogger.info("Updating %s Label to %s" % (self.GID(), label))
        self.label = label
        if update_iflow:
            self.iflow.SetLabel(label)
        if update_rflow:
            self.rflow.SetLabel(label)
        return

    def __get_span_lens(self, spec):
        ingspanlen = 0
        egspanlen = 0
        span = getattr(spec, 'span', None)
        if span is None: return(0,0)
        ing = getattr(span, 'ingress', None)
        if ing is not None: ingspanlen = len(ing)
        egr = getattr(span, 'egress', None)
        if egr is not None: espanlen = len(egr)
        return (ingspanlen, egspanlen)

    def Show(self):
        cfglogger.info("Created Session with GID:%s" % self.GID())
        cfglogger.info("- Label    : %s" % self.label)
        string = None
        if self.IsTCP():
            tr = getattr(self.spec, 'tracking', False)
            if tr: string = 'Tracking'
            ts = getattr(self.spec, 'timestamp', False)
            if ts: string += '/Timestamp'

        if string:
           cfglogger.info("- Info     : %s" % string)

        self.initiator.Show('Initiator')
        self.iflow.Show()

        if self.multicast is False:
            self.responder.Show('Responder')
            self.rflow.Show()
        
        ilen,elen = self.__get_span_lens(self.ispec)
        cfglogger.info(" Initatior SPAN sessions : %d/%d" % (ilen, elen))
        ilen,elen = self.__get_span_lens(self.rspec)
        cfglogger.info(" Responder SPAN sessions : %d/%d" % (ilen, elen))
        return

    def __copy__(self):
        session = SessionObject()
        session.id = self.id
        session.hal_handle = self.hal_handle
        session.conn_track_en = self.conn_track_en
        session.tcp_ts_option = self.tcp_ts_option
        session.tcp_sack_perm_option = self.tcp_sack_perm_option
        session.iflow = copy.copy(self.iflow)
        session.rflow = copy.copy(self.rflow)

        return session

    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False

        fields = ["id", "hal_handle", "tcp_ts_option", "conn_track_en", "tcp_sack_perm_optionr"]
        if not self.CompareObjectFields(other, fields, lgh):
            return False

        if not self.iflow.Equals(other.iflow, lgh) or not self.rflow.Equals(other.rflow, lgh):
            return False

        return True

    def Get(self):
        halapi.GetSessions([self])

    def IsTCP(self):
        return self.initiator.IsTCP()
    def IsUDP(self):
        return self.initiator.IsUDP()
    def IsICMP(self):
        return self.initiator.IsICMP()
    def IsICMPV6(self):
        return self.initiator.IsICMPV6()
    def IsIP(self):
        return self.initiator.IsIP()
    def IsIPV4(self):
        return self.initiator.IsIPV4()
    def IsIPV6(self):
        return self.initiator.IsIPV6()
    def IsMAC(self):
        return self.initiator.IsMAC()

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.vrf_id = self.initiator.ep.tenant.id

        req_spec.session_id = self.id
        if self.IsTCP():
            req_spec.conn_track_en = getattr(self.spec, 'tracking', False)
            req_spec.tcp_ts_option = getattr(self.spec, 'timestamp', False)
            req_spec.tcp_sack_perm_option = getattr(self.spec, 'sack_perm', False)
        else:
            req_spec.conn_track_en = False
            req_spec.tcp_ts_option = False
            req_spec.tcp_sack_perm_option = False

        self.iflow.PrepareHALRequestSpec(req_spec.initiator_flow)
        self.rflow.PrepareHALRequestSpec(req_spec.responder_flow)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("Configuring Session with GID:%s" % self.GID())
        self.iflow.ProcessHALResponse(req_spec.initiator_flow,
                                      resp_spec.status.iflow_status)
        self.rflow.ProcessHALResponse(req_spec.responder_flow,
                                      resp_spec.status.rflow_status)

        self.hal_handle = resp_spec.status.session_handle

        if self.iflow.IsCollisionFlow():
            self.label = self.iflow.label
        elif self.rflow.IsCollisionFlow():
            self.label = self.rflow.label
        cfglogger.info("  - Session %s = %s(HDL = %x), Label = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle, self.label))

        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.meta.vrf_id = self.initiator.ep.tenant.id
        get_req_spec.session_handle = self.hal_handle
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        if get_resp.api_status == haldefs.common.ApiStatus.Value('API_STATUS_OK'):
            self.id = get_resp.spec.session_id
            self.tcp_ts_option = get_resp.spec.tcp_ts_option
            self.conn_track_en = get_resp.spec.conn_track_en
            self.tcp_sack_perm_option = get_resp.spec.tcp_sack_perm_option
        else:
            self.id = None
            self.tcp_ts_option = None
            self.conn_track_en = None
            self.tcp_sack_perm_option = None

        self.iflow.ProcessHALGetResponse(get_req_spec, get_resp.spec.initiator_flow)
        self.rflow.ProcessHALGetResponse(get_req_spec, get_resp.spec.responder_flow)
        return

    def IsFilterMatch(self, selectors):
        cfglogger.debug("Matching Session %s" % self.GID())
        
        # Match on Initiator Flow
        selectors.SetFlow(selectors.session.iflow)
        match = self.iflow.IsFilterMatch(selectors)
        cfglogger.debug("- IFlow Filter Match =", match)
        if match == False: return match
        
        # Match on Responder Flow
        # Swap Src/Dst selectors for Rflow match.
        selectors.SwapSrcDst()
        selectors.SetFlow(selectors.session.rflow)
        match = self.rflow.IsFilterMatch(selectors)
        selectors.SwapSrcDst()
        cfglogger.debug("- RFlow Filter Match =", match)
        if match == False: return match
        
        # Match on the Session
        match = super().IsFilterMatch(selectors.session.base.filters)
        cfglogger.debug("- Session Filter Match =", match)
        return match

    def SetupTestcaseConfig(self, obj):
        self.iflow.SetupTestcaseConfig(obj.session.iconfig)
        self.rflow.SetupTestcaseConfig(obj.session.rconfig)
        obj.root = self
        return

    def ShowTestcaseConfig(self, obj, logger):
        logger.info("Config Objects for %s" % self.GID())
        self.iflow.ShowTestcaseConfig(obj.session.iconfig, logger)
        self.rflow.ShowTestcaseConfig(obj.session.rconfig, logger)
        return

class SessionObjectHelper:
    def __init__(self):
        self.objs = []
        self.classicssns = []
        self.ftessns = []
        self.ssns = []
        self.fep_pairs = {}
        return

    def __get_fep_pair_key(self, fep1, fep2):
        gid1 = fep1.ep.GID()
        gid2 = fep2.ep.GID()
        key = "%s__%s__TO__%s__%s" % (fep1.type, gid1, gid2, fep1.proto)
        return key

    def __add_fep_pair(self, fep1, fep2):
        key = self.__get_fep_pair_key(fep1, fep2)
        self.fep_pairs[key] = True
        return

    def __is_fep_pair_done(self, fep1, fep2):
        key1 = self.__get_fep_pair_key(fep1, fep2)
        key2 = self.__get_fep_pair_key(fep2, fep1)
        if key1 in self.fep_pairs or key2 in self.fep_pairs:
            return True
        return False

    def __skip_fep_pair(self, flowep1, flowep2):
        if flowep1.IsTCP() or flowep1.IsUDP() or\
           flowep1.IsICMP() or flowep1.IsICMPV6() or\
           flowep1.IsESP():
           return False

        if self.__is_fep_pair_done(flowep1, flowep2):
            return True
        return False

    def __process_multicast_group(self, ep1, group):
        tenant = ep1.tenant
        flowep1 = flowep.FlowEndpointObject(ep = ep1)
        flowep1.InitMulticastSourceFlowEndpoint(group)

        flowep2 = flowep.FlowEndpointObject(group = group)

        session = SessionObject()
        session.Init(flowep1, flowep2, group.session_spec)

        self.objs.append(session)
        if session.IsFteEnabled():
            cfglogger.info("Adding Session:%s to FTE Session List" % session.GID())
            self.ftessns.append(session)
        elif GlobalOptions.classic:
            cfglogger.info("Adding Session:%s to Classic Session List" % session.GID())
            self.classicssns.append(session)
        else:
            cfglogger.info("Adding Session:%s to NON-FTE Session List" % session.GID())
            self.ssns.append(session)
        return

    def __process_multicast(self, ep_list):
        groups = Store.objects.GetAllByClass(multicast_group.MulticastGroupObject)
        for ep in ep_list:
            for g in groups:
                if ep.tenant != g.segment.tenant: continue
                if ep.segment != g.segment: continue
                self.__process_multicast_group(ep, g)
        return

    def __pre_process_spec_entry(self, entry):
        if 'span' not in entry.initiator.__dict__:
            entry.initiator.span = None
        if 'span' not in entry.responder.__dict__:
            entry.responder.span = None
        return

    def __process_session_specs(self, flowep1, flowep2, refs):
        for ref in refs:
            spec = ref.Get(Store)

            proto = spec.proto.upper() if spec.proto else None

            if flowep1.IsProtoMatch(proto) == False: continue
            if flowep2.IsProtoMatch(proto) == False: continue

            flowep1.proto = proto
            flowep2.proto = proto
            if self.__skip_fep_pair(flowep1, flowep2):
                return

            self.__add_fep_pair(flowep1, flowep2)
            for t in spec.entries:
                self.__pre_process_spec_entry(t.entry)
                session = SessionObject()
                status = session.Init(flowep1, flowep2, t.entry)
                if status != defs.status.SUCCESS:
                    continue
                self.objs.append(session)
                if session.IsFteEnabled():
                    cfglogger.info("Adding Session:%s to FTE Session List" % session.GID())
                    self.ftessns.append(session)
                elif GlobalOptions.classic:
                    cfglogger.info("Adding Session:%s to Classic Session List" % session.GID())
                    self.classicssns.append(session)
                else:
                    cfglogger.info("Adding Session:%s to NON-FTE Session List" % session.GID())
                    self.ssns.append(session)
        return

    def __process_ipv6(self, flowep1, flowep2, entries):
        flowep1.type = 'IPV6'
        flowep2.type = 'IPV6'

        flowep1.dom = flowep1.GetTenantId()
        flowep2.dom = flowep2.GetTenantId()
        for addr1 in flowep1.GetIpv6Addrs():
            for addr2 in flowep2.GetIpv6Addrs():
                flowep1.addr = addr1
                flowep2.addr = addr2
                self.__process_session_specs(flowep1, flowep2, entries)
        return

    def __process_ipv4(self, flowep1, flowep2, entries):
        flowep1.type = 'IPV4'
        flowep2.type = 'IPV4'

        flowep1.dom = flowep1.GetTenantId()
        flowep2.dom = flowep2.GetTenantId()

        for addr1 in flowep1.GetIpAddrs():
            for addr2 in flowep2.GetIpAddrs():
                flowep1.addr = addr1
                flowep2.addr = addr2
                self.__process_session_specs(flowep1, flowep2, entries)
        return

    def __process_mac(self, flowep1, flowep2, entries):
        flowep1.type = 'MAC'
        flowep2.type = 'MAC'
        seg1 = flowep1.ep.segment
        seg2 = flowep2.ep.segment

        if seg1 != seg2: return

        flowep1.dom = flowep1.ep.segment.id
        flowep2.dom = flowep2.ep.segment.id

        flowep1.addr = flowep1.ep.macaddr
        flowep2.addr = flowep2.ep.macaddr
        self.__process_session_specs(flowep1, flowep2, entries)
        return

    def __process_unidest(self, ep1, ep2):
        ten1 = ep1.tenant
        ten2 = ep2.tenant

        assert(ten1 == ten2)
        tenant = ten1

        flowep1 = flowep.FlowEndpointObject(ep = ep1)
        flowep2 = flowep.FlowEndpointObject(ep = ep2)

        if tenant.spec.sessions == None:
            return

        if tenant.spec.sessions.unidest.ipv4:
            self.__process_ipv4(flowep1, flowep2,
                                tenant.spec.sessions.unidest.ipv4)

        if tenant.spec.sessions.unidest.ipv6:
            self.__process_ipv6(flowep1, flowep2,
                                tenant.spec.sessions.unidest.ipv6)

        if tenant.spec.sessions.unidest.mac:
            self.__process_mac(flowep1, flowep2,
                               tenant.spec.sessions.unidest.mac)
        return


    def __process_ep_pair(self, ep1, ep2):
        if ep1.IsL4LbBackend() or ep2.IsL4LbBackend(): return
        self.__process_unidest(ep1, ep2)
        return

    def __get_eps(self):
        eps = []
        tenants = Store.objects.GetAllByClass(tenant.TenantObject)
        for t in tenants:
            eps += t.GetEps()
        return eps

    def __process_l4lb_service(self, ep, svc):
        tenant = ep.tenant
        if tenant.IsL4LbEnabled() == False:
            return

        # Twice NAT disabled for now.
        #if svc.IsTwiceNAT():
        #    return

        l4lbspec = getattr(tenant.spec.sessions, 'l4lb', None)
        if l4lbspec is None:
            return


        flowep1 = flowep.FlowEndpointObject(ep = ep)
        flowep2 = flowep.FlowEndpointObject(l4lbsvc = svc)


        # L2 DSR: Create only Sessions only in one L2 Segment
        if svc.IsNATDSR() and (flowep1.GetSegment() != flowep2.GetSegment()):
            return

        if l4lbspec.ipv4:
            self.__process_ipv4(flowep1, flowep2, l4lbspec.ipv4)
        if l4lbspec.ipv6:
            self.__process_ipv6(flowep1, flowep2, l4lbspec.ipv6)
        return

    def __process_l4lb(self, ep_list):
        svcs = Store.objects.GetAllByClass(l4lb.L4LbServiceObject)
        for ep in ep_list:
            for svc in svcs:
                if ep.tenant != svc.tenant: continue
                self.__process_l4lb_service(ep, svc)
        return

    def Generate(self):
        ep_list = self.__get_eps()

        # Generate L4LbService Flows for each EP.
        self.__process_l4lb(ep_list)

        # Generate Multicast Flows
        self.__process_multicast(ep_list)

        for ep1 in ep_list:
            for ep2 in ep_list:
                if ep1 == ep2: continue
                if ep1.tenant != ep2.tenant: continue
                self.__process_ep_pair(ep1, ep2)
        self.__add_collector_sessions()
        return

    def Configure(self):
        cfglogger.info("Configuring %d NON-FTE Sessions." % len(self.ssns))
        if len(self.ssns):
            halapi.ConfigureSessions(self.ssns)
        return

    def main(self):
        self.Generate()
        ProxyCbServiceHelper.main(self.objs)
        self.Configure()
        if len(self.objs):
            Store.objects.SetAll(self.objs)
        return

    def GetAll(self):
        return self.objs

    def GetAllMatchingLabel(self, label):
        return [s for s in self.objs if s.label == label]

    def __get_matching_sessions(self, selectors = None):
        ssns = []
        for ssn in self.objs:
            if ssn.IsFilterMatch(selectors):
                ssns.append(ssn)
        if selectors.maxsessions == None:
            return ssns
        if selectors.maxsessions >= len(ssns):
            return ssns
        return ssns[:selectors.maxsessions]

    def __get_matching_flows(self, selectors = None):
        timeprofiler.SelectorProfiler.Start()
        flows = []
        for ssn in self.objs:
            if ssn.iflow and ssn.iflow.IsFilterMatch(selectors):
                flows.append(ssn.iflow)
            if ssn.rflow and ssn.rflow.IsFilterMatch(selectors):
                flows.append(ssn.rflow)
        if selectors.maxflows is None:
            return flows
        if selectors.maxflows >= len(flows):
            return flows
        objs = flows[:selectors.maxflows]
        timeprofiler.SelectorProfiler.Stop()
        return objs

    def GetMatchingConfigObjects(self, selectors = None):
        if selectors.IsFlowBased():
            objs = self.__get_matching_flows(selectors)
        elif selectors.IsSessionBased():
            objs = self.__get_matching_sessions(selectors)
        else:
            objs = []
            cfglogger.error("INVALID Config Filter in testspec.")
        return objs

    def __add_collector_sessions(self):
        #pdb.set_trace()
        for ep1, ep2, spec in collector.CollectorHelper.GetCollectorSessions():
            ten1 = ep1.tenant
            ten2 = ep2.tenant

            assert(ten1 == ten2)
            tenant = ten1

            flowep1 = flowep.FlowEndpointObject(ep = ep1)
            flowep2 = flowep.FlowEndpointObject(ep = ep2)

            self.__process_ipv4(flowep1, flowep2, spec)

SessionHelper = SessionObjectHelper()

def GetMatchingObjects(selectors):
    sessions =  Store.objects.GetAllByClass(SessionObject)
    return [session for session in sessions if session.IsFilterMatch(selectors)]
