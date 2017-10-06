#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.logging       import cfglogger
from config.store               import Store

class UplinkObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('UPLINK'))
        return

    def Init(self, spec):
        self.GID(spec.id)
        self.id = resmgr.InterfaceIdAllocator.get()
        self.port   = spec.port
        self.type   = 'UPLINK'
        self.status = haldefs.interface.IF_STATUS_UP
        self.sriov  = spec.sriov
        self.mode   = spec.mode
        self.ports  = [ spec.port ]
        self.native_segment = None

        return
    def Show(self):
        cfglogger.info("Creating Uplink = %s Port=%d" %\
                       (self.GID(), self.port))
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/ID:%d' % self.id
        summary += '/Ports:' + str(self.ports)
        return summary

    def IsTrunkPort(self):
        return self.mode == 'TRUNK'

    def SetNativeSegment(self, seg):
        assert(self.native_segment == None)
        cfglogger.info("Adding Segment:%s as native segment on Uplink:%s" %\
                       (seg.GID(), self.GID()))
        self.native_segment = seg
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.interface_id = self.id

        req_spec.type = haldefs.interface.IF_TYPE_UPLINK
        req_spec.admin_status = self.status

        req_spec.if_uplink_info.port_num = self.port
        if self.native_segment:
            req_spec.if_uplink_info.native_l2segment_id = self.native_segment.id
        
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        cfglogger.info("- Uplink %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return
    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)


class UplinkObjectHelper:
    def __init__(self):
        self.objlist = []
        self.trunks = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Uplinks." % len(self.objlist))
        halapi.ConfigureInterfaces(self.objlist)
        return

    def ReConfigure(self):
        cfglogger.info("Updating %d Uplinks." % len(self.objlist))
        halapi.ConfigureInterfaces(self.objlist, update = True)
        return

    def ConfigureAllSegments(self):
        segs = Store.objects.GetAllByClass(segment.SegmentObject)
        for seg in segs:
            if seg.native == False: continue
            for uplink in self.trunks:
                uplink.SetNativeSegment(seg)
        self.ReConfigure()
        halapi.ConfigureInterfaceSegmentAssociations(self.trunks, segs)
        return

    def Generate(self, topospec):
        upspec = getattr(topospec, 'uplink', None)
        if upspec is None:
            return
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
        if len(self.objlist) == 0: return
        Store.objects.SetAll(self.objlist)
        Store.SetTrunkingUplinks(self.trunks)
        return

    def GetAll(self):
        return self.objlist

    def GetAllTrunkPorts(self):
        return self.trunks

UplinkHelper = UplinkObjectHelper()
