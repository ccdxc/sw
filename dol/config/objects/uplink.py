#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects

import config.resmgr            as resmgr
import config.objects.segment   as segment

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.logging       import cfglogger
from config.store               import Store

class UplinkObject(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('UPLINK'))
        return

    def Init(self, spec):
        self.GID(spec.id)
        self.id = resmgr.InterfaceIdAllocator.get()
        self.port   = spec.port
        self.type   = haldefs.interface.IF_TYPE_UPLINK
        self.status = haldefs.interface.IF_STATUS_UP
        self.sriov  = spec.sriov
        self.mode   = spec.mode

        cfglogger.info("Creating Uplink = %s Port=%d" %\
                       (self.GID(), self.port))
        return

    def IsTrunkPort(self):
        return self.mode == 'TRUNK'

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.interface_id = self.id

        req_spec.type = self.type
        req_spec.admin_status = self.status

        req_spec.if_uplink_info.port_num = self.port
        req_spec.if_uplink_info.native_l2segment_id = 0
        
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        cfglogger.info("- Uplink %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return

class UplinkObjectHelper:
    def __init__(self):
        self.objlist = []
        self.trunks = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Uplinks." % len(self.objlist))
        halapi.ConfigureInterfaces(self.objlist)
        return

    def ConfigureAllSegments(self):
        segs = Store.objects.GetAllByClass(segment.SegmentObject)
        halapi.ConfigureInterfaceSegmentAssociations(self.trunks, segs)
        return

    def Generate(self, topospec):
        for upl in topospec.uplink:
            obj = UplinkObject()
            obj.Init(upl.entry)
            self.objlist.append(obj)
            if obj.IsTrunkPort(): self.trunks.append(obj)
        return

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        cfglogger.info("Adding %d Uplinks to Store." % len(self.objlist))
        Store.objects.SetAll(self.objlist)
        Store.trunk_uplinks.SetAll(self.trunks)
        return

    def GetAll(self):
        return self.objlist

    def GetAllTrunkPorts(self):
        return self.trunks

UplinkHelper = UplinkObjectHelper()
