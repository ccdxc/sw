#! /usr/bin/python3
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.queue_type     as queue_type
import infra.clibs.clibs        as clibs

from config.store               import Store
from infra.common.logging       import cfglogger
from infra.common.glopts        import GlobalOptions

import config.hal.api            as halapi
import config.hal.defs           as haldefs

import pdb

import ctypes

class QInfoStruct(ctypes.Structure):
    _fields_ = [("dryrun", ctypes.c_bool),
                ("lif_id", ctypes.c_uint64),
                ("q0_addr", ctypes.c_uint64 * 8)]

class LifObject(base.ConfigObjectBase):
    def __init__(self, tenant, spec, namespace = None):
        super().__init__()

        if namespace:
            self.id = namespace.get()
        else:
            self.id = resmgr.LifIdAllocator.get()
        self.GID("Lif%d" % self.id)
        self.mac_addr   = resmgr.LifMacAllocator.get()
        self.status     = haldefs.interface.IF_STATUS_UP
        self.hw_lif_id = 0
        self.qstate_base = {}
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

        self.queue_types = objects.ObjectDatabase(cfglogger)
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
            self.pd_allocator = objects.TemplateFieldObject("range/0/128")
            self.mr_key_allocator = objects.TemplateFieldObject("range/0/1024")

        self.tenant     = tenant
        self.spec       = spec
        self.Show()

    def GetQpid(self):
        return self.qpid_allocator.get()

    #TODO: Until Yogesh's fix comes in for Unaligned write back, just allocate CQIDs as even number
    def GetCqid(self):
        cqid = self.cqid_allocator.get()
        self.cqid_allocator.get()
        return cqid

    def GetPd(self):
        return self.pd_allocator.get()

    def GetMrKey(self):
        return self.mr_key_allocator.get()

    def GetQ(self, type, qid):
        qt = self.queue_types.Get(type)
        if qt is not None:
            return qt.queues.Get(str(qid))

    def GetQstateAddr(self, type):
        return self.qstate_base[type]

    def ConfigureQueueTypes(self):
        if GlobalOptions.dryrun:
            return 0
        self.obj_helper_q.Configure()

    def __copy__(self):
        lif = LifObject(self.tenant, self.spec)
        lif.id = self.id
        lif.mac_addr = self.mac_addr
        lif.status = self.status
        lif.enable_rdma = self.enable_rdma
        lif.rdma_max_pt_entries = self.rdma_max_pt_entries
        lif.rdma_max_keys = self.rdma_max_keys
        lif.queue_types = self.queue_types
        lif.queue_types_list = []
        lif.queue_list = []
        for queue_type in self.queue_types.GetAll():
            lif.queue_types_list.append(copy.copy(queue_type))
        for queue in self.queue_list:
            lif.queue_list.append(copy.copy(queue))
            
        return lif
    
    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False
        fields = ["id", "mac_addr", "status", "enable_rdma", "rdma_max_pt_entries",
                   "rdma_max_keys"]
        if not self.CompareObjectFields(other, fields, lgh):
            return False
        
        for c_qtype, o_q_type in zip(self.queue_types_list, other.queue_types_list):
            if not c_qtype.Equals(o_q_type, lgh):
                lgh.error("Queue Type mismatch")
                return False
       
        return True
        

    def Show(self):
        cfglogger.info("- LIF   : %s" % self.GID())
        cfglogger.info("  - # Queue Types    : %d" % len(self.obj_helper_q.queue_types))

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.key_or_handle.lif_id = self.id
        req_spec.mac_addr = self.mac_addr.getnum()
        req_spec.admin_status = self.status
        req_spec.enable_rdma = self.enable_rdma
        req_spec.rdma_max_keys = self.rdma_max_keys
        req_spec.rdma_max_pt_entries = self.rdma_max_pt_entries
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
        self.hw_lif_id = resp_spec.hw_lif_id
        cfglogger.info("- LIF %s = %s HW_LIF_ID = %s (HDL = 0x%x)" %
                       (self.GID(),
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hw_lif_id, self.hal_handle))
        for qstate in resp_spec.qstate:
            cfglogger.info("- QUEUE_TYPE = %d QSTATE_ADDR = 0x%x" % (qstate.type_num, qstate.addr))
            self.qstate_base[qstate.type_num] = qstate.addr
        if self.enable_rdma:
           self.rdma_pt_base_addr = resp_spec.rdma_data.pt_base_addr
           self.rdma_kt_base_addr = resp_spec.rdma_data.kt_base_addr
           cfglogger.info("- LIF %s =  HW_LIF_ID = %s PT-Base-Addr = 0x%x KT-Base-Addr= 0x%x)" %
                          (self.GID(), self.hw_lif_id, self.rdma_pt_base_addr, self.rdma_kt_base_addr))


    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.key_or_handle.lif_id = self.id
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        if get_resp.api_status == haldefs.common.ApiStatus.Value('API_STATUS_OK'):
            self.mac_addr = objects.MacAddressBase(integer=get_resp.spec.mac_addr)
            self.status = get_resp.spec.admin_status
            self.enable_rdma = get_resp.spec.enable_rdma
            self.rdma_max_keys = get_resp.spec.rdma_max_keys
            self.rdma_max_pt_entries = get_resp.spec.rdma_max_pt_entries
            for qstate_spec, queue_type in zip(get_resp.spec.lif_qstate_map, self.queue_types_list):
                queue_type.ProcessHALGetResponse(qstate_spec)
            #TODO Still.
            #for q_spec, queue in zip(get_resp.spec.lif_qstate, self.queue_list):
            #    queue.ProcessHALGetResponse(q_spec)                
        else:
            self.mac_addr = None
            self.status = None
            self.enable_rdma = None
            self.rdma_max_keys = None
            self.rdma_max_pt_entries = None
            self.queue_types_list = []

    def Get(self):
        halapi.GetLifs([self])

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

    def CLibConfig(self, resp_spec):
       qaddrs_type = ctypes.c_uint64 * 8
       qaddrs = qaddrs_type()
       for qstate in resp_spec.qstate:
           qaddrs[int(qstate.type_num)] = qstate.addr
       qinfo = QInfoStruct(GlobalOptions.dryrun, resp_spec.hw_lif_id, qaddrs)
       self.c_lib_config(ctypes.byref(qinfo))


class LifObjectHelper:
    def __init__(self):
        self.lifs = []
        self.aidx = 0

    def Generate(self, tenant, spec, namespace):
        count = namespace.GetCount()
        cfglogger.info("Creating %d Lifs. for Tenant:%s" %\
                       (count, tenant.GID()))
        for l in range(count):
            lif = LifObject(tenant, spec, namespace)
            self.lifs.append(lif)

    def Configure(self):
        cfglogger.info("Configuring %d LIFs." % len(self.lifs)) 
        halapi.ConfigureLifs(self.lifs)
        for lif in self.lifs:
            lif.ConfigureQueueTypes()
        Store.objects.SetAll(self.lifs)

    def Alloc(self):
        if self.aidx >= len(self.lifs):
            assert(0)
        lif = self.lifs[self.aidx]
        self.aidx += 1
        return lif

def GetMatchingObjects(selectors):
    lifs =  Store.objects.GetAllByClass(LifObject)
    return [lif for lif in lifs if lif.IsFilterMatch(selectors.lif)]
