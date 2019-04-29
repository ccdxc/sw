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

from infra.common.glopts        import GlobalOptions

from iris.config.objects.session     import SessionHelper

from scapy.utils import inet_aton, inet_ntoa, inet_pton
import struct
import socket

class NvmeSessionObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, session):
        self.id = resmgr.NvmeSessionIdAllocator.get()
        self.GID("NvmeSession%d" % self.id)
        self.session = session
        self.IsIPV6 = session.IsIPV6()

    def Configure(self):
        #TBD
        return
         
    def Show(self):
        logger.info('Nvme Session: %s' % self.GID())
        logger.info('- Session: %s' % self.session.GID())
        logger.info('- IsIPV6: %s' % self.IsIPV6)
        return

    def PrepareHALRequestSpec(self, req_spec):
        if (GlobalOptions.dryrun): return
        #TBD
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("ProcessHALResponse:: Nvme Session: %s Session: %s "
                     % (self.GID(), self.session.GID()))
        #TBD
        return

    def IsIPV6(self):
        return self.IsIPV6

    def IsFilterMatch(self, selectors):
        logger.debug('Matching Nvme Session: %s' % self.GID())

        #TBD: add nvme specific filters here
        print(str(selectors))

        if hasattr(selectors.nvmesession, 'base'):
            match = super().IsFilterMatch(selectors.nvmesession.base.filters)
            if match == False: return  match

        if hasattr(selectors.nvmesession, 'session'):
            match = super().IsFilterMatch(selectors.nvmesession.session.filters)
            logger.debug("- IsIPV6 Filter Match =", match)
            if match == False: return  match
        
        return True

    def SetupTestcaseConfig(self, obj):
        obj.Nvmesession = self;
        return

    def ShowTestcaseConfig(self, obj):
        logger.info("Config Objects for %s" % self.GID())
        return

class NvmeSessionObjectHelper:
    def __init__(self):
        self.nvme_sessions = []
        return

    def Generate(self):
        logger.info("Generating NVME sessions..")
        self.nw_sessions = SessionHelper.GetAllMatchingLabel('NVME-PROXY')
        for nw_s in self.nw_sessions:

            ep1 = nw_s.initiator.ep
            ep2 = nw_s.responder.ep

            # create nvme sessions only for nw sessions between 'local' initiator
            # and 'remote' responder which has TCP port 4420
            if ep1.remote: continue
            if not ep2.remote: continue
            if not nw_s.responder.proto == 'TCP': continue
            if not nw_s.responder.port == 4420: continue

            logger.info("Nvme PICKED: EP1 %s (%s) EP2 %s (%s) IPv6 %d " 
                         % (ep1.GID(), not ep1.remote, ep2.GID(), not ep2.remote, 
                            nw_s.IsIPV6()))

            Nvme_s = NvmeSessionObject()
            Nvme_s.Init(nw_s)
            self.nvme_sessions.append(Nvme_s)

        return

    def Configure(self):
        logger.info("Configuring NVME sessions..")
        if GlobalOptions.agent:
            return
        for Nvme_s in self.nvme_sessions:
            logger.info('Walking Nvme Session: %s ' % 
                        (Nvme_s.GID()))
            Nvme_s.Configure()

    def main(self):
        self.Generate()
        self.Configure()
        if len(self.nvme_sessions):
            Store.objects.SetAll(self.nvme_sessions) 

        [s.Show() for s in self.nvme_sessions]

    def __get_matching_sessions(self, selectors = None):
        ssns = []
        for ssn in self.nvme_sessions:
            if ssn.IsFilterMatch(selectors):
                ssns.append(ssn)
        return ssns

    def GetMatchingConfigObjects(self, selectors = None):
        return self.__get_matching_sessions(selectors)

NvmeSessionHelper = NvmeSessionObjectHelper()
def GetMatchingObjects(selectors):
    return NvmeSessionHelper.GetMatchingConfigObjects(selectors)
