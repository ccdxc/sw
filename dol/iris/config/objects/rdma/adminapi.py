from infra.factory.store    import FactoryStore
from infra.common.logging   import logger
from iris.config.objects.rdma.keytable import *

class RdmaAdminCreateBase:
    def __init__(self):
        self.op = 0
        self.type_state = 0
        self.dbid_flags = 0
        self.id_ver = 0

class RdmaAdminCreateCq(RdmaAdminCreateBase):
    def __init__(self):
        super().__init__()
        self.eq_id = 0
        self.depth_log2 = 0
        self.stride_log2 = 0
        self.page_size_log2 = 0
        self.tbl_index = 0
        self.map_count = 0
        self.dma_addr = 0

class RdmaAdminCreateMr(RdmaAdminCreateBase):
    def __init__(self):
        super().__init__()
        self.va = 0
        self.length = 0
        self.pd_id = 0
        self.access_flags = 0
        self.dir_size_log2 = 0
        self.page_size_log2 = 0
        self.tbl_index = 0
        self.map_count = 0
        self.dma_addr = 0

class RdmaAdminCreateQp(RdmaAdminCreateBase):
    def __init__(self):
        self.pd_id = 0
        self.access_perms_flags = 0
        self.sq_cq_id = 0
        self.sq_depth_log2 = 0
        self.sq_stride_log2 = 0
        self.sq_page_size_log2 = 0
        self.sq_tbl_index_xrcd_id = 0
        self.sq_map_count = 0
        self.sq_dma_addr = 0
        self.rq_cq_id = 0
        self.rq_depth_log2 = 0
        self.rq_stride_log2 = 0
        self.rq_page_size_log2 = 0
        self.rq_tbl_index_srq_id = 0
        self.rq_map_count = 0
        self.rq_dma_add = 0

class RdmaAdminModQp(RdmaAdminCreateBase):
    def __init__(self):
        self.attr_mask = 0
        self.access_flags = 0
        self.rq_psn = 0
        self.sq_psn = 0
        self.qkey_dest_qpn = 0
        self.rate_limit_kbps = 0
        self.pmtu = 0
        self.retry = 0
        self.rnr_timer = 0
        self.retry_timeout = 0
        self.rsq_depth = 0
        self.rrq_depth = 0
        self.pkey_id = 0
        self.ah_id_len = 0
        self.rsvd = 0
        self.rrq_index = 0
        self.rsq_index = 0
        self.dma_addr = 0

def ConfigureCqs(lif, objlist):
    lif.aq.aq.qstate.Read()
    num_wqes = GetAQRemainingLength(lif.aq.aq.qstate.data)

    for i in range(0, len(objlist), num_wqes):
        end = i + num_wqes
        if end > len(objlist):
            end = len(objlist)

        cqs = objlist[i : end]
        ConfigureCq(lif, cqs)

    return

def ConfigureCq(lif, cqs):
    # Create and INIT Admin WQE
    template = FactoryStore.templates.Get('DESC_RDMA_AQ')
    wqe = template.CreateObjectInstance()
    obj = RdmaAdminCreateCq()
    # Create and INIT Admin CQE
    template = FactoryStore.templates.Get('DESC_RDMA_CQ_ADMIN')
    cqe = template.CreateObjectInstance()
    for cq in cqs:
        cq.PrepareAdminRequestSpec(obj)
        wqe.InitCQ(obj)
        PostAdminWqe(lif.aq, wqe)
    RingDoorbell(lif.aq.aq, 'AQ')
    for cq in cqs:
        cq.SetupRings()
        ConsumeAdminCqe(lif.aq, cqe)
    RingDoorbell(lif.aq.cq, 'CQ')

    return

def ConfigureMrs(lif, objlist):
    lif.aq.aq.qstate.Read()
    num_wqes = GetAQRemainingLength(lif.aq.aq.qstate.data)

    for i in range(0, len(objlist), num_wqes):
        end = i + num_wqes
        if end > len(objlist):
            end = len(objlist)

        mrs = objlist[i : end]
        ConfigureMr(lif, mrs)

    return

def ConfigureMr(lif, mrs):
    # Create and INIT Admin WQE
    template = FactoryStore.templates.Get('DESC_RDMA_AQ')
    wqe = template.CreateObjectInstance()
    obj = RdmaAdminCreateMr()
    # Create and INIT Admin CQE
    template = FactoryStore.templates.Get('DESC_RDMA_CQ_ADMIN')
    cqe = template.CreateObjectInstance()
    for mr in mrs:
        mr.PrepareAdminRequestSpec(obj)
        wqe.InitMR(obj)
        PostAdminWqe(lif.aq, wqe)
    RingDoorbell(lif.aq.aq, 'AQ')
    for mr in mrs:
        ConsumeAdminCqe(lif.aq, cqe)
    RingDoorbell(lif.aq.cq, 'CQ')

    return

def ConfigureQps(lif, objlist):
    lif.aq.aq.qstate.Read()
    num_wqes = GetAQRemainingLength(lif.aq.aq.qstate.data)

    for i in range(0, len(objlist), num_wqes):
        end = i + num_wqes
        if end > len(objlist):
            end = len(objlist)

        qps = objlist[i : end]
        ConfigureQp(lif, qps)

    return

def ConfigureQp(lif, qps):
    # Create and INIT Admin WQE
    template = FactoryStore.templates.Get('DESC_RDMA_AQ')
    wqe = template.CreateObjectInstance()
    obj = RdmaAdminCreateQp()
    # Create and INIT Admin CQE
    template = FactoryStore.templates.Get('DESC_RDMA_CQ_ADMIN')
    cqe = template.CreateObjectInstance()
    for qp in qps:
        qp.PrepareAdminRequestSpec(obj)
        wqe.InitQP(obj)
        PostAdminWqe(lif.aq, wqe)
    RingDoorbell(lif.aq.aq, 'AQ')
    for qp in qps:
        qp.SetupRings()
        ConsumeAdminCqe(lif.aq, cqe)
    RingDoorbell(lif.aq.cq, 'CQ')

    return

def ModifyQps(lif, objlist):
    lif.aq.aq.qstate.Read()
    num_wqes = GetAQRemainingLength(lif.aq.aq.qstate.data)

    for i in range(0, len(objlist), num_wqes):
        end = i + num_wqes
        if end > len(objlist):
            end = len(objlist)

        qps = objlist[i : end]
        ModifyQp(lif, qps)

    return

def ModifyQp(lif, qps):
    # Create and INIT Admin WQE
    template = FactoryStore.templates.Get('DESC_RDMA_AQ')
    wqe = template.CreateObjectInstance()
    obj = RdmaAdminModQp()
    # Create and INIT Admin CQE
    template = FactoryStore.templates.Get('DESC_RDMA_CQ_ADMIN')
    cqe = template.CreateObjectInstance()
    for qp in qps:
        qp.PrepareModAdminRequestSpec(obj)
        wqe.InitModQP(obj)
        PostAdminWqe(lif.aq, wqe)
    RingDoorbell(lif.aq.aq, 'AQ')
    for qp in qps:
        ConsumeAdminCqe(lif.aq, cqe)
    RingDoorbell(lif.aq.cq, 'CQ')

    return

def PostAdminWqe(aq, wqe):
    ring = aq.aq.rings.Get('AQ')
    ring.Post(wqe, debug=False)

def RingDoorbell(queue, key):
    if key == 'AQ':
        upd = 0xb
    elif key == 'CQ':
        upd = 0x7
    else:
        assert(0)
    ring = queue.rings.Get(key)
    ring.doorbell.Ring(upd_bits = upd)

def ConsumeAdminCqe(aq, cqe):
    ring = aq.cq.rings.Get('CQ')
    ring.Consume(cqe, debug=False)

def GetAQRemainingLength(state):
    log_num_size = getattr(state, 'log_num_size')
    mask = (2 ** log_num_size) - 1
    pindex = int(getattr(state, 'p_index0'))
    cindex = int(getattr(state, 'c_index0'))
    q_length = (pindex - cindex) & mask
    num_wqes = mask - q_length
    return num_wqes

def ValidateAdminCQChecks(prestate, state, num_completions = 1):
    log_num_cq_wqes = getattr(state, 'log_num_wqes')
    ring0_mask = (2 ** log_num_cq_wqes) - 1

    proxy_pre = getattr(prestate, 'proxy_pindex')
    proxy_post = getattr(state, 'proxy_pindex')

    cmp = (((proxy_pre + num_completions) & ring0_mask) == proxy_post)

    return cmp
