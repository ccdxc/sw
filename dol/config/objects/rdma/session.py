#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
from config.store               import Store
from infra.common.logging       import logger

import config.hal.api           as halapi
import config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2

import config.objects.qp        as qp
import config.objects.mr        as mr
from infra.common.glopts        import GlobalOptions

from config.objects.session     import SessionHelper

from scapy.utils import inet_aton, inet_ntoa, inet_pton
import struct
import socket

class RdmaSessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('RDMA_SESSION'))
        return

    def Init(self, session, lqp, rqp, vxlan):
        self.id = resmgr.RdmaSessionIdAllocator.get()
        self.GID("RdmaSession%d" % self.id)
        self.lqp = lqp
        self.rqp = rqp
        self.ah_handle = 0
        self.ah_size = 0
        self.session = session
        self.IsIPV6 = session.IsIPV6()
        self.IsVXLAN = vxlan

    def Configure(self):
        self.lqp.ConfigureHeaderTemplate(self, self.session.initiator,
                                         self.session.responder,
                                         self.session.iflow,
                                         self.session.IsIPV6(), True)
        self.lqp.set_dst_qp(self.rqp.id)

        # For local-local rdma sessions, need to setup reverse direction too
        if not self.rqp.remote:
            self.rqp.ConfigureHeaderTemplate(self, self.session.responder,
                                         self.session.initiator,
                                         self.session.rflow,
                                         self.session.IsIPV6(), False)
            self.rqp.set_dst_qp(self.lqp.id)

        if self.lqp.svc == 3:
            self.lqp.set_q_key(self.lqp.id) #we are using q_id as q_key
        #self.lqp.rq.set_dst_qp(self.rqp.id)
        #logger.info('RDMA Session: %s  Setting SQCB Local QID: %d Remote QID: %d ' % 
        #               (self.GID(), self.lqp.id, self.rqp.id))
        pass
         
    def Show(self):
        logger.info('RDMA Session: %s' % self.GID())
        logger.info('- Session: %s' % self.session.GID())
        logger.info('- LQP: %s' % self.lqp.GID())
        logger.info('- RQP: %s' % self.rqp.GID())
        logger.info('- IsIPV6: %s' % self.IsIPV6())
        logger.info('- IsVxLAN: %s' % self.IsVxLAN)
        return

    def PrepareHALRequestSpec(self, req_spec):
        #
        # For now we only have Ah Requests. In future we need to make sure what kind
        # of spec it is.
        #
        logger.info("PrepareHALRequestSpec:: RDMA Session: %s Session: %s "
                       "Remote QP: %s Local QP: %s" %\
                        (self.GID(), self.session.GID(), self.rqp.GID(), self.lqp.GID()))
        if (GlobalOptions.dryrun): return

        req_spec.smac = bytes(self.session.initiator.ep.macaddr.getnum().to_bytes(6, 'little'))
        req_spec.dmac = bytes(self.session.responder.ep.macaddr.getnum().to_bytes(6, 'little'))
        if self.session.IsIPV6():
             req_spec.ethtype = 0x86dd
             req_spec.ip_ver = 6
             req_spec.ip_saddr.ip_af = haldefs.common.IP_AF_INET6
             req_spec.ip_saddr.v6_addr = self.session.initiator.addr.getnum().to_bytes(16, 'big')
             req_spec.ip_daddr.ip_af = haldefs.common.IP_AF_INET6
             req_spec.ip_daddr.v6_addr = self.session.responder.addr.getnum().to_bytes(16, 'big')
        else:
             req_spec.ethtype = 0x800
             req_spec.ip_ver = 4
             req_spec.ip_saddr.ip_af = haldefs.common.IP_AF_INET
             req_spec.ip_saddr.v4_addr = self.session.initiator.addr.getnum()
             req_spec.ip_daddr.ip_af = haldefs.common.IP_AF_INET
             req_spec.ip_daddr.v4_addr = self.session.responder.addr.getnum()

        req_spec.ip_tos = self.session.iflow.txqos.dscp
        req_spec.vlan = self.session.initiator.ep.intf.encap_vlan_id
        req_spec.vlan_pri = self.session.iflow.txqos.cos
        req_spec.vlan_cfi = 0
        req_spec.ip_ttl = 64
        req_spec.udp_sport = int(self.session.iflow.sport)
        req_spec.udp_dport = int(self.session.iflow.dport)
        
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("ProcessHALResponse:: RDMA Session: %s Session: %s "
                       "Remote QP: %s Local QP: %s ah_handle: %d, ah_size: %d" %\
                        (self.GID(), self.session.GID(), self.rqp.GID(), self.lqp.GID(), 
                         resp_spec.ah_handle, resp_spec.ah_size))

        self.ah_handle = resp_spec.ah_handle
        self.ah_size = resp_spec.ah_size
        return

    def IsIPV6(self):
        return self.IsIPV6

    def IsFilterMatch(self, selectors):
        logger.debug('Matching RDMA Session: %s' % self.GID())

        match = self.lqp.IsFilterMatch(selectors.rdmasession.lqp)
        logger.debug("- LQP Filter Match =", match)
        if match == False: return match
        
        match = self.rqp.IsFilterMatch(selectors.rdmasession.rqp)
        logger.debug("- RQP Filter Match =", match)
        if match == False: return  match
        
        if hasattr(selectors.rdmasession, 'base'):
            match = super().IsFilterMatch(selectors.rdmasession.base.filters)
            if match == False: return  match

        if hasattr(selectors.rdmasession, 'session'):
            match = super().IsFilterMatch(selectors.rdmasession.session.filters)
            logger.debug("- IsIPV6 Filter Match =", match)
            if match == False: return  match
        
        return True

    def SetupTestcaseConfig(self, obj):
        obj.rdmasession = self;
        return

    def ShowTestcaseConfig(self, obj):
        logger.info("Config Objects for %s" % self.GID())
        self.lqp.ShowTestcaseConfig(obj.rdmasession.lqp)
        self.rqp.ShowTestcaseConfig(obj.rdmasession.rqp)
        return

    def IsRetryEnabled(self):
        return False

class RdmaSessionObjectHelper:
    def __init__(self):
        self.rdma_sessions = []
        self.nw_sessions = []
        self.used_qps = []
        self.v4_non_vxlan_count = 0
        self.v6_non_vxlan_count = 0
        self.v4_vxlan_count = 0
        self.v6_vxlan_count = 0
        return

    def __get_qps_for_ep(self, ep):
        if hasattr(ep, 'pds'):
            pds = ep.pds.GetAll()
            for pd in pds:
                qps = pd.qps.GetAll()
                return qps
        else:
            return []

    def __get_perf_qps_for_ep(self, ep):
        if hasattr(ep, 'pds'):
            pds = ep.pds.GetAll()
            for pd in pds:
                qps = pd.perf_qps.GetAll()
                return qps
        else:
            return []

    def RCGenerate(self):
        self.nw_sessions = SessionHelper.GetAllMatchingLabel('RDMA')
        for nw_s in self.nw_sessions:

            ep1 = nw_s.initiator.ep
            ep2 = nw_s.responder.ep

            # lqp should come from a local endpoint
            if ep1.remote: continue
            if not ep2.remote: continue

            ep1_qps = self.__get_qps_for_ep(ep1)
            ep2_qps = self.__get_qps_for_ep(ep2)
            
            #logger.info("RDMA RC Generate: EP1 %s (%s, %d) EP2 %s (%s, %d) IPv6 %d VXLAN %d" %\
            #                (ep1.GID(), not ep1.remote, len(ep1_qps), ep2.GID(), not ep2.remote,
            #                len(ep2_qps), nw_s.IsIPV6(), ep2.segment.IsFabEncapVxlan()))

            # Select 8 total RC Sessions, for now hard code this session split
            # but later can be added to topo spec, so selction criterion can change between
            # different topologies
            # 2 : v4 & non-VXLAN
            # 2 : v6 & non-VXLAN
            # 1 : v4 & VXLAN (enable after testspec changes)
            # 1 : v6 & VXLAN (enable after testspec changes)
            # In case of Perf mode (GlobalOptions.perf is set), select them as below:
            # we would like to pick different QPs for perf tests, so just pick all v4 sessions and zero v6 sessions
            # we will need minimum 8 sessions to avoid critical section locking at stage 0 for a QP
            # 8 : v4 & non-VXLAN
            # 0 : v6 & non-VXLAN
            # 1 : v4 & VXLAN (enable after testspec changes)
            # 1 : v6 & VXLAN (enable after testspec changes)

            for lqp in ep1_qps:
                if lqp in self.used_qps: continue
                if not lqp.svc == 0 : continue
                for rqp in ep2_qps:
                    if not rqp.svc == 0 : continue
                    if rqp in self.used_qps: continue
                    vxlan = ep2.segment.IsFabEncapVxlan()
                    ipv6  = nw_s.IsIPV6()

                    if vxlan and not nw_s.iflow.IsL2():
                       #logger.info('SKIP: VXLAN Routed Sessions for now')
                       continue

                    #logger.info("RDMA RC CHECK: EP1 %s (%s) EP2 %s (%s) LQP %s RQP %s"
                    #               " IPv6 %d VXLAN %d" % (ep1.GID(), not ep1.remote, ep2.GID(),
                    #               not ep2.remote, lqp.GID(), rqp.GID(), nw_s.IsIPV6(),
                    #               ep2.segment.IsFabEncapVxlan()))

                    if not ipv6 and not vxlan:  # v4 non-vxlan
                       self.v4_non_vxlan_count += 1
                       if GlobalOptions.perf:
                           if self.v4_non_vxlan_count > 8: continue
                       else:
                           if self.v4_non_vxlan_count > 2: continue
                    elif ipv6 and not vxlan:    # v6 non-vxlan
                       self.v6_non_vxlan_count +=1
                       if GlobalOptions.perf:
                           if self.v6_non_vxlan_count > 0: continue
                       else:
                           if self.v6_non_vxlan_count > 2: continue
                    elif not ipv6 and vxlan:    # v4 vxlan
                       self.v4_vxlan_count += 1
                       if self.v4_vxlan_count > 2: continue
                    else:                       # v6 vxlan
                       self.v6_vxlan_count += 1
                       if self.v6_vxlan_count > 2: continue

                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    logger.info("RDMA RC PICKED: EP1 %s (%s) EP2 %s (%s) LQP %s RQP %s" 
                                   "IPv6 %d VXLAN %d" % (ep1.GID(), not ep1.remote, ep2.GID(),
                                   not ep2.remote, lqp.GID(), rqp.GID(), nw_s.IsIPV6(), 
                                   ep2.segment.IsFabEncapVxlan()))

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp, vxlan)
                    self.rdma_sessions.append(rdma_s)
                    break
                break
        return

    def PerfRCGenerate(self):
        self.nw_sessions = SessionHelper.GetAllMatchingLabel('RDMA')
        for nw_s in self.nw_sessions:

            ep1 = nw_s.initiator.ep
            ep2 = nw_s.responder.ep

            # lqp should come from a local endpoint
            if ep1.remote: continue

            # rqp should be local end point for local-local tests
            # rqp should be remote end point for all regular tests
            if GlobalOptions.l2l:
                if ep2.remote: continue
            else:
                if not ep2.remote: continue

            ep1_qps = self.__get_perf_qps_for_ep(ep1)
            ep2_qps = self.__get_perf_qps_for_ep(ep2)
            
            for lqp in ep1_qps:
                if lqp in self.used_qps: continue
                for rqp in ep2_qps:
                    if rqp in self.used_qps: continue
                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    vxlan = ep2.segment.IsFabEncapVxlan()

                    logger.info("RDMA PERF RC PICKED: EP1 %s (%s) EP2 %s (%s) LQP %s RQP %s" 
                                   "IPv6 %d VXLAN %d" % (ep1.GID(), not ep1.remote, ep2.GID(),
                                   not ep2.remote, lqp.GID(), rqp.GID(), nw_s.IsIPV6(), 
                                   ep2.segment.IsFabEncapVxlan()))

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp, vxlan)
                    self.rdma_sessions.append(rdma_s)
                    break
                break
        return


    def UDGenerate(self):
        self.nw_sessions = SessionHelper.GetAllMatchingLabel('RDMA')
        for nw_s in self.nw_sessions:

            ep1 = nw_s.initiator.ep
            ep2 = nw_s.responder.ep

            # lqp should come from a local endpoint
            if ep1.remote or not ep2.remote : continue

            ep1_qps = self.__get_qps_for_ep(ep1)
            ep2_qps = self.__get_qps_for_ep(ep2)
            
            for lqp in ep1_qps:
                if not lqp.svc == 3 : continue
                for rqp in ep2_qps:
                    if not rqp.svc == 3 : continue
                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    vxlan = ep2.segment.IsFabEncapVxlan()

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp, vxlan)
                    self.rdma_sessions.append(rdma_s)
                    break
                break
        return

    def Generate(self):
        # Local-Local tests are only run with perf RCs - mainly for performance
        # so for l2l tests, dont create Regular RC and UD sessions

        if not GlobalOptions.l2l:
            self.RCGenerate()

        self.PerfRCGenerate()

        if not GlobalOptions.l2l:
            self.UDGenerate()
        
    def Configure(self):
        if GlobalOptions.agent:
            return
        for rdma_s in self.rdma_sessions:
            rdma_s.Configure()
        halapi.ConfigureAhs(self.rdma_sessions)

    def main(self):
        self.Generate()
        self.Configure()
        if len(self.rdma_sessions):
            Store.objects.SetAll(self.rdma_sessions) 

    def __get_matching_sessions(self, selectors = None):
        ssns = []
        for ssn in self.rdma_sessions:
            if ssn.IsFilterMatch(selectors):
                ssns.append(ssn)
        if selectors.maxrdmasessions == 0:
            return ssns
        if selectors.maxrdmasessions >= len(ssns):
            return ssns
        return ssns[:selectors.maxrdmasessions]

    def GetMatchingConfigObjects(self, selectors = None):
        return self.__get_matching_sessions(selectors)

RdmaSessionHelper = RdmaSessionObjectHelper()
