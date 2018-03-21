# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
#import config.objects.session   as session

from config.store               import Store
from infra.common.logging       import logger

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

class SystemObjectHelper:
    def __init__(self):
        self.systemObject = []
        return

    def Configure(self):
        logger.info("Configuring System Object")
        objs = []

        objs.append(self.systemObject)
        halapi.ConfigureSystem(objs)
        return

    def Generate(self, topospec):
        if 'dropaction' in topospec.__dict__ and topospec.dropaction == "span":
            self.systemObject = SystemObject()
            self.systemObject.Init()
            for ssn in Store.objects.GetAllByClass(span.SpanSessionObject):
                if ssn.IsErspan:
                    # reconfigure the ERSPAN session.
                    ssn.Update(0, "ERSPAN", None)
                self.systemObject.mirror_sessions.append(ssn)
            self.systemObject.Show()

    def GetSystemObject(self):
        ret = []
        for c in self.systemObject:
            ret.append(c)
        return ret

    def main(self, topospec):
        self.Generate(topospec)
        if 'dropaction' in topospec.__dict__ and topospec.dropaction == "span":
            self.Configure()
        return

SystemHelper = SystemObjectHelper()
