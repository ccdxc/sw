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

from config.objects.session     import SessionHelper

class RdmaSessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, session, lqp, rqp):
        self.id = resmgr.RdmaSessionIdAllocator.get()
        self.GID("RdmaSession%d" % self.id)
        self.lqp = lqp
        self.rqp = rqp
        self.session = session

    def Configure(self):
        self.lqp.ConfigureHeaderTemplate(self, True)
        #self.rqp.ConfigureHeaderTemplate(self, False)
        pass
         
    def Show(self):
        cfglogger.info('RDMA Session: %s' % self.GID())
        cfglogger.info('- Session: %s' % self.session.GID())
        cfglogger.info('- LQP: %s' % self.lqp.GID())
        cfglogger.info('- RQP: %s' % self.rqp.GID())
        return

    def PrepareHALRequestSpec(self, req_spec):
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("Configuring Session with GID:%s" % self.GID())
        return

    def IsFilterMatch(self, config_filter):
        cfglogger.debug('Matching RDMA Session: %s' % self.GID())
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
        pds = ep.pds.GetAll()
        for pd in pds:
            qps = pd.qps.GetAll()
            return qps

    def Generate(self):
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
                for rqp in ep2_qps:
                    if rqp in self.used_qps: continue
                    self.used_qps.append(lqp)
                    self.used_qps.append(rqp)

                    rdma_s = RdmaSessionObject()
                    rdma_s.Init(nw_s, lqp, rqp)
                    self.rdma_sessions.append(rdma_s)
                    break
                break
        return

    def Configure(self):
        for rdma_s in self.rdma_sessions:
            rdma_s.Configure()

    def main(self):
        self.Generate()
        self.Configure()
        if len(self.rdma_sessions):
            Store.objects.SetAll(self.rdma_sessions) 

    def GetMatchingConfigObjects(self, config_filter = None):
        return self.rdma_sessions

RdmaSessionHelper = RdmaSessionObjectHelper()
