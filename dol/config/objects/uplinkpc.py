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
        self.native_segment_id = 0

        self.members = []
        self.ports = []
        self.segment_ids = []
        self.members_hdls = []

        cfglogger.info("Creating UplinkPC = %s Port=%d" %\
                       (self.GID(), self.port))

        for mbr in spec.members:
            mbrobj = mbr.Get(Store)
            self.members.append(mbrobj)
            self.ports.append(mbrobj.port)
        self.__init_qos()
        self.Show()
        return

    def __init_qos(self):
        self.txqos.cos = 7
        self.rxqos.cos = 7 
        self.txqos.dscp = 7 
        self.rxqos.dscp = 7
        return

    def GetTxQosCos(self):
        return self.txqos.cos

    def GetRxQosCos(self):
        return self.rxqos.cos

    def GetTxQosDscp(self):
        return self.txqos.dscp

    def GetRxQosDscp(self):
        return self.rxqos.dscp

    def __copy__(self):
        uplink = UplinkPcObject()
        uplink.id = self.id
        uplink.port = self.port
        uplink.type = self.type
        uplink.status = self.status
        uplink.ports = self.ports[:]
        uplink.native_segment = self.native_segment
        uplink.native_segment_id = self.native_segment_id
        uplink.mode = self.mode
        uplink.members = self.members[:]
        uplink.members_hdls = self.members_hdls[:]
        uplink.segment_ids = self.segment_ids[:]
        return uplink


    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False
        fields = ["id", "port", "type", "status", "ports", "native_segment",
                  "native_segment_id", "mode", "sriov", "members",
                  "members_hdls", "segment_ids"]

        if not self.CompareObjectFields(other, fields, lgh):
            return False

        return True


    def Show(self):
        cfglogger.info("Creating UplinkPC = %s(IFID:%d) Port=%d" %\
                       (self.GID(), self.id, self.port))
        cfglogger.info("- txqos: Cos:%s/Dscp:%s" %\
                       (str(self.txqos.cos), str(self.txqos.dscp)))
        cfglogger.info("- rxqos: Cos:%s/Dscp:%s" %\
                       (str(self.rxqos.cos), str(self.rxqos.dscp)))
        for mbr in self.members:
            cfglogger.info("- Member: %s" % mbr.GID())
        return

    def Summary(self):
        summary = ''
        summary += 'GID:%s' % self.GID()
        summary += '/ID:%d' % self.id
        summary += '/Port:%d' % self.port
        if self.native_segment:
            summary += '/NativeSeg:%s' % self.native_segment.GID()
        summary += '/Ports:' + str(self.ports)
        return summary

    def SetNativeSegment(self, seg):
        assert(self.native_segment == None)
        cfglogger.info("Adding Segment:%s as native segment on UplinkPC:%s" %\
                       (seg.GID(), self.GID()))
        self.native_segment = seg
        self.native_segment_id = seg.id
        return

    def IsTrunkPort(self):
        return self.mode == 'TRUNK'

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.interface_id = self.id

        req_spec.type = haldefs.interface.IF_TYPE_UPLINK_PC
        req_spec.admin_status = self.status

        #req_spec.if_uplink_pc_info.uplink_pc_num = self.port
        if self.native_segment:
            req_spec.if_uplink_pc_info.native_l2segment_id = self.native_segment.id
        for mbr in self.members:
            req_spec.if_uplink_pc_info.member_if_handle.append(mbr.hal_handle)
            self.members_hdls.append(mbr.hal_handle)

        #segments = Store.objects.GetAllByClass(segment.SegmentObject)
        #for seg in segments:
        #    req_spec.if_uplink_pc_info.l2segment_id.append(seg.id)
        #    self.segment_ids.append(seg.id)

#        # QOS stuff
#        if self.txqos.cos is not None:
#            req_spec.tx_qos_actions.marking_spec.pcp_rewrite_en = True
#            req_spec.tx_qos_actions.marking_spec.pcp = self.txqos.cos
#        if self.txqos.dscp is not None:
#            req_spec.tx_qos_actions.marking_spec.dscp_rewrite_en = True
#            req_spec.tx_qos_actions.marking_spec.dscp = self.txqos.dscp


        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        cfglogger.info("- Uplink %s = %s" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))

        self.hal_handle = resp_spec.status.if_handle
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.key_or_handle.if_handle = self.hal_handle
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        if get_resp.spec.key_or_handle.HasField("interface_id"):
            self.id = get_resp.spec.key_or_handle.interface_id
        else:
            self.hal_handle = get_resp.spec.key_or_handle.if_handle
        if get_resp.spec.type == haldefs.interface.IF_TYPE_UPLINK_PC:
            self.status = get_resp.spec.admin_status
            #self.port = get_resp.spec.if_uplink_pc_info.uplink_pc_num
            self.native_segment_id = get_resp.spec.if_uplink_pc_info.native_l2segment_id
            get_resp.spec.if_uplink_pc_info.member_if_handle
            self.members_hdls = []
            for mbr in get_resp.spec.if_uplink_pc_info.member_if_handle:
                self.members_hdls.append(mbr)
            #self.segment_ids  = []
            #for segment_id in get_resp.spec.if_uplink_pc_info.l2segment_id:
            #    self.segment_ids.append(segment_id)
        else:
            self.status = None
            self.port = None
            self.native_segment_id = None
            self.members = []
            self.members_hdls = []
            self.segment_ids = []

    def Get(self):
        halapi.GetInterfaces([self])
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
        self.ReConfigure()
        halapi.ConfigureInterfaceSegmentAssociations(self.trunks, segs)
        return

    def Generate(self, topospec):
        upspec = getattr(topospec, 'uplinkpc', None)
        if upspec is None:
            return
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
        Store.SetTrunkingUplinks(self.trunks)
        return

    def GetAll(self):
        return self.uplinkpcs

    def GetAllTrunkPorts(self):
        return self.trunks

UplinkPcHelper = UplinkPcObjectHelper()


def GetMatchingObjects(selectors):
    uplinks =  Store.objects.GetAllByClass(UplinkPcObject)
    return [uplink for uplink in uplinks if uplink.IsFilterMatch(selectors.uplink)]
