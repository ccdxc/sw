#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr

from infra.common.logging       import cfglogger
from config.store               import Store

import config.hal.api            as halapi
import config.hal.defs           as haldefs

ENIC_TYPE_DIRECT= 'DIRECT'
ENIC_TYPE_USEG  = 'USEG'
ENIC_TYPE_PVLAN = 'PVLAN'

class EnicObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.id = resmgr.InterfaceIdAllocator.get()
        self.GID("Enic%d" % self.id)
        return

    def Init(self, segment, type):
        self.segment = segment
        self.tenant  = segment.tenant
        self.type    = type
        self.macaddr = resmgr.EnicMacAllocator.get()
        if self.IsUseg() or self.IsPvlan():
            self.encap_vlan_id = resmgr.EncapVlanAllocator.get()
        else:
            self.encap_vlan_id = segment.vlan_id
        self.ep      = None
        return

    def AttachEndpoint(self, ep):
        assert(self.ep == None)
        self.lif            = self.tenant.AllocLif()
        self.ep = ep
        self.Show()
        return

    def Show(self):
        cfglogger.info("- Enic = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("  - TenantID   = %d" % self.tenant.id)
        cfglogger.info("  - Ep         = %s" % self.ep.GID())
        cfglogger.info("  - EnicType   = %s" % self.type)
        cfglogger.info("  - EncapVlan  = %d" % self.encap_vlan_id)
        cfglogger.info("  - Lif        = %s" % self.lif.GID())
        return

    def IsSegmentMatch(self, segid):
        return self.segment.GID() == segid

    def IsDirect(self):
        return self.type == ENIC_TYPE_DIRECT

    def IsUseg(self):
        return self.type == ENIC_TYPE_USEG

    def IsPvlan(self):
        return self.type == ENIC_TYPE_PVLAN

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.meta.tenant_id = self.tenant.id
        req_spec.type           = haldefs.interface.IF_TYPE_ENIC
        req_spec.admin_status   = haldefs.interface.IF_STATUS_UP
        req_spec.key_or_handle.interface_id = self.id
        req_spec.if_enic_info.lif_key_or_handle.lif_id = self.lif.id
        req_spec.if_enic_info.mac_address = self.macaddr.getnum()
        req_spec.if_enic_info.encap_vlan_id = self.encap_vlan_id
        req_spec.if_enic_info.l2segment_id = self.ep.segment.id
        if self.IsDirect():
            req_spec.if_enic_info.enic_type = haldefs.interface.IF_ENIC_TYPE_DIRECT
        elif self.IsUseg():
            req_spec.if_enic_info.enic_type = haldefs.interface.IF_ENIC_TYPE_USEG
        elif self.IsPvlan():
            req_spec.if_enic_info.enic_type = haldefs.interface.IF_ENIC_TYPE_PVLAN
        else:
            assert(0)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        cfglogger.info("- Enic %s = %s (HDL = 0x%x)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class EnicObjectHelper:
    def __init__(self):
        self.enics  = []
        self.direct = []
        self.useg   = []
        self.pvlan  = []
        return
    
    def Show(self):
        cfglogger.info("  - # EN: Dir=%d Useg=%d Pvlan=%d Tot=%d" %\
                       (len(self.direct), len(self.useg), len(self.pvlan),
                        len(self.enics)))
        return

    def __create_enics(self, segment, count, enic_type):
        enics = []
        for e in range(count):
            enic = EnicObject()
            enic.Init(segment, enic_type)
            enics.append(enic)
        return enics

    def Generate(self, segment, spec):
        if spec.direct:
            cfglogger.info("Creating %d DIRECT Enics in Segment = %s" %\
                           (spec.direct, segment.GID()))
            self.direct = self.__create_enics(segment, spec.direct,
                                              ENIC_TYPE_DIRECT)
            Store.objects.SetAll(self.direct)
            self.enics += self.direct

        if spec.useg:
            cfglogger.info("Creating %d USEG Enics in Segment = %s" %\
                           (spec.useg, segment.GID()))
            self.useg   = self.__create_enics(segment, spec.useg,
                                              ENIC_TYPE_USEG)
            Store.objects.SetAll(self.useg)
            self.enics += self.useg

        if spec.pvlan:
            cfglogger.info("Creating %d PVLAN Enics in Segment = %s" %\
                           (spec.pvlan, segment.GID()))
            self.pvlan  = self.__create_enics(segment, spec.pvlan,
                                              ENIC_TYPE_PVLAN)
            Store.objects.SetAll(self.pvlan)
            self.enics += self.pvlan
        return

    def Configure(self):
        cfglogger.info("Configuring %d Enics." % len(self.enics))
        halapi.ConfigureInterfaces(self.enics)
        return
