#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment
import config.objects.tenant    as tenant

import config.hal.api            as halapi
import config.hal.defs           as haldefs

import config.objects.interface_segment_association as if_seg_assoc

from infra.common.glopts        import GlobalOptions
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
        self.native_segment_id = 0
        self.__init_qos()
        return

    def __init_qos(self):
        self.txqos.cos = 7
        self.rxqos.cos = 7
        self.txqos.dscp = 7
        self.rxqos.dscp = 7
        return

    def GetDistLabel(self):
        return "NW%d" % self.ports[0]

    def GetTxQosCos(self):
        return self.txqos.cos

    def GetRxQosCos(self):
        return self.rxqos.cos

    def GetTxQosDscp(self):
        return self.txqos.dscp

    def GetRxQosDscp(self):
        return self.rxqos.dscp
    
    def __copy__(self):
        uplink = UplinkObject()
        uplink.id = self.id
        uplink.port = self.port
        uplink.type = self.type
        uplink.status = self.status
        uplink.ports = self.ports
        uplink.native_segment = self.native_segment
        uplink.native_segment_id = self.native_segment_id
        uplink.mode = self.mode
        uplink.sriov = self.sriov
        return uplink


    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False
        fields = ["id", "port", "type", "status", "ports", "native_segment",
                  "native_segment_id", "mode", "sriov"]
        
        if not self.CompareObjectFields(other, fields, lgh):
            return False
        
        return True
    
    def Show(self):
        cfglogger.info("Creating Uplink = %s Port=%d" %\
                       (self.GID(), self.port))
        cfglogger.info("- txqos: Cos:%s/Dscp:%s" %\
                       (str(self.txqos.cos), str(self.txqos.dscp)))
        cfglogger.info("- rxqos: Cos:%s/Dscp:%s" %\
                       (str(self.rxqos.cos), str(self.rxqos.dscp)))
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
        self.native_segment_id = seg.id
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.interface_id = self.id

        req_spec.type = haldefs.interface.IF_TYPE_UPLINK
        req_spec.admin_status = self.status

        req_spec.if_uplink_info.port_num = self.port
        if self.native_segment:
            req_spec.if_uplink_info.native_l2segment_id = self.native_segment.id
     
#        # QOS stuff
#        if self.txqos.cos is not None:
#            req_spec.tx_qos_actions.marking_spec.pcp_rewrite_en = True
#            req_spec.tx_qos_actions.marking_spec.pcp = self.txqos.cos
#        if self.txqos.dscp is not None:
#            req_spec.tx_qos_actions.marking_spec.dscp_rewrite_en = True
#            req_spec.tx_qos_actions.marking_spec.dscp = self.txqos.dscp
   
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        cfglogger.info("- Uplink %s = %s (HDL = 0x%x)" %\
                       (self.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.key_or_handle.if_handle = self.hal_handle
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        if get_resp.spec.key_or_handle.HasField("interface_id"):
            self.id = get_resp.spec.key_or_handle.interface_id
        else:
            self.hal_handle = get_resp.spec.key_or_handle.if_handle
            
        if get_resp.spec.type == haldefs.interface.IF_TYPE_UPLINK:
            self.status = get_resp.spec.admin_status
            self.port = get_resp.spec.if_uplink_info.port_num
            self.native_segment_id = get_resp.spec.if_uplink_info.native_l2segment_id
        else:
            self.status = None
            self.port = 0
            self.native_segment_id = 0
        
    def Get(self):
        halapi.GetInterfaces([self])
            
    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)


class UplinkObjectHelper:
    def __init__(self):
        self.objlist = []
        self.trunks = []
        self.obj_helper_if_seg_assoc = if_seg_assoc.HalInterfaceSegmentAssociationObjectHelper()
        return

    def Configure(self):
        cfglogger.info("Configuring %d Uplinks." % len(self.objlist))
        halapi.ConfigureInterfaces(self.objlist)
        return

    def ReConfigure(self):
        cfglogger.info("Updating %d Uplinks." % len(self.objlist))
        halapi.ConfigureInterfaces(self.objlist, update = True)
        return

    def __configure_all_segments(self):
        segs = Store.objects.GetAllByClass(segment.SegmentObject)
        for seg in segs:
            if seg.native == False: continue
            for uplink in self.trunks:
                uplink.SetNativeSegment(seg)
        self.ReConfigure()
        self.obj_helper_if_seg_assoc.Configure(self.trunks, segs)
        return

    def __configure_all_segments_classic_nic(self):
        tens = Store.objects.GetAllByClass(tenant.TenantObject)
        for ten in tens:
            intf = ten.GetPinIf() 
            segs = ten.GetSegments()
            for seg in segs:
                if seg.native == False: continue
                intf.SetNativeSegment(seg)
                break
            halapi.ConfigureInterfaces([intf], update = True)
            self.obj_helper_if_seg_assoc.Configure([intf], segs)
        return

    def ConfigureAllSegments(self):
        if GlobalOptions.classic:
            self.__configure_all_segments_classic_nic()
        else:
            self.__configure_all_segments()
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

def GetMatchingObjects(selectors):
    uplinks =  Store.objects.GetAllByClass(UplinkObject)
    return [uplink for uplink in uplinks if uplink.IsFilterMatch(selectors.uplink)]
