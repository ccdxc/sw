#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api           as halapi
import config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2

import config.objects.qp        as qp
import config.objects.mr        as mr
from infra.common.glopts        import GlobalOptions

from config.objects.session     import SessionHelper

from scapy.utils import inet_aton, inet_ntoa
import struct

class RdmaSessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, session, lqp, rqp):
        self.id = resmgr.RdmaSessionIdAllocator.get()
        self.GID("RdmaSession%d" % self.id)
        self.lqp = lqp
        self.rqp = rqp
        self.ah_handle = 0
        self.session = session

    def Configure(self):
        self.lqp.ConfigureHeaderTemplate(self, True)
        #self.rqp.ConfigureHeaderTemplate(self, False)
        self.lqp.set_dst_qp(self.rqp.id)
        if self.lqp.svc == 3:
            self.lqp.set_q_key(self.lqp.id) #we are using q_id as q_key
        #self.lqp.rq.set_dst_qp(self.rqp.id)
        #cfglogger.info('RDMA Session: %s  Setting SQCB Local QID: %d Remote QID: %d ' % 
        #               (self.GID(), self.lqp.id, self.rqp.id))
        pass
         
    def Show(self):
        cfglogger.info('RDMA Session: %s' % self.GID())
        cfglogger.info('- Session: %s' % self.session.GID())
        cfglogger.info('- LQP: %s' % self.lqp.GID())
        cfglogger.info('- RQP: %s' % self.rqp.GID())
        return

    def PrepareHALRequestSpec(self, req_spec):
        #
        # For now we only have Ah Requests. In future we need to make sure what kind
        # of spec it is.
        #
        cfglogger.info("PrepareHALRequestSpec:: RDMA Session: %s Session: %s "
                       "Remote QP: %s Local QP: %s\n" %\
                        (self.GID(), self.session.GID(), self.rqp.GID(), self.lqp.GID()))
        if (GlobalOptions.dryrun): return

        req_spec.smac = bytes(self.session.initiator.ep.macaddr.getnum().to_bytes(6, 'little'))
        req_spec.dmac = bytes(self.session.responder.ep.macaddr.getnum().to_bytes(6, 'little'))
        req_spec.ethtype = 0x800
        req_spec.vlan = self.session.initiator.ep.intf.encap_vlan_id
        req_spec.vlan_pri = self.session.iflow.txqos.cos
        req_spec.vlan_cfi = 0
        req_spec.ip_ver = 4
        req_spec.ip_tos = self.session.iflow.txqos.dscp
        req_spec.ip_ttl = 64
        req_spec.ip_saddr = struct.unpack("!L",inet_aton(self.session.initiator.addr.get()))[0]
        req_spec.ip_daddr = struct.unpack("!L",inet_aton(self.session.responder.addr.get()))[0]
        req_spec.udp_sport = int(self.session.iflow.sport)
        req_spec.udp_dport = int(self.session.iflow.dport)
        
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("ProcessHALResponse:: RDMA Session: %s Session: %s "
                       "Remote QP: %s Local QP: %s\n" %\
                        (self.GID(), self.session.GID(), self.rqp.GID(), self.lqp.GID()))

        self.ah_handle = resp_spec.ah_handle
        return

    def IsFilterMatch(self, selectors):
        cfglogger.debug('Matching RDMA Session: %s' % self.GID())

        match = self.lqp.IsFilterMatch(selectors.rdmasession.lqp)
        cfglogger.debug("- LQP Filter Match =", match)
        if match == False: return match
        
        match = self.rqp.IsFilterMatch(selectors.rdmasession.rqp)
        cfglogger.debug("- RQP Filter Match =", match)
        if match == False: return  match
        
        return True

    def SetupTestcaseConfig(self, obj):
        obj.rdmasession = self;
        return

    def ShowTestcaseConfig(self, obj, logger):
        logger.info("Config Objects for %s" % self.GID())
        self.lqp.ShowTestcaseConfig(obj.rdmasession.lqp, logger)
        self.rqp.ShowTestcaseConfig(obj.rdmasession.rqp, logger)
        return


class RdmaSessionObjectHelper:
    def __init__(self):
        self.rdma_sessions = []
        self.nw_sessions = []
        self.used_qps = []
        return

    def __get_qps_for_ep(self, ep):
        if hasattr(ep, 'pds'):
            pds = ep.pds.GetAll()
            for pd in pds:
                qps = pd.qps.GetAll()
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
            
            for lqp in ep1_qps:
                if lqp in self.used_qps: continue
                if not lqp.svc == 0 : continue
                for rqp in ep2_qps:
                    if not rqp.svc == 0 : continue
                    if rqp in self.used_qps: continue
                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp)
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

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp)
                    self.rdma_sessions.append(rdma_s)
                    break
                break
        return

    def Generate(self):
        self.RCGenerate()
        self.UDGenerate()
        
    def Configure(self):
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
