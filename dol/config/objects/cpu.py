#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.logging       import logger
from config.store               import Store

class CpuObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('CPU'))
        return

    def Init(self, spec):
        self.GID(spec.id)
        self.id = resmgr.CpuIdAllocator.get()
        self.lif_id = spec.lif_id
        return

    def Show(self):
        logger.info("Creating Cpu = %s lif-id=%d" %\
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
        req_spec.type = haldefs.interface.IF_TYPE_CPU
        req_spec.if_cpu_info.lif_key_or_handle.lif_id = self.lif_id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        logger.info("- Cpu %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        #get_req_spec.key_or_handle.if_handle = self.hal_handle
        # HAL reserved 1003 as CPU If ID.
        get_req_spec.key_or_handle.interface_id = 1003
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        if get_resp.spec.key_or_handle.HasField("interface_id"):
            self.id = get_resp.spec.key_or_handle.interface_id
        else:
            self.hal_handle = get_resp.spec.key_or_handle.if_handle

        self.hal_handle = get_resp.status.if_handle;
        logger.info("- Cpu %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(get_resp.api_status),\
                        self.hal_handle))


class CpuObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Configure(self):
        logger.info("Configuring %d Cpus" % len(self.objlist))
        halapi.ConfigureInterfaces(self.objlist)
        return

    def GetConf(self):
        logger.info("Get Config from HAL for %d Cpus" % len(self.objlist))
        halapi.GetInterfaces(self.objlist)
        return

    def Generate(self, topospec):
        cpuspec = getattr(topospec, 'cpu', None)
        if cpuspec == None: return
        for cpu in cpuspec:
            obj = CpuObject()
            obj.Init(cpu.entry)
            self.objlist.append(obj)
        Store.objects.SetAll(self.objlist)
        return

    def main(self, topospec):
        self.Generate(topospec)
        # CPU if is created inside HAL during init ...
        #     DOL just queries for handle
        #self.Configure()
        self.GetConf()
        logger.info("Adding %d Cpus to Store." % len(self.objlist))
        return

    def GetAll(self):
        return self.objlist

CpuHelper = CpuObjectHelper()
