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

    def Init(self, segment, type, l4lb_backend = False):
        self.segment = segment
        self.tenant  = segment.tenant
        self.type    = type
        self.l4lb_backend = l4lb_backend
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
        cfglogger.info("  - Tenant     = %s" % self.tenant.GID())
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
        assert(self.ep != None)
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

        self.backend_enics  = []
        self.backend_direct = []
        self.backend_useg   = []
        self.backend_pvlan  = []
        return
    
    def Show(self):
        cfglogger.info("  - # Enics: Dir=%d Useg=%d Pvlan=%d Tot=%d" %\
                       (len(self.direct), len(self.useg),
                        len(self.pvlan), len(self.enics)))
        cfglogger.info("  - # Backend Enics: Dir=%d Useg=%d Pvlan=%d Tot=%d" %\
                       (len(self.backend_direct), len(self.backend_useg),
                        len(self.backend_pvlan), len(self.backend_enics)))
        return

    def __create(self, segment, count, enic_type, l4lb_backend = False):
        enics = []
        for e in range(count):
            enic = EnicObject()
            enic.Init(segment, enic_type, l4lb_backend)
            enics.append(enic)
        return enics

    def __gen_direct(self, segment, spec, l4lb_backend = False):
        enics = []
        if spec.direct == 0: return enics
        cfglogger.info("Creating %d DIRECT (Backend:%s) Enics in Segment = %s" %\
                       (spec.direct, l4lb_backend, segment.GID()))
        enics = self.__create(segment, spec.direct, ENIC_TYPE_DIRECT)
        Store.objects.SetAll(enics)
        return enics

    def __gen_useg(self, segment, spec, l4lb_backend = False):
        enics = []
        if spec.useg == 0: return enics
        cfglogger.info("Creating %d USEG (Backend:%s) Enics in Segment = %s" %\
                       (spec.useg, l4lb_backend, segment.GID()))
        enics = self.__create(segment, spec.useg, ENIC_TYPE_USEG)
        Store.objects.SetAll(enics)
        return enics

    def __gen_pvlan(self, segment, spec, l4lb_backend = False):
        enics = []
        if spec.pvlan == 0: return enics
        cfglogger.info("Creating %d PVLAN (Backend:%s) Enics in Segment = %s" %\
                       (spec.pvlan, l4lb_backend, segment.GID()))
        enics = self.__create(segment, spec.pvlan, ENIC_TYPE_PVLAN)
        Store.objects.SetAll(enics)
        return enics

    def Generate(self, segment, spec):
        self.direct = self.__gen_direct(segment, spec)
        self.enics += self.direct
        self.useg = self.__gen_useg(segment, spec)
        self.enics += self.useg
        self.pvlan = self.__gen_pvlan(segment, spec)
        self.enics += self.pvlan

        if segment.l4lb:
            self.backend_direct = self.__gen_direct(segment, spec,
                                                    l4lb_backend = True)
            self.backend_enics += self.backend_direct
        
            self.backend_useg = self.__gen_useg(segment, spec,
                                                l4lb_backend = True)
            self.backend_enics += self.backend_useg

            self.backend_pvlan = self.__gen_pvlan(segment, spec,
                                                  l4lb_backend = True)
            self.backend_enics += self.backend_pvlan
        return

    def Configure(self):
        cfglogger.info("Configuring %d Enics." % len(self.enics))
        halapi.ConfigureInterfaces(self.enics)
        if len(self.backend_enics):
            cfglogger.info("Configuring %d L4LbBackend Enics." %\
                           len(self.backend_enics))
            halapi.ConfigureInterfaces(self.backend_enics)
        return
