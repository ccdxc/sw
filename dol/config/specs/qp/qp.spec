# RDMA QP Configuration Spec
meta:
    id: QP_RDMA

rc:
    count           : 4
    svc_name        : "RC"
    service         : 0
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_sges     : 4
    num_sq_wqes     : 16
    num_rrq_wqes    : 4
    num_rq_sges     : 4
    num_rq_wqes     : 64
    num_rsq_wqes    : 4
    sq_in_nic       : 0

perf_rc:
    count           : 0
    svc_name        : "RC"
    service         : 0
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_sges     : 4
    num_sq_wqes     : 8
    num_rrq_wqes    : 4
    num_rq_sges     : 4
    num_rq_wqes     : 64
    num_rsq_wqes    : 4
    sq_in_nic       : 1
 
ud:
    count           : 2
    svc_name        : "UD"
    service         : 3
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_sges     : 4
    num_sq_wqes     : 16
    num_rrq_wqes    : 4
    num_rq_sges     : 4
    num_rq_wqes     : 16
    num_rsq_wqes    : 4
    sq_in_nic       : 0

