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
import config.objects.l4lb            as l4lb

from config.store                      import Store
from infra.common.logging              import cfglogger
from config.objects.proxycb_service    import ProxyCbServiceHelper

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
        self.spec = spec

        self.fte = getattr(spec, 'fte', False)
        self.initiator = flowep.FlowEndpointObject(srcobj = initiator)
        self.initiator.SetInfo(spec.initiator)
        status = self.initiator.SelectL4LbBackend()
        if status != defs.status.SUCCESS: return status
        
        self.responder = flowep.FlowEndpointObject(srcobj = responder)
        self.responder.SetInfo(spec.responder)
        status = self.responder.SelectL4LbBackend()
        if status != defs.status.SUCCESS: return status

        
        self.type = self.initiator.type	
        self.label = self.spec.label.upper()
        
        assert(initiator.proto == responder.proto)
        self.proto = initiator.proto

        self.iflow = flow.FlowObject(self, self.initiator, self.responder,
                                     'IFLOW', self.label,
                                     self.spec.initiator.span)
        self.rflow = flow.FlowObject(self, self.responder, self.initiator,
                                     'RFLOW', self.label,
                                     self.spec.responder.span)
        self.Show()
        return defs.status.SUCCESS

    def IsFteEnabled(self):
        return self.fte

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
        if self.iflow.in_qos:
            cfglogger.info("  -   Ingress QoS   : CosRW=%s/Cos=%d, DscpRW=%s/Dscp=%d" %\
                           (self.iflow.in_qos.cos_rw.get(), self.iflow.in_qos.cos.get(),
                            self.iflow.in_qos.dscp_rw.get(), self.iflow.in_qos.dscp.get()))
        if self.iflow.eg_qos:
            cfglogger.info("  -   Eggress QoS   : CosRW=%s/Cos=%d, DscpRW=%s/Dscp=%d" %\
                           (self.iflow.eg_qos.cos_rw.get(), self.iflow.eg_qos.cos.get(),
                            self.iflow.eg_qos.dscp_rw.get(), self.iflow.eg_qos.dscp.get()))
        self.responder.Show('Responder')
        self.rflow.Show()
        if self.rflow.in_qos:
            cfglogger.info("  -   Ingress QoS   : CosRW=%s/Cos=%d, DscpRW=%s/Dscp=%d" %\
                           (self.rflow.in_qos.cos_rw.get(), self.rflow.in_qos.cos.get(),
                            self.rflow.in_qos.dscp_rw.get(), self.rflow.in_qos.dscp.get()))
        if self.rflow.eg_qos:
            cfglogger.info("  -   Eggress QoS   : CosRW=%s/Cos=%d, DscpRW=%s/Dscp=%d" %\
                           (self.rflow.eg_qos.cos_rw.get(), self.rflow.eg_qos.cos.get(),
                            self.rflow.eg_qos.dscp_rw.get(), self.rflow.eg_qos.dscp.get()))
        ingspanlen = 0
        egspanlen = 0
        if self.spec.initiator.span != None and 'ingress' in self.spec.initiator.span.__dict__:
            ingspanlen = len(self.spec.initiator.span.ingress)
        if self.spec.initiator.span != None and 'egress' in self.spec.initiator.span.__dict__:
            espanlen = len(self.spec.initiator.span.egress)
        cfglogger.info(" Initatior SPAN sessions : %d/%d" % (ingspanlen, egspanlen))
        ingspanlen = 0
        egspanlen = 0
        if self.spec.responder.span != None and 'ingress' in self.spec.responder.span.__dict__:
            ingspanlen = len(self.spec.responder.span.ingress)
        if self.spec.responder.span != None and 'egress' in self.spec.responder.span.__dict__:
            espanlen = len(self.spec.responder.span.egress)
        cfglogger.info(" Responder SPAN sessions : %d/%d" % (ingspanlen, egspanlen))
        return

    def IsTCP(self):
        return self.initiator.IsTCP()

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.tenant_id = self.initiator.ep.tenant.id

        req_spec.session_id = self.id
        if self.IsTCP():
            req_spec.conn_track_en = getattr(self.spec, 'tracking', False)
            req_spec.tcp_ts_option = getattr(self.spec, 'timestamp', False)
        else:
            req_spec.conn_track_en = False
            req_spec.tcp_ts_option = False

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

    def IsFilterMatch(self, selectors):
        cfglogger.debug("Matching Session %s" % self.GID())
        # Match on Initiator Flow
        selectors.flow = selectors.session.iflow
        match = self.iflow.IsFilterMatch(selectors)
        cfglogger.debug("- IFlow Filter Match =", match)
        if match == False: return match
        # Match on Responder Flow
        selectors.flow = selectors.session.rflow
        match = self.rflow.IsFilterMatch(selectors)
        cfglogger.debug("- RFlow Filter Match =", match)
        if match == False: return match
        # Match on the Session
        match = super().IsFilterMatch(selectors.session.base.filters)
        cfglogger.debug("- Session Filter Match =", match)
        return match

    def SetupTestcaseConfig(self, obj):
        self.iflow.SetupTestcaseConfig(obj.session.iconfig)
        self.rflow.SetupTestcaseConfig(obj.session.rconfig)
        return

    def ShowTestcaseConfig(self, obj, logger):
        logger.info("Config Objects for %s" % self.GID())
        self.iflow.ShowTestcaseConfig(obj.session.iconfig, logger)
        self.rflow.ShowTestcaseConfig(obj.session.rconfig, logger)
        return

class SessionObjectHelper:
    def __init__(self):
        self.objs = []
        self.ftessns = []
        self.ssns = []
        self.gid_pairs = {}
        return

    def __add_gid_pair(self, gid1, gid2):
        key = "%s__TO__%s" % (gid1, gid2)
        self.gid_pairs[key] = True
        return

    def __is_gid_pair_done(self, gid1, gid2):
        key1 = "%s__TO__%s" % (gid1, gid2)
        key2 = "%s__TO__%s" % (gid2, gid1)
        if key1 in self.gid_pairs or key2 in self.gid_pairs:
            return True
        return False

    def __process_multidest(self, src_ep, dst_ep):
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

            if flowep1.IsProtoMatch(proto) == False: return
            if flowep2.IsProtoMatch(proto) == False: return
            
            flowep1.proto = proto
            flowep2.proto = proto

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
        if self.__is_gid_pair_done(flowep1.ep.GID(), flowep2.ep.GID()): return

        flowep1.dom = flowep1.ep.segment.id
        flowep2.dom = flowep2.ep.segment.id
        
        flowep1.addr = flowep1.ep.macaddr
        flowep2.addr = flowep2.ep.macaddr
        self.__process_session_specs(flowep1, flowep2, entries)
        self.__add_gid_pair(flowep1.ep.GID(), flowep2.ep.GID())
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
        self.__process_multidest(ep2, ep2)
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
        if svc.IsTwiceNAT():
            return

        flowep1 = flowep.FlowEndpointObject(ep = ep)
        flowep2 = flowep.FlowEndpointObject(l4lbsvc = svc)
        if tenant.spec.sessions.l4lb.ipv4:
            self.__process_ipv4(flowep1, flowep2,
                                tenant.spec.sessions.l4lb.ipv4)
        if tenant.spec.sessions.l4lb.ipv6:
            self.__process_ipv6(flowep1, flowep2,
                                tenant.spec.sessions.l4lb.ipv6)
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

        for ep1 in ep_list:
            for ep2 in ep_list:
                if ep1 == ep2: continue
                if ep1.tenant != ep2.tenant: continue
                self.__process_ep_pair(ep1, ep2)
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
            if ssn.iflow.IsFilterMatch(selectors):
                flows.append(ssn.iflow)
            if ssn.rflow.IsFilterMatch(selectors):
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

SessionHelper = SessionObjectHelper()
