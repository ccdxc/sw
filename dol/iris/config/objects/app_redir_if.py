#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
import iris.config.objects.segment   as segment

import iris.config.hal.api            as halapi
import iris.config.hal.defs           as haldefs

from infra.common.logging       import logger
from iris.config.store               import Store

class AppRedirIfObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('APP_REDIR_IF'))
        return

    def Init(self, spec):
        self.GID(spec.id)
        self.id = resmgr.InterfaceIdAllocator.get()
        self.lif_id = spec.lif_id
        return

    def Show(self):
        logger.info("Creating APP_REDIR_IF = %s lif-id=%d" %\
                       (self.GID(), self.lif_id))
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/ID:%d' % self.id
        summary += '/Lif:%d' + self.lif_id
        return summary


    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.interface_id = self.id
        req_spec.type = haldefs.interface.IF_TYPE_APP_REDIR
        req_spec.if_app_redir_info.lif_key_or_handle.lif_id = self.lif_id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        logger.info("- APP_REDIR_IF %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return


class AppRedirIfObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self):
        logger.info("Configuring %d APP_REDIR_IFs" % len(self.objlist))
        halapi.ConfigureInterfaces(self.objlist)
        return

    def Generate(self, topospec):
        app_redir_if_spec = getattr(topospec, 'app_redir_if', None)
        if app_redir_if_spec == None: return
        for app_redir_if in app_redir_if_spec:
            obj = AppRedirIfObject()
            obj.Init(app_redir_if.entry)
            self.objlist.append(obj)
        Store.objects.SetAll(self.objlist)
        return

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        logger.info("Adding %d APP_REDIR_IFs to Store." % len(self.objlist))
        return

    def GetAll(self):
        return self.objlist

AppRedirIfHelper = AppRedirIfObjectHelper()
