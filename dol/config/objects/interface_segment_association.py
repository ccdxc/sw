#! /usr/bin/python3

import infra.config.base        as base
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import logger
from config.store               import Store

class HalInterfaceSegmentAssociationObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('INTERFACE_SEGMENT_ASSOCIATION'))
        return

    def Init(self, intf, seg):
        self.intf   = intf
        self.seg    = seg
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.if_key_handle.interface_id = self.intf.id
        req_spec.l2segment_key_or_handle.segment_id = self.seg.id
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("- Intf:%s <--> Seg:%s. Status = %s)" %\
                       (self.intf.GID(), self.seg.GID(),\
                        haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return


class HalInterfaceSegmentAssociationObjectHelper:
    def __init__(self):
        return

    def Generate(self, intfs, segs):
        objs = []
        for intf in intfs:
            for seg in segs:
                intf_seg_assoc = HalInterfaceSegmentAssociationObject()
                intf_seg_assoc.Init(intf, seg)
                objs.append(intf_seg_assoc)
        return objs

    def Configure(self, intfs, segs):
        objs = self.Generate(intfs, segs)
        halapi.ConfigureInterfaceSegmentAssociations(objs)
        return
