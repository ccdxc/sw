#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment

from infra.common.logging       import cfglogger
from config.store               import Store

import config.hal.api            as halapi
import config.hal.defs           as haldefs

class UplinkPcObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('UPLINKPC'))
        return

    def Init(self, spec):
        self.id = resmgr.InterfaceIdAllocator.get()
        self.GID(spec.id)
        self.port   = spec.port
        self.type   = 'UPLINK_PC'
        self.status = haldefs.interface.IF_STATUS_UP
        self.mode   = spec.mode
        self.native_segment = None

        self.members = []
        self.ports = []

        cfglogger.info("Creating UplinkPC = %s Port=%d" %\
                       (self.GID(), self.port))

        for mbr in spec.members:
            mbrobj = mbr.Get(Store)
            self.members.append(mbrobj)
            self.ports.append(mbrobj.port)
        return

    def SetNativeSegment(self, seg):
        assert(self.native_segment == None)
        cfglogger.info("Adding Segment:%s as native segment on UplinkPC:%s" %\
                       (seg.GID(), self.GID()))
        self.native_segment = seg
        return

    def IsTrunkPort(self):
        return self.mode == 'TRUNK'

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.interface_id = self.id

        req_spec.type = haldefs.interface.IF_TYPE_UPLINK_PC
        req_spec.admin_status = self.status

        req_spec.if_uplink_pc_info.uplink_pc_num = self.port
        if self.native_segment:
            req_spec.if_uplink_pc_info.native_l2segment_id = self.native_segment.id
        for mbr in self.members:
            req_spec.if_uplink_pc_info.member_if_handle.append(mbr.hal_handle)

        segments = Store.objects.GetAllByClass(segment.SegmentObject)
        for seg in segments:
            req_spec.if_uplink_pc_info.l2segment_id.append(seg.id)

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("- Uplink %s = %s" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))

        self.hal_handle = resp_spec.status.if_handle
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class UplinkPcObjectHelper:
    def __init__(self):
        self.uplinkpcs = []
        self.trunks = []
        return

    def Configure(self):
        if len(self.uplinkpcs) == 0: return
        cfglogger.info("Configuring %d UplinkPCs." % len(self.uplinkpcs))
        halapi.ConfigureInterfaces(self.uplinkpcs)
        return

    def ReConfigure(self):
        if len(self.uplinkpcs) == 0: return
        cfglogger.info("Updating %d UplinkPCs." % len(self.uplinkpcs))
        halapi.ConfigureInterfaces(self.uplinkpcs, update = True)
        return

    def ConfigureAllSegments(self):
        if len(self.uplinkpcs) == 0: return
        segs = Store.objects.GetAllByClass(segment.SegmentObject)
        for seg in segs:
            if seg.native == False: continue
            for uplinkpc in self.trunks:
                uplinkpc.SetNativeSegment(seg)
        halapi.ConfigureInterfaceSegmentAssociations(self.trunks, segs)
        return

    def Generate(self, topospec):
        if topospec.uplinkpc == None: return
        for upl in topospec.uplinkpc:
            obj = UplinkPcObject()
            obj.Init(upl.entry)
            self.uplinkpcs.append(obj)
            if obj.IsTrunkPort(): self.trunks.append(obj)
        return

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        if len(self.uplinkpcs) == 0: return
        cfglogger.info("Adding %d UplinkPcs to Store." % len(self.uplinkpcs))
        Store.objects.SetAll(self.uplinkpcs)
        Store.trunk_uplinks.SetAll(self.trunks)
        return

    def GetAll(self):
        return self.uplinkpcs

    def GetAllTrunkPorts(self):
        return self.trunks

UplinkPcHelper = UplinkPcObjectHelper()
