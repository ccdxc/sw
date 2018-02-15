# /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.tenant    as tenant
import config.objects.segment   as segment
import config.objects.lif       as lif
import config.objects.tunnel    as tunnel
import config.objects.span      as span

from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.defs          as haldefs
import config.hal.api           as halapi

class QosClassObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('QOS_CLASS'))
        self.id = resmgr.QosClassIdAllocator.get()
        return

    def Init(self, spec):
        self.spec = objects.MergeObjects(spec, self.spec)

        self.GID('QOS_CLASS_' + spec.group)

        self.hal_handle = None
        self.cmap_dot1q_pcp = 0

        if self.UplinkCmapNeeded():
            self.cmap_dot1q_pcp = resmgr.QosCosAllocator.get()

        if self.spec.marking.dot1q_pcp_en:
            self.spec.marking.dot1q_pcp = self.cmap_dot1q_pcp

        if self.spec.marking.ip_dscp_en:
            self.spec.marking.ip_dscp = resmgr.QosDscpAllocator.get()

        self.Show()
        return

    def Show(self):
        cfglogger.info("QosClass  : %s (id: %d)" % (self.GID(), self.id))
        cfglogger.info("- UplinkCmap: dot1q_pcp %d" % self.cmap_dot1q_pcp)
        cfglogger.info("- Spec")
        cfglogger.info("    Marking: dot1q_pcp %s/%d ip_dscp %s/%d" % \
                                        (self.spec.marking.dot1q_pcp_en,
                                         self.spec.marking.dot1q_pcp,
                                         self.spec.marking.ip_dscp_en,
                                         self.spec.marking.ip_dscp))
        return

    def Configure(self):
        cfglogger.info("Configuring QosClass  : %s (id: %d)" % (self.GID(), self.id))
        self.Show()
        halapi.ConfigureQosClass([self])
        return

    def __getGroupEnum(self, group):
        return haldefs.qos.QosGroup.Value(group)

    def PrepareHALRequestSpec(self, req_spec):

        req_spec.key_or_handle.qos_group = self.__getGroupEnum(self.spec.group)
        # Mandatory parameters
        req_spec.mtu = 2000
        req_spec.sched.dwrr.bw_percentage = 10

        if self.UplinkCmapNeeded():
            req_spec.uplink_class_map.dot1q_pcp = self.cmap_dot1q_pcp

        req_spec.marking.dot1q_pcp_rewrite_en = self.spec.marking.dot1q_pcp_en
        req_spec.marking.dot1q_pcp = self.spec.marking.dot1q_pcp
        req_spec.marking.ip_dscp_rewrite_en = self.spec.marking.ip_dscp_en
        req_spec.marking.ip_dscp = self.spec.marking.ip_dscp
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.qos_class_handle
        cfglogger.info("  - Qos-class %s = %s handle: 0x%x" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status), 
                        self.hal_handle))
        return

    def GetTxQosCos(self):
        if self.spec.marking.dot1q_pcp_en:
            return self.spec.marking.dot1q_pcp
        return 7

    def GetTxQosDscp(self):
        if self.spec.marking.ip_dscp_en:
            return self.spec.marking.ip_dscp
        return 7

    def UplinkCmapNeeded(self):
        if self.spec.group.startswith('USER_DEFINED'): 
            return True
        return False

# Helper Class to Generate/Configure/Manage Qos Objects.
class QosClassObjectHelper:
    def __init__(self):
        self.qos_classes = []
        return

    def Configure(self):
        cfglogger.info("Configuring %d Qos-classes." % len(self.qos_classes)) 
        halapi.ConfigureQosClass(self.qos_classes)
        return
        
    def Generate(self, topospec):
        qosspec = getattr(topospec, 'qos', False)
        if qosspec is False:
            return
        spec = Store.specs.Get('QOS_CLASS')
        cfglogger.info("Creating Qos classes")
        for e in spec.entries:
            qos_class = QosClassObject()
            qos_class.Init(e.entry)
            self.qos_classes.append(qos_class)
        Store.objects.SetAll(self.qos_classes)
        return

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        return

QosClassHelper = QosClassObjectHelper()
