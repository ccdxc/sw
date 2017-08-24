#! /usr/bin/python3

import pdb
import copy

import infra.common.defs            as defs
import infra.common.objects         as objects
import infra.common.utils           as utils
import infra.config.base            as base

import config.resmgr                as resmgr
import config.objects.flow          as flow
import config.objects.flow_endpoint as flowep
import config.objects.endpoint      as endpoint

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api            as halapi
import config.hal.defs           as haldefs

class SessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SESSION'))
        return

    def Init(self, initiator, responder, label,
             initiator_span, responder_span):
        self.id = resmgr.SessionIdAllocator.get()
        self.GID("Session%d" % self.id)
        self.initiator = initiator
        self.initiator_span = initiator_span
        self.responder_span = responder_span
        self.responder = responder
        self.label = label.upper()
        
        assert(initiator.proto == responder.proto)
        self.proto = initiator.proto

        self.iflow = flow.FlowObject(self, self.initiator,
                                     self.responder, 'IFLOW', 
                                     self.label, self.initiator_span)
        self.rflow = flow.FlowObject(self, self.responder,
                                     self.initiator, 'RFLOW', 
                                     self.label, self.responder_span)

        cfglogger.info("Created Session with GID:%s" % self.GID())
        cfglogger.info("  - Label           : %s" % self.label)
        cfglogger.info("  - Initiator       : %s" % self.initiator)
        cfglogger.info("  - Responder       : %s" % self.responder)
        cfglogger.info("  - Initiator Flow  : %s" % self.iflow)
        cfglogger.info("  - Responder Flow  : %s" % self.rflow)
        return

    def PrepareHALRequestSpec(self, req_spec):
        cfglogger.info("Configuring Session with GID:%s" % self.GID())
        cfglogger.info("  - Initiator       : %s" % self.initiator)
        cfglogger.info("  - Responder       : %s" % self.responder)

        req_spec.meta.tenant_id = self.initiator.ep.tenant.id

        req_spec.session_id = self.id
        req_spec.conn_track_en = False

        self.iflow.PrepareHALRequestSpec(req_spec.initiator_flow)
        self.rflow.PrepareHALRequestSpec(req_spec.responder_flow)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("  - Session %s = %s" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status)))

        self.hal_handle = resp_spec.status.session_handle
        return

    def IsFilterMatch(self, config_filter):
        cfglogger.debug("Matching Session %s" % self.GID())
        # Match on Initiator Flow
        config_filter.flow = config_filter.session.iflow
        match = self.iflow.IsFilterMatch(config_filter)
        cfglogger.debug("- IFlow Filter Match =", match)
        if match == False: return match
        # Match on Responder Flow
        config_filter.flow = config_filter.session.rflow
        match = self.rflow.IsFilterMatch(config_filter)
        cfglogger.debug("- RFlow Filter Match =", match)
        if match == False: return match
        # Match on the Session
        match = super().IsFilterMatch(config_filter.session.base.filters)
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
        self.objlist = []
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
            assert(spec)
            if spec.proto:
                proto = spec.proto.upper()
                flowep1.proto = proto
                flowep2.proto = proto
            else:
                flowep1.proto = None
                flowep2.proto = None

            for t in spec.entries:
                self.__pre_process_spec_entry(t.entry)
                flowep1.SetInfo(t.entry.initiator)
                flowep2.SetInfo(t.entry.responder)
                session = SessionObject()
                session.Init(copy.copy(flowep1),
                             copy.copy(flowep2),
                             t.entry.label,
                             t.entry.initiator.span,
                             t.entry.responder.span)
                self.objlist.append(session)
        return
            
    def __process_ipv6(self, flowep1, flowep2, entries):
        flowep1.type = 'IPV6'
        flowep2.type = 'IPV6'

        flowep1.dom = flowep1.ep.tenant.id
        flowep2.dom = flowep2.ep.tenant.id
        for addr1 in flowep1.ep.ipv6addrs:
            for addr2 in flowep2.ep.ipv6addrs:
                flowep1.addr = addr1
                flowep2.addr = addr2
                self.__process_session_specs(flowep1, flowep2, entries)
        return


    def __process_ipv4(self, flowep1, flowep2, entries):
        flowep1.type = 'IPV4'
        flowep2.type = 'IPV4'

        flowep1.dom = flowep1.ep.tenant.id
        flowep2.dom = flowep2.ep.tenant.id

        for addr1 in flowep1.ep.ipaddrs:
            for addr2 in flowep2.ep.ipaddrs:
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
            entries = tenant.spec.sessions.unidest.ipv4
            self.__process_ipv4(flowep1, flowep2, entries)

        if tenant.spec.sessions.unidest.ipv6:
            entries = tenant.spec.sessions.unidest.ipv6
            self.__process_ipv6(flowep1, flowep2, entries)

        if tenant.spec.sessions.unidest.mac:
            entries = tenant.spec.sessions.unidest.mac
            self.__process_mac(flowep1, flowep2, entries)
        return


    def __process_ep_pair(self, ep1, ep2):
        self.__process_unidest(ep1, ep2)
        self.__process_multidest(ep2, ep2)
        return

    def Generate(self):
        ep1_list = Store.objects.GetAllByClass(endpoint.EndpointObject)
        ep2_list = ep1_list
        for ep1 in ep1_list:
            for ep2 in ep2_list:
                if ep1 == ep2: continue
                if ep1.tenant != ep2.tenant: continue
                self.__process_ep_pair(ep1, ep2)
        return

    def Configure(self):
        cfglogger.info("Configuring %d Sessions." % len(self.objlist)) 
        halapi.ConfigureSessions(self.objlist)
        return

    def main(self):
        self.Generate()
        self.Configure()
        Store.objects.SetAll(self.objlist)
        return

    def GetAll(self):
        return self.objlist

    def __get_matching_sessions(self, config_filter = None):
        ssns = []
        for ssn in self.objlist:
            if ssn.IsFilterMatch(config_filter):
                ssns.append(ssn)
        if config_filter.maxsessions == None:
            return ssns
        if config_filter.maxsessions >= len(ssns):
            return ssns
        return ssns[:config_filter.maxsessions]

    def __get_matching_flows(self, config_filter = None):
        flows = []
        for ssn in self.objlist:
            if ssn.iflow.IsFilterMatch(config_filter):
                flows.append(ssn.iflow)
            if ssn.rflow.IsFilterMatch(config_filter):
                flows.append(ssn.rflow)
        if config_filter.maxflows == None:
            return flows
        if config_filter.maxflows >= len(flows):
            return flows
        return flows[:config_filter.maxflows]

    def GetMatchingConfigObjects(self, config_filter = None):
        if config_filter.IsFlowBased():
            return self.__get_matching_flows(config_filter)
        elif config_filter.IsSessionBased():
            return self.__get_matching_sessions(config_filter)
        
        cfglogger.error("INVALID Config Filter in testspec.")
        return []

SessionHelper = SessionObjectHelper()
