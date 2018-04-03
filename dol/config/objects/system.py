# /usr/bin/python3
import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
#import config.objects.session   as session

from config.store               import Store
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions

import config.hal.defs          as haldefs
import config.hal.api           as halapi
import telemetry_pb2            as telemetry_pb2
import system_pb2               as system_pb2
import config.objects.span      as span

class SystemObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SYSTEM_OBJECT'))
        return

    def Init(self):
        self.mirror_sessions = []

    def Show(self):
        logger.info("System Object       : %s" % self.GID())
        logger.info(" - MirrorSessions :")
        for sess in self.mirror_sessions:
            logger.info("    Session ID  : %d" % sess.id)

    def PrepareHALRequestSpec(self, reqspec):
        for sess in self.mirror_sessions:
            s = reqspec.span_on_drop_sessions.add()
            s.session_id = sess.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("  - System Object %s = %s" %(self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def PrepareHALGetRequestSpec(self, get_req_spec):
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        self.__get_resp = copy.deepcopy(get_resp)
        return

    def Get(self):
        halapi.GetSystem([self])
        return

    def GetDropStats(self):
        if GlobalOptions.dryrun:
            return None
        self.Get()
        return self.__get_resp.stats.drop_stats

class SystemObjectHelper:
    def __init__(self):
        self.systemObject = None
        return

    def Configure(self):
        if self.systemObject:
            logger.info("Configuring System Object")
            halapi.ConfigureSystem([self.systemObject])
        return

    def Generate(self, topospec):
        self.systemObject = SystemObject()
        self.systemObject.Init()

        dropaction = getattr(topospec, 'dropaction', None)
        if dropaction != "span":
            return

        for ssn in Store.objects.GetAllByClass(span.SpanSessionObject):
            if ssn.IsErspan:
                # reconfigure the ERSPAN session.
                ssn.Update(0, "ERSPAN", None)
            self.systemObject.mirror_sessions.append(ssn)
        self.systemObject.Show()

    def GetSystemObject(self):
        return self.systemObject

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        return

SystemHelper = SystemObjectHelper()
