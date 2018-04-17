#! /usr/bin/python3
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.queue_type     as queue_type
import infra.clibs.clibs        as clibs

from config.store               import Store
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions

import config.hal.api            as halapi
import config.hal.defs           as haldefs
import test.callbacks.eth.toeplitz as toeplitz

import pdb

import ctypes

class QInfoStruct(ctypes.Structure):
    _fields_ = [("dryrun", ctypes.c_bool),
                ("lif_id", ctypes.c_uint64),
                ("q0_addr", ctypes.c_uint64 * 8)]

class LifObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('LIF'))
        return

    def Init(self, tenant, spec, namespace = None):
        if namespace:
            self.id = namespace.get()
        else:
            self.id = resmgr.LifIdAllocator.get()
        self.GID("Lif%d" % self.id)
        self.status     = haldefs.interface.IF_STATUS_UP
        self.hw_lif_id = -1
        self.qstate_base = {}
        self.promiscuous = False
        self.allmulticast = False

        self.c_lib_name = getattr(spec, 'c_lib', None)
        if self.c_lib_name:
            self.c_lib = clibs.LoadCLib(self.c_lib_name)
            if self.c_lib:
                self.c_lib_config = self.c_lib[self.c_lib_name + '_config']
                self.c_lib_config.argtypes = [ctypes.POINTER(QInfoStruct)]
                self.c_lib_config.restype = None
        else:
            self.c_lib = None

        if hasattr(spec, 'rdma') and spec.rdma.enable:
            self.enable_rdma = spec.rdma.enable
            self.rdma_max_pt_entries = spec.rdma.max_pt_entries
            self.rdma_max_keys = spec.rdma.max_keys
        else:
            self.enable_rdma = False
            self.rdma_max_pt_entries = 0
            self.rdma_max_keys = 0
        self.rdma_pt_base_addr = 0
        self.rdma_kt_base_addr = 0

        self.vlan_strip_en = False
        self.vlan_insert_en = False

        self.queue_types = objects.ObjectDatabase()
        self.obj_helper_q = queue_type.QueueTypeObjectHelper()
        self.obj_helper_q.Generate(self, spec)
        self.queue_types.SetAll(self.obj_helper_q.queue_types)
        self.queue_types_list = self.obj_helper_q.queue_types
        self.queue_list = []
        for q_type in self.queue_types_list:
            for queue in q_type.queues.GetAll():
                self.queue_list.append(queue)

        # RDMA per LIF allocators
        if self.enable_rdma:
            self.qpid_allocator = objects.TemplateFieldObject("range/0/16384")
            self.cqid_allocator = objects.TemplateFieldObject("range/0/16384")
            self.eqid_allocator = objects.TemplateFieldObject("range/0/256")
            self.pd_allocator = objects.TemplateFieldObject("range/0/128")
            self.mr_key_allocator = objects.TemplateFieldObject("range/0/8192")

        if hasattr(spec, 'rss') and spec.rss.enable:
            self.rss_enable = True
            self.rss_type = (haldefs.interface.LifRssType.Value("RSS_TYPE_IPV4") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV4_TCP") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV4_UDP") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV6") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV6_TCP") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV6_UDP"))
            self.rss_key = toeplitz.toeplitz_msft_key
        else:
            self.rss_enable = False
            self.rss_type = 0
            self.rss_key = toeplitz.toeplitz_msft_key

        self.tenant     = tenant
        self.spec       = spec
        
        self.tx_qos_class = None
        self.rx_qos_class = None
        if self.tenant.IsQosEnabled():
            self.tx_qos_class = getattr(spec, 'tx_qos_class', None)
            self.rx_qos_class = getattr(spec, 'rx_qos_class', None)
            if self.tx_qos_class:
                self.tx_qos_class = Store.objects.Get(self.tx_qos_class)
            if self.rx_qos_class:
                self.rx_qos_class = Store.objects.Get(self.rx_qos_class)

        self.Show()

    def GetQpid(self):
        return self.qpid_allocator.get()

    #TODO: Until Yogesh's fix comes in for Unaligned write back, just allocate CQIDs as even number
    def GetCqid(self):
        cqid = self.cqid_allocator.get()
        self.cqid_allocator.get()
        return cqid

    def GetEqid(self):
        return self.eqid_allocator.get()

    def GetPd(self):
        return self.pd_allocator.get()

    def GetMrKey(self):
        return self.mr_key_allocator.get()

    def GetQt(self, type):
        return self.queue_types.Get(type)

    def GetQ(self, type, qid):
        qt = self.queue_types.Get(type)
        if qt is not None:
            return qt.queues.Get(str(qid))

    def GetQstateAddr(self, type):
        if GlobalOptions.dryrun:
            return 0
        return self.qstate_base[type]

    def GetTxQosCos(self):
        if self.tx_qos_class:
            return self.tx_qos_class.GetTxQosCos()
        return 7

    def GetTxQosDscp(self):
        if self.tx_qos_class:
            return self.tx_qos_class.GetTxQosDscp()
        return 7

    def ConfigureQueueTypes(self):
        if GlobalOptions.dryrun:
            return 0
        self.obj_helper_q.Configure()

    def Show(self):
        logger.info("- LIF   : %s" % self.GID())
        logger.info("  - # Queue Types    : %d" % len(self.obj_helper_q.queue_types))

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.lif_id = self.id
        req_spec.admin_status = self.status
        req_spec.enable_rdma = self.enable_rdma
        req_spec.rdma_max_keys = self.rdma_max_keys
        req_spec.rdma_max_pt_entries = self.rdma_max_pt_entries
        req_spec.vlan_strip_en = self.vlan_strip_en
        req_spec.vlan_insert_en = self.vlan_insert_en
        if self.tx_qos_class:
            req_spec.tx_qos_class.qos_group = self.tx_qos_class.GroupEnum()
        if self.rx_qos_class:
            req_spec.rx_qos_class.qos_group = self.rx_qos_class.GroupEnum()

        if GlobalOptions.classic:
            req_spec.packet_filter.receive_broadcast = True
            req_spec.packet_filter.receive_promiscuous = self.promiscuous
            req_spec.packet_filter.receive_all_multicast = self.allmulticast
        req_spec.rss.enable = self.rss_enable
        req_spec.rss.type = self.rss_type
        req_spec.rss.key = bytes(self.rss_key)
        for queue_type in self.queue_types.GetAll():
            qstate_map_spec = req_spec.lif_qstate_map.add()
            queue_type.PrepareHALRequestSpec(qstate_map_spec)
            for queue in queue_type.queues.GetAll():
                qstate_spec = req_spec.lif_qstate.add()
                queue.PrepareHALRequestSpec(qstate_spec)

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.lif_handle
        if (self.c_lib):
            self.CLibConfig(resp_spec)
        if self.hw_lif_id == -1:
            # HAL does not return hw_lif_id in the UpdateResponse. Set the hw_lif_id only once.
            self.hw_lif_id = resp_spec.status.hw_lif_id
        logger.info("- LIF %s = %s HW_LIF_ID = %s (HDL = 0x%x)" %
                       (self.GID(),
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hw_lif_id, self.hal_handle))
        for qstate in resp_spec.qstate:
            logger.info("- QUEUE_TYPE = %d QSTATE_ADDR = 0x%x" % (qstate.type_num, qstate.addr))
            self.qstate_base[qstate.type_num] = qstate.addr
        if self.enable_rdma:
           if resp_spec.rdma_data_valid:
               self.rdma_pt_base_addr = resp_spec.rdma_data.pt_base_addr
               self.rdma_kt_base_addr = resp_spec.rdma_data.kt_base_addr
           logger.info("- RDMA-DATA: LIF %s =  HW_LIF_ID = %s PT-Base-Addr = 0x%x KT-Base-Addr= 0x%x)" %
                          (self.GID(), self.hw_lif_id, self.rdma_pt_base_addr, self.rdma_kt_base_addr))

    def PrepareHALGetRequestSpec(self, req_spec):
        req_spec.key_or_handle.lif_id = self.id
        return

    def ProcessHALGetResponse(self, req_spec, resp_spec):
        logger.info("- GET LIF %s = %s" % (self.GID(), 
                    haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        self.get_resp = copy.deepcopy(resp_spec)
        return

    def Get(self):
        halapi.GetLifs([self])
        return

    def GetStats(self):
        if GlobalOptions.dryrun:
            return None
        self.Get()
        return self.get_resp.stats

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def CLibConfig(self, resp_spec):
       qaddrs_type = ctypes.c_uint64 * 8
       qaddrs = qaddrs_type()
       for qstate in resp_spec.qstate:
           qaddrs[int(qstate.type_num)] = qstate.addr
       qinfo = QInfoStruct(GlobalOptions.dryrun, resp_spec.status.hw_lif_id, qaddrs)
       self.c_lib_config(ctypes.byref(qinfo))

    def Update(self):
        halapi.ConfigureLifs([self], update=True)

    def SetPromiscous(self):
        logger.info("Setting PROMISCUOUS mode for LIF:%s" % self.GID())
        self.promiscuous = True
        return

    def IsPromiscous(self):
        return self.promiscuous

    def SetAllMulticast(self):
        logger.info("Setting ALL MULTICAST mode for LIF:%s" % self.GID())
        self.allmulticast = True
        return

    def IsAllMulticast(self):
        return self.allmulticast

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class LifObjectHelper:
    def __init__(self):
        self.lifs = []
        self.plifs = []
        self.mlifs = []
        self.aidx = 0

    def Generate(self, tenant, spec, namespace, n_prom, n_allmc):
        if namespace is None:
            return
        count = namespace.GetCount()
        logger.info("Creating %d Lifs. for Tenant:%s NProm:%d NAllmc:%d" %\
                       (count, tenant.GID(), n_prom, n_allmc))
        for l in range(count):
            lif = LifObject()
            lif.Init(tenant, spec, namespace)
            self.lifs.append(lif)

        if n_prom != 0:
            self.plifs = self.lifs[-n_prom:]
            for l in self.plifs:
                l.SetPromiscous()

        if n_allmc != 0:
            self.mlifs = self.lifs[-n_allmc:]
            for l in self.mlifs:
                l.SetAllMulticast()
        return

    def Configure(self):
        if not LifObject().InFeatureSet():
            return
        if len(self.lifs) == 0:
            return
        logger.info("Configuring %d LIFs." % len(self.lifs))
        halapi.ConfigureLifs(self.lifs)
        for lif in self.lifs:
            lif.ConfigureQueueTypes()
        Store.objects.SetAll(self.lifs)

    def Update(self):
        logger.info("Updating %d LIFs." % len(self.lifs))
        halapi.ConfigureLifs(self.lifs, update=True)
        for lif in self.lifs:
            lif.ConfigureQueueTypes()

    def Alloc(self):
        if self.aidx >= len(self.lifs):
            assert(0)
        lif = self.lifs[self.aidx]
        self.aidx += 1
        return lif
