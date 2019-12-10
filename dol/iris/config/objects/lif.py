#! /usr/bin/python3
import copy
import array

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
import iris.config.objects.queue_type     as queue_type
import iris.config.objects.nvme.lif as nvme_lif
import infra.clibs.clibs        as clibs

from iris.config.store               import Store
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions

import iris.config.hal.api            as halapi
import iris.config.hal.defs           as haldefs
import iris.test.callbacks.eth.toeplitz as toeplitz

import iris.config.objects.aq   as aqs
import iris.config.objects.cq   as cqs
import iris.config.objects.eq   as eq
import iris.config.objects.slab   as slab
import iris.config.objects.rdma.dcqcn_profile_table   as dcqcn
import pdb

from bitstring import BitArray
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
        self.pds = []

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
            self.rdma_max_ahs = spec.rdma.max_ahs
            self.hostmem_pg_size = spec.rdma.hostmem_pg_size
            self.hbm_barmap_entries = (int(spec.rdma.hbm_barmap_size / spec.rdma.hostmem_pg_size))
            self.rdma_tbl_pos = BitArray(length=self.rdma_max_pt_entries)
            self.hbm_tbl_pos = BitArray(length=self.hbm_barmap_entries)
            self.rdma_async_eq_id = 0
            self.rdma_admin_aq_id = 1
            self.rdma_admin_cq_id = 0
        else:
            self.enable_rdma = False
            self.rdma_max_pt_entries = 0
            self.rdma_max_keys = 0
            self.rdma_max_ahs = 0
            self.hbm_barmap_entries = 0
            self.hostmem_pg_size = 0
        self.rdma_pt_base_addr = 0
        self.rdma_kt_base_addr = 0
        self.rdma_dcqcn_profile_base_addr = 0
        self.rdma_at_base_addr = 0

        if hasattr(spec, 'nvme') and spec.nvme.enable:
            self.enable_nvme = spec.nvme.enable
            self.nvme_lif = nvme_lif.NvmeLifObject(self, spec.nvme)
        else:
            self.enable_nvme = False

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
            # QP 0, 1 are special QPs
            self.qpid_allocator = objects.TemplateFieldObject("range/2/" + str(spec.rdma.max_qp))
            # AQ ID 0 is owned by ETH
            self.aqid_allocator = objects.TemplateFieldObject("range/1/" + str(spec.rdma.max_aq))
            # Reserve CQ 0, 1 for special QPs, AQ
            self.cqid_allocator = objects.TemplateFieldObject("range/2/" + str(spec.rdma.max_cq))
            self.eqid_allocator = objects.TemplateFieldObject("range/0/" + str(spec.rdma.max_eq))
            self.pd_allocator = objects.TemplateFieldObject("range/0/" + str(spec.rdma.max_pd))
            self.mr_key_allocator = objects.TemplateFieldObject("range/1/" + str(spec.rdma.max_mr))
            self.slab_allocator = objects.TemplateFieldObject("range/0/2048")
            self.kslab_allocator = objects.TemplateFieldObject("range/0/2048")

            # Generate RDMA LIF owned resources
            self.slabs = objects.ObjectDatabase()

            # Generate KernelSlab of 4KB and 10 MB for FRPMR 
            self.kslab_4KB = slab.SlabObject(self, 4096, True) 

            self.obj_helper_slab = slab.SlabObjectHelper()
            #slab_spec = spec.rdma.slab.Get(Store)
            #self.obj_helper_slab.Generate(self, slab_spec)
            #self.slabs.SetAll(self.obj_helper_slab.slabs)

            # Create EQs for RDMA LIF
            self.eqs = objects.ObjectDatabase()
            self.obj_helper_eq = eq.EqObjectHelper()
            self.obj_helper_eq.Generate(self, spec.rdma.max_eq, spec.rdma.max_eqe)
            if len(self.obj_helper_eq.eqs):
                self.eqs.SetAll(self.obj_helper_eq.eqs)

            # Create CQ 0 for adminQ
            logger.info("Creating 1 Cqs. for LIF:%s" % (self.GID()))
            # Hardcode CQ 0 for AQ
            # Page size is calculated as max_cqe * cqe_size by the CQ for privileged resources
            cq_id = 0
            self.cq = cqs.CqObject(None, cq_id, spec.rdma.max_cqe, 0, True, self)

            # Create AdminQ
            logger.info("Creating 1 Aqs. for LIF:%s" % (self.GID()))
            aq_id = self.GetAqid()
            self.aq = aqs.AqObject(self, aq_id, spec.rdma.max_aqe, spec.rdma.hostmem_pg_size)

            self.dcqcn_config_spec = spec.rdma.dcqcn_config.Get(Store)

        if hasattr(spec, 'rss'):
            self.rss_type = (haldefs.interface.LifRssType.Value("RSS_TYPE_IPV4") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV4_TCP") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV4_UDP") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV6") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV6_TCP") |
                            haldefs.interface.LifRssType.Value("RSS_TYPE_IPV6_UDP"))
            self.rss_key = array.array('B', toeplitz.toeplitz_msft_key)
            self.rss_indir = array.array('B', [0] * 128)
        else:
            self.rss_type = haldefs.interface.LifRssType.Value("RSS_TYPE_NONE")
            self.rss_key = array.array('B', toeplitz.toeplitz_msft_key)
            self.rss_indir = array.array('B', [0] * 128)

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

    def GetCqid(self):
        return self.cqid_allocator.get()

    def GetEqid(self):
        return self.eqid_allocator.get()

    def GetAqid(self):
        return self.aqid_allocator.get()

    def GetSlabid(self):
        return self.slab_allocator.get()

    def GetKSlabid(self):
        return self.kslab_allocator.get()

    def GetPd(self):
        return self.pd_allocator.get()

    def GetMrKey(self):
        return self.mr_key_allocator.get()

    def GetRdmaTblPos(self, num_pages):
        if num_pages <= 0:
            return
        total_pages = int(num_pages)
        if total_pages <= 8:
            total_pages = 8
        else:
            total_pages = 1 << (total_pages - 1).bit_length()
        logger.info("- LIF: %s Requested: %d Actual: %d page allocation in RDMA PT Table" %
                        (self.GID(), num_pages, total_pages))
        page_order = BitArray(length=total_pages)
        page_order.set(False)
        tbl_pos = self.rdma_tbl_pos.find(page_order)
        assert(tbl_pos)
        self.rdma_tbl_pos.set(True, range(tbl_pos[0], tbl_pos[0] + total_pages))

        return tbl_pos[0]

    def GetHbmTblPos(self, num_pages):
        if num_pages <= 0:
            return
        total_pages = int(num_pages)
        if total_pages <= 8:
            total_pages = 8
        else:
            total_pages = 1 << (total_pages - 1).bit_length()
        logger.info("- LIF: %s Requested: %d Actual: %d page allocation in NIC HBM barmap area" %
                        (self.GID(), num_pages, total_pages))
        page_order = BitArray(length=total_pages)
        page_order.set(False)
        tbl_pos = self.hbm_tbl_pos.find(page_order)
        assert(tbl_pos)
        self.hbm_tbl_pos.set(True, range(tbl_pos[0], tbl_pos[0] + total_pages))

        return tbl_pos[0]

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

    def ConfigureRdmaLifRes(self):
        if self.enable_rdma:
            # EQID 0 on LIF is used for Async events/errors across all PDs
            self.async_eq = self.GetQ('RDMA_EQ', self.rdma_async_eq_id)
            fail = self.async_eq is None
            self.admin_eq = self.async_eq

            # Get EQID 0, CQID 0 for Admin queue AQ 0
            self.admin_cq = self.GetQ('RDMA_CQ', self.rdma_admin_cq_id)
            fail = fail or self.admin_cq is None
            # AQ position in list is different from AQ qid
            self.adminq = self.GetQ('RDMA_AQ', self.rdma_admin_aq_id)
            fail = fail or self.adminq is None
            if (fail is True):
                assert(0)

            self.obj_helper_slab.Configure()
            self.kslab_4KB.Configure()

            if len(self.obj_helper_eq.eqs):
                self.obj_helper_eq.Configure()
            halapi.ConfigureCqs([self.cq])
            halapi.ConfigureAqs([self.aq])

            logger.info("Configuring DCQCN Configs for LIF:%s" % (self.GID()))
            self.dcqcn_config_helper = dcqcn.RdmaDcqcnProfileObjectHelper()
            self.dcqcn_config_helper.Generate(self, self.dcqcn_config_spec)
            self.dcqcn_config_helper.Configure()
        return 0

    def Show(self):
        logger.info("- LIF   : %s" % self.GID())
        logger.info("  - # Queue Types    : %d" % len(self.obj_helper_q.queue_types))

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.lif_id = self.id
        req_spec.admin_status = self.status
        req_spec.enable_rdma = self.enable_rdma
        req_spec.rdma_max_keys = self.rdma_max_keys
        req_spec.rdma_max_ahs = self.rdma_max_ahs
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
        req_spec.rss.type = self.rss_type
        req_spec.rss.key = bytes(self.rss_key)
        req_spec.rss.indir = bytes(self.rss_indir)

        req_spec.enable_nvme = self.enable_nvme
        if self.enable_nvme:
            req_spec.nvme_max_ns = self.spec.nvme.max_ns
            req_spec.nvme_max_sess = self.spec.nvme.max_sess
            req_spec.nvme_host_page_size = self.spec.nvme.host_page_size
        else:
            req_spec.nvme_max_ns = 0
            req_spec.nvme_max_sess = 0
            req_spec.nvme_host_page_size = 0

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
               self.rdma_dcqcn_profile_base_addr = resp_spec.rdma_data.dcqcn_profile_base_addr
               self.rdma_at_base_addr = resp_spec.rdma_data.at_base_addr
               self.hbm_barmap_base = resp_spec.rdma_data.barmap_base_addr
           logger.info("- RDMA-DATA: LIF %s =  HW_LIF_ID = %s %s= 0x%x %s= 0x%x %s= 0x%x %s= 0x%x %s= 0x%x" %
                   (self.GID(), self.hw_lif_id,
                       'PT-Base-Addr', self.rdma_pt_base_addr,
                       'KT-Base-Addr', self.rdma_kt_base_addr,
                       'DCQCN-Prof-Base-Addr', self.rdma_dcqcn_profile_base_addr,
                       'AT-Base-Addr', self.rdma_at_base_addr,
                       'BARMAP-Base-Addr', self.hbm_barmap_base))

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

    def RegisterPd(self, pd):
        if self.enable_rdma:
            logger.info("Registering PD: %s LIF %s" % (pd.GID(), self.GID()))
            self.pds.append(pd)

    def AddSlab(self, slab):
        self.obj_helper_slab.AddSlab(slab)
        self.slabs.Add(slab)

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
            lif.ConfigureRdmaLifRes()
            if lif.enable_nvme:
                lif.nvme_lif.Configure()
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

def GetMatchingObjects(selectors):
    lif_store =  Store.objects.GetAllByClass(LifObject)
    lifs = []
    for lif in lif_store:
        if lif.IsFilterMatch(selectors.lif):
            logger.info("Selecting LIF : %s" % lif.GID())
            lifs.append(lif)

    return lifs
