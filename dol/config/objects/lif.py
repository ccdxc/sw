#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects

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

class LifObject(objects.FrameworkObject):
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

        if hasattr(spec, 'rdma'):
            self.enable_rdma = spec.rdma.enable
            self.rdma_max_pt_entries = spec.rdma.max_pt_entries
            self.rdma_max_keys = spec.rdma.max_keys
        else:
            self.enable_rdma = False
            self.rdma_max_pt_entries = 0
            self.rdma_max_keys = 0

        self.queue_types = objects.ObjectDatabase(cfglogger)
        self.obj_helper_q = queue_type.QueueTypeObjectHelper()
        self.obj_helper_q.Generate(self, spec)
        self.queue_types.SetAll(self.obj_helper_q.queue_types)

        # RDMA per LIF allocators
        if self.enable_rdma:
            self.qpid_allocator = objects.TemplateFieldObject("range/0/16384")
            self.pd_allocator = objects.TemplateFieldObject("range/0/128")
            self.mr_key_allocator = objects.TemplateFieldObject("range/0/1024")

        self.tenant     = tenant
        self.Show()

    def GetQpid(self):
        return self.qpid_allocator.get()

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
        self.obj_helper_q.Configure()

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

    def Alloc(self):
        if self.aidx == (len(self.lifs) - 1):
            assert(0)
        lif = self.lifs[self.aidx]
        self.aidx += 1
        return lif
