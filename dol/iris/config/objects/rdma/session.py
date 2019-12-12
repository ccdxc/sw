#! /usr/bin/python3
import pdb

import scapy.all                as scapy
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
from iris.config.store               import Store
from infra.common.logging       import logger

import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2

import iris.config.objects.qp        as qp
import iris.config.objects.mr        as mr
from infra.common.glopts        import GlobalOptions

from iris.config.objects.session     import SessionHelper

from scapy.utils import inet_aton, inet_ntoa, inet_pton
import struct
import socket

class RdmaSessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('RDMA_SESSION'))
        return

    def Init(self, session, lqp, rqp, vxlan, ahid, ahid_remote):
        self.id = resmgr.RdmaSessionIdAllocator.get()
        self.GID("RdmaSession%d" % self.id)
        self.lqp = lqp
        self.rqp = rqp
        self.ah_handle = ahid
        self.ah_remote = ahid_remote
        self.session = session
        self.IsIPV6 = session.IsIPV6()
        self.IsVXLAN = vxlan

    def Configure(self):
        if not self.lqp.svc == 3:
            self.lqp.ConfigureHeaderTemplate(self, self.session.initiator,
                                         self.session.responder,
                                         self.session.iflow,
                                         self.session.IsIPV6(), True,
                                         self.ah_handle)
            self.lqp.set_dst_qp(self.rqp.id)
            self.lqp.set_rsq_wqes()
            self.lqp.set_rrq_wqes()
            self.lqp.set_pmtu()

        if self.lqp.svc == 3:
            self.lqp.set_q_key(self.lqp.id) #we are using q_id as q_key

        self.lqp.set_access_flags(1,1,1) # REMOTE_WRITE | READ | ATOMIC
        self.lqp.set_q_state(3) # Q_STATE_RTS

        # For local-local rdma sessions, need to setup reverse direction too
        if not self.rqp.remote:
            self.rqp.ConfigureHeaderTemplate(self, self.session.responder,
                                         self.session.initiator,
                                         self.session.rflow,
                                         self.session.IsIPV6(), False,
                                         self.ah_remote)
            self.rqp.set_dst_qp(self.lqp.id)
            self.rqp.set_rsq_wqes(flip=True)
            self.rqp.set_rrq_wqes(flip=True)
            self.rqp.set_pmtu()
            self.rqp.set_access_flags(1,1,1) # REMOTE_WRITE | READ | ATOMIC
            self.rqp.set_q_state(3)

        pass
         
    def Show(self):
        logger.info('RDMA Session: %s' % self.GID())
        logger.info('- Session: %s' % self.session.GID())
        logger.info('- LQP: %s  Tiny: %s Remote: %s Service: %d' \
                    %(self.lqp.GID(), self.lqp.tiny, self.lqp.remote, self.lqp.svc))
        logger.info('- RQP: %s  Tiny: %s Remote: %s Service: %d' \
                     %(self.rqp.GID(), self.rqp.tiny, self.rqp.remote, self.rqp.svc))
        logger.info('- IsIPV6: %s' % self.IsIPV6)
        logger.info('- IsVxLAN: %s' % self.IsVXLAN)
        return

    def PrepareHALRequestSpec(self, req_spec):
        #
        # For now we only have Ah Requests. In future we need to make sure what kind
        # of spec it is.
        #
        if not self.lqp.svc == 3: return

        logger.info("PrepareHALRequestSpec:: RDMA Session: %s Session: %s "
                       "Remote QP: %s Local QP: %s" %\
                       (self.GID(), self.session.GID(), self.rqp.GID(), self.lqp.GID()))
        if (GlobalOptions.dryrun): return

        logger.info("RdmaAhCreate:: src_ip: %s dst_ip: %s src_mac: %s dst_mac: %s proto: %s "
                        "sport: %s dport: %s ah_handle: %d isipv6: %d" %\
                    (self.session.initiator.addr.get(), self.session.responder.addr.get(),
                     self.session.initiator.ep.macaddr.get(), self.session.responder.ep.macaddr.get(),
                     self.session.iflow.proto, self.session.iflow.sport, self.session.iflow.dport,
                     self.ah_handle, self.session.IsIPV6()))

        EthHdr = scapy.Ether(src=self.session.initiator.ep.macaddr.get(),
                             dst=self.session.responder.ep.macaddr.get())
        Dot1qHdr = scapy.Dot1Q(vlan=self.session.initiator.ep.intf.encap_vlan_id,
                               prio=self.session.iflow.txqos.cos)
        if self.session.IsIPV6():
            IpHdr = scapy.IPv6(src=self.session.initiator.addr.get(),
                             dst=self.session.responder.addr.get(),
                             tc=self.session.iflow.txqos.dscp,
                             plen = 0)
        else:
            IpHdr = scapy.IP(src=self.session.initiator.addr.get(),
                             dst=self.session.responder.addr.get(),
                             tos=0, # keep tos = 0 to not trigger ecn mark
                             len = 0, chksum = 0)
        UdpHdr = scapy.UDP(sport=self.session.iflow.sport,
                           dport=self.session.iflow.dport,
                           len = 0, chksum = 0)

        req_spec.hw_lif_id = self.lqp.pd.ep.intf.lif.hw_lif_id
        req_spec.header_template = bytes(EthHdr/Dot1qHdr/IpHdr/UdpHdr)
        req_spec.ahid = self.ah_handle

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("ProcessHALResponse:: RDMA Session: %s Session: %s "
                       "Remote QP: %s Local QP: %s" %\
                        (self.GID(), self.session.GID(), self.rqp.GID(), self.lqp.GID()))
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
        self.rdma_ud_sessions = []
        self.nw_sessions = []
        self.used_qps = []
        self.v4_non_vxlan_count = 0
        self.v6_non_vxlan_count = 0
        self.v4_vxlan_count = 0
        self.v6_vxlan_count = 0
        self.ahid = 0
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

    def __get_dcqcn_qps_for_ep(self, ep):
        if hasattr(ep, 'pds'):
            pds = ep.pds.GetAll()
            for pd in pds:
                qps = pd.dcqcn_qps.GetAll()
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
                           if self.v4_non_vxlan_count > 3: continue
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
                                   "IPv6 %d VXLAN %d AHID %d" % (ep1.GID(), not ep1.remote, ep2.GID(),
                                   not ep2.remote, lqp.GID(), rqp.GID(), nw_s.IsIPV6(), 
                                   ep2.segment.IsFabEncapVxlan(), self.ahid))

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp, vxlan, self.ahid, self.ahid + 1)
                    self.rdma_sessions.append(rdma_s)

                    if not rqp.remote:
                        self.ahid += 1
                    self.ahid += 1
                    break
                break
        return

    def DcqcnRCGenerate(self):
        self.nw_sessions = SessionHelper.GetAllMatchingLabel('RDMA')
        for nw_s in self.nw_sessions:

            ep1 = nw_s.initiator.ep
            ep2 = nw_s.responder.ep

            # lqp should come from a local endpoint
            if ep1.remote or not ep2.remote : continue

            ep1_qps = self.__get_dcqcn_qps_for_ep(ep1)
            ep2_qps = self.__get_dcqcn_qps_for_ep(ep2)

            for lqp in ep1_qps:
                if lqp in self.used_qps: continue
                for rqp in ep2_qps:
                    if rqp in self.used_qps: continue
                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    vxlan = ep2.segment.IsFabEncapVxlan()
                    ipv6  = nw_s.IsIPV6()

                    if not ipv6 and not vxlan:  # v4 non-vxlan
                       self.v4_non_vxlan_count += 1
                       if GlobalOptions.perf:
                           if self.v4_non_vxlan_count > 0: continue
                       else:
                           if self.v4_non_vxlan_count > 31: continue
                    elif ipv6 and not vxlan:    # v6 non-vxlan
                       self.v6_non_vxlan_count +=1
                       if GlobalOptions.perf:
                           if self.v6_non_vxlan_count > 0: continue
                       else:
                           if self.v6_non_vxlan_count > 0: continue

                    logger.info("RDMA DCQCN RC PICKED: EP1 %s (%s) EP2 %s (%s) LQP %s RQP %s" 
                                   "IPv6 %d VXLAN %d AHID %d" % (ep1.GID(), not ep1.remote, ep2.GID(),
                                   not ep2.remote, lqp.GID(), rqp.GID(), nw_s.IsIPV6(), 
                                   ep2.segment.IsFabEncapVxlan(), self.ahid))

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp, vxlan, self.ahid, self.ahid + 1)
                    self.rdma_sessions.append(rdma_s)

                    if not rqp.remote:
                        self.ahid += 1
                    self.ahid += 1
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
                    vxlan = ep2.segment.IsFabEncapVxlan()
                    ipv6  = nw_s.IsIPV6()

                    if vxlan and not nw_s.iflow.IsL2():
                       #logger.info('SKIP: VXLAN Routed Sessions for now')
                       continue

                    if not ipv6 and not vxlan:  # v4 non-vxlan
                       self.v4_non_vxlan_count += 1
                       if GlobalOptions.perf:
                           if self.v4_non_vxlan_count > 8: continue
                       else:
                           if self.v4_non_vxlan_count > 21:
                               continue
                    elif ipv6 and not vxlan:    # v6 non-vxlan
                       self.v6_non_vxlan_count +=1
                       if GlobalOptions.perf:
                           if self.v6_non_vxlan_count > 0: continue
                       else:
                           if self.v6_non_vxlan_count > 0: continue

                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    vxlan = ep2.segment.IsFabEncapVxlan()

                    logger.info("RDMA PERF RC PICKED: EP1 %s (%s) EP2 %s (%s) LQP %s RQP %s" 
                                   "IPv6 %d VXLAN %d AHID %d" % (ep1.GID(), not ep1.remote, ep2.GID(),
                                   not ep2.remote, lqp.GID(), rqp.GID(), nw_s.IsIPV6(), 
                                   ep2.segment.IsFabEncapVxlan(), self.ahid))

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp, vxlan, self.ahid, self.ahid + 1)
                    self.rdma_sessions.append(rdma_s)

                    if not rqp.remote:
                        self.ahid += 1
                    self.ahid += 1
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
                    if not rqp.remote : continue
                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    vxlan = ep2.segment.IsFabEncapVxlan()

                    logger.info("RDMA UD PICKED: EP1 %s (%s) EP2 %s (%s) LQP %s RQP %s"
                                   "IPv6 %d VXLAN %d AHID %d" % (ep1.GID(), not ep1.remote, ep2.GID(),
                                   not ep2.remote, lqp.GID(), rqp.GID(), nw_s.IsIPV6(),
                                   ep2.segment.IsFabEncapVxlan(), self.ahid))

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp, vxlan, self.ahid, 0)
                    self.rdma_sessions.append(rdma_s)
                    self.rdma_ud_sessions.append(rdma_s)

                    self.ahid += 1
                    break
                break
        return

    def Generate(self):
        # Local-Local tests are only run with perf RCs - mainly for performance
        # so for l2l tests, dont create Regular RC, UD and DCQCN sessions

        if not GlobalOptions.l2l:
            self.RCGenerate()

        self.PerfRCGenerate()

        if not GlobalOptions.l2l:
            self.DcqcnRCGenerate()

        if not GlobalOptions.l2l:
            self.UDGenerate()
       
    def Configure(self):
        if GlobalOptions.agent:
            return
        for rdma_s in self.rdma_sessions:
            logger.info('Walking RDMA Session: %s ' % 
                        (rdma_s.GID()))
            rdma_s.Configure()
        halapi.ConfigureAhs(self.rdma_ud_sessions)

    def main(self):
        self.Generate()
        self.Configure()
        if len(self.rdma_sessions):
            Store.objects.SetAll(self.rdma_sessions) 

        [s.Show() for s in self.rdma_sessions]
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
def GetMatchingObjects(selectors):
    return RdmaSessionHelper.GetMatchingConfigObjects(selectors)
