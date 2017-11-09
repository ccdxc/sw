#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base
import config.resmgr            as resmgr

from infra.common.logging       import cfglogger
from infra.common.glopts        import GlobalOptions
from config.store               import Store

import config.hal.api            as halapi
import config.hal.defs           as haldefs

ENIC_TYPE_DIRECT= 'DIRECT'
ENIC_TYPE_USEG  = 'USEG'
ENIC_TYPE_PVLAN = 'PVLAN'
ENIC_TYPE_CLASSIC = 'CLASSIC'

global gl_pinif_iter
gl_pinif_iter = 0

class EnicObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('ENIC'))
        self.id = resmgr.InterfaceIdAllocator.get()
        self.GID("Enic%d" % self.id)
        return

    def Init(self, segment, type, l4lb_backend = False):
        self.segment        = segment
        self.tenant         = segment.tenant
        self.tenant_id      = self.tenant.id
        self.type           = type
        self.l4lb_backend   = l4lb_backend

        self.pinnedif       = None
        self.macaddr = resmgr.EnicMacAllocator.get()
        self.__pin_interface()

        if self.IsUseg() or self.IsPvlan():
            self.encap_vlan_id = resmgr.EncapVlanAllocator.get()
        else:
            self.encap_vlan_id = segment.vlan_id
        self.ep = None
        self.label = None
        self.__init_qos()
        return

    def __pin_interface_for_hostpin_mode(self):
        trunks = Store.GetTrunkingUplinks()
        global gl_pinif_iter
        gl_pinif_iter += 1
        gl_pinif_iter %= len(trunks)
        self.pinnedif = trunks[gl_pinif_iter]
        self.macaddr.update(self.pinnedif.id << 16)
        return
        
    def __pin_interface_for_classic(self):
        self.pinnedif = self.tenant.GetPinIf()
        return

    def __pin_interface(self):
        if self.tenant.IsHostPinned():
            self.__pin_interface_for_hostpin_mode()
        elif self.IsClassic():
            self.__pin_interface_for_classic()
        else:
            return
        cfglogger.info("- %s: Pinning to Interface: %s" %\
                       (self.GID(), self.pinnedif))
        return

    def AttachEndpoint(self, ep):
        assert(self.ep == None)
        self.lif = self.tenant.AllocLif()
        self.lif_id = self.lif.id
        self.ep = ep
        self.ep_segment_id = self.ep.segment.id
        self.Show()
        return

    def Show(self):
        cfglogger.info("Enic = %s(%d)" % (self.GID(), self.id))
        cfglogger.info("- Tenant     = %s" % self.tenant.GID())
        cfglogger.info("- Ep         = %s" % self.ep.GID())
        cfglogger.info("- EnicType   = %s" % self.type)
        cfglogger.info("- EncapVlan  = %d" % self.encap_vlan_id)
        cfglogger.info("- Lif        = %s" % self.lif.GID())
        cfglogger.info("- TxQos      = Cos:%s/Dscp:%s" %\
                       (str(self.txqos.cos), str(self.txqos.dscp)))
        cfglogger.info("- RxQos      = Cos:%s/Dscp:%s" %\
                       (str(self.rxqos.cos), str(self.rxqos.dscp)))
        if self.pinnedif:
            cfglogger.info("- PinnedIF   = %s" % self.pinnedif.GID())
        return

    def Summary(self):
        summary = ''
        summary += '%s' % self.GID()
        summary += '/%s' % self.tenant.GID()
        summary += '/%s' % self.ep.GID()
        summary += '/%s' % self.type
        summary += '/EncVlan:%s' % self.encap_vlan_id
        summary += '/%s' % self.lif.GID()
        return summary

    def __init_qos(self):
        self.txqos.cos = resmgr.QosCosAllocator.get()
        self.rxqos.cos = resmgr.QosCosAllocator.get()
        self.txqos.dscp = resmgr.QosDscpAllocator.get()
        self.rxqos.dscp = resmgr.QosDscpAllocator.get()
        return

    def GetTxQosCos(self):
        return self.txqos.cos

    def GetRxQosCos(self):
        return self.rxqos.cos

    def GetTxQosDscp(self):
        return self.txqos.dscp

    def GetRxQosDscp(self):
        return self.rxqos.cos

    def __copy__(self):
        enic = EnicObject()
        enic.id = self.id
        enic.hal_handle = self.hal_handle
        enic.tenant_id = self.tenant_id
        enic.lif_id = self.lif_id
        enic.ep_segment_id = self.ep_segment_id
        enic.encap_vlan_id = self.encap_vlan_id
        enic.macaddr = self.macaddr
        enic.type = self.type
        return enic

    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False
        fields = ["tenant_id", "hal_handle", "lif_id", "ep_segment_id", "encap_vlan_id", "type"]
        if not self.CompareObjectFields(other, fields, lgh):
            return False

        if  self.macaddr.getnum() != other.macaddr.getnum():
            lgh.error("Field mismatch, Field : %s, Expected : %s, Actual : %s"
                 %("macaddr", self.macaddr.string, other.macaddr.string))
            return False
        return True

    def IsSegmentMatch(self, segid):
        return self.segment.GID() == segid

    def IsDirect(self):
        return self.type == ENIC_TYPE_DIRECT

    def IsUseg(self):
        return self.type == ENIC_TYPE_USEG

    def IsPvlan(self):
        return self.type == ENIC_TYPE_PVLAN

    def IsClassic(self):
        return self.type == ENIC_TYPE_CLASSIC

    def PrepareHALRequestSpec(self, req_spec):
        assert(self.ep != None)
        req_spec.meta.vrf_id = self.tenant.id
        req_spec.type           = haldefs.interface.IF_TYPE_ENIC
        req_spec.admin_status   = haldefs.interface.IF_STATUS_UP
        req_spec.key_or_handle.interface_id = self.id

        req_spec.if_enic_info.lif_key_or_handle.lif_id = self.lif.id
        if self.IsClassic():
            req_spec.if_enic_info.enic_type = haldefs.interface.IF_ENIC_TYPE_CLASSIC
            if self.segment.native:
                req_spec.if_enic_info.classic_enic_info.native_l2segment_handle = self.segment.hal_handle
            else:
                req_spec.if_enic_info.classic_enic_info.l2segment_handle.append(self.segment.hal_handle)
            req_spec.if_enic_info.pinned_uplink_if_handle = self.pinnedif.hal_handle
        else:
            req_spec.if_enic_info.enic_info.mac_address = self.macaddr.getnum()
            req_spec.if_enic_info.enic_info.encap_vlan_id = self.encap_vlan_id
            req_spec.if_enic_info.enic_info.l2segment_id = self.ep.segment.id
            if self.IsDirect():
                req_spec.if_enic_info.enic_type = haldefs.interface.IF_ENIC_TYPE_DIRECT
            elif self.IsUseg():
                req_spec.if_enic_info.enic_type = haldefs.interface.IF_ENIC_TYPE_USEG
            elif self.IsPvlan():
                req_spec.if_enic_info.enic_type = haldefs.interface.IF_ENIC_TYPE_PVLAN
            else:
                assert(0)

        # QOS stuff
        if self.txqos.cos is not None:
            req_spec.tx_qos_actions.marking_spec.pcp_rewrite_en = True
            req_spec.tx_qos_actions.marking_spec.pcp = self.txqos.cos
        if self.txqos.dscp is not None:
            req_spec.tx_qos_actions.marking_spec.dscp_rewrite_en = True
            req_spec.tx_qos_actions.marking_spec.dscp = self.txqos.dscp

        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.if_handle
        cfglogger.info("- Enic %s = %s (HDL = 0x%x)" %\
                       (self.GID(), haldefs.common.ApiStatus.Name(resp_spec.api_status),\
                        self.hal_handle))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.meta.vrf_id = self.tenant.id
        get_req_spec.key_or_handle.if_handle = self.hal_handle
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        self.tenant_id = get_resp.spec.meta.vrf_id
        if get_resp.spec.key_or_handle.HasField("interface_id"):
            self.id = get_resp.spec.key_or_handle.interface_id
        else:
            self.hal_handle = get_resp.spec.key_or_handle.if_handle

        if get_resp.spec.type == haldefs.interface.IF_TYPE_ENIC:
            if get_resp.spec.if_enic_info.enic_type == haldefs.interface.IF_ENIC_TYPE_DIRECT:
                self.type = ENIC_TYPE_DIRECT
            elif get_resp.spec.if_enic_info.enic_type == haldefs.interface.IF_ENIC_TYPE_USEG:
                self.type  = ENIC_TYPE_USEG
            elif get_resp.spec.if_enic_info.enic_type  == haldefs.interface.IF_ENIC_TYPE_PVLAN:
                self.type  = ENIC_TYPE_PVLAN
            else:
                self.type = None
        self.lif.id = get_resp.spec.if_enic_info.lif_key_or_handle.lif_id
        self.macaddr = objects.MacAddressBase(integer=get_resp.spec.if_enic_info.mac_address)
        self.encap_vlan_id = get_resp.spec.if_enic_info.encap_vlan_id
        self.ep_segment_id = get_resp.spec.if_enic_info.l2segment_id

    def Get(self):
        halapi.GetInterfaces([self])

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
        cfglogger.info("- # Enics: Dir=%d Useg=%d Pvlan=%d Tot=%d" %\
                       (len(self.direct), len(self.useg),
                        len(self.pvlan), len(self.enics)))
        cfglogger.info("- # Backend Enics: Dir=%d Useg=%d Pvlan=%d Tot=%d" %\
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
        num_direct = getattr(spec, 'direct', 0)
        if num_direct == 0: return enics
        cfglogger.info("Creating %d DIRECT (Backend:%s) Enics in Segment = %s" %\
                       (spec.direct, l4lb_backend, segment.GID()))
        enics = self.__create(segment, spec.direct, ENIC_TYPE_DIRECT)
        Store.objects.SetAll(enics)
        return enics

    def __gen_useg(self, segment, spec, l4lb_backend = False):
        enics = []
        num_useg = getattr(spec, 'useg', 0)
        if num_useg == 0: return enics
        cfglogger.info("Creating %d USEG (Backend:%s) Enics in Segment = %s" %\
                       (spec.useg, l4lb_backend, segment.GID()))
        enics = self.__create(segment, spec.useg, ENIC_TYPE_USEG)
        Store.objects.SetAll(enics)
        return enics

    def __gen_pvlan(self, segment, spec, l4lb_backend = False):
        enics = []
        num_pvlan = getattr(spec, 'pvlan', 0)
        if num_pvlan == 0: return enics
        cfglogger.info("Creating %d PVLAN (Backend:%s) Enics in Segment = %s" %\
                       (spec.pvlan, l4lb_backend, segment.GID()))
        enics = self.__create(segment, spec.pvlan, ENIC_TYPE_PVLAN)
        Store.objects.SetAll(enics)
        return enics

    def __gen_classic(self, segment, spec, l4lb_backend = False):
        enics = []
        if GlobalOptions.classic is False: return enics
        num_classic = getattr(spec, 'classic', 0)
        if num_classic == 0: return enics
        cfglogger.info("Creating %d CLASSIC Enics in Segment = %s" %\
                       (spec.classic, segment.GID()))
        enics = self.__create(segment, spec.classic, ENIC_TYPE_CLASSIC)
        Store.objects.SetAll(enics)
        return enics


    def Generate(self, segment, spec):
        self.direct = self.__gen_direct(segment, spec)
        self.enics += self.direct
        self.useg = self.__gen_useg(segment, spec)
        self.enics += self.useg
        self.pvlan = self.__gen_pvlan(segment, spec)
        self.enics += self.pvlan
        self.classic = self.__gen_classic(segment, spec)
        self.enics += self.classic

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

def GetMatchingObjects(selectors):
    enics =  Store.objects.GetAllByClass(EnicObject)
    return [enic for enic in enics if enic.IsFilterMatch(selectors.enic)]
