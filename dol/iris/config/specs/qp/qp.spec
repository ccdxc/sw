# RDMA QP Configuration Spec
meta:
    id: QP_RDMA
useAdmin: True
rc:
    count           : 5
    svc_name        : "RC"
    service         : 0
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_wqes     : 4096
    num_rrq_wqes    : 256
    num_rq_wqes     : 4096
    num_rsq_wqes    : 256
    sq_in_nic       : 0
    rq_in_nic       : 0

perf_rc:
    count           : 1
    svc_name        : "RC"
    service         : 0
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_wqes     : 8
    num_rrq_wqes    : 4
    num_rq_wqes     : 64
    num_rsq_wqes    : 4
    sq_in_nic       : 1
    rq_in_nic       : 1
 
perf_rc_sqnic:
    count           : 1
    svc_name        : "RC"
    service         : 0
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_wqes     : 8
    num_rrq_wqes    : 4
    num_rq_wqes     : 2048
    num_rsq_wqes    : 128
    sq_in_nic       : 1
    rq_in_nic       : 0
 
perf_rc_rqnic:
    count           : 1
    svc_name        : "RC"
    service         : 0
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_wqes     : 2048
    num_rrq_wqes    : 128
    num_rq_wqes     : 64
    num_rsq_wqes    : 4
    sq_in_nic       : 0
    rq_in_nic       : 1
 
ud:
    count           : 2
    svc_name        : "UD"
    service         : 3
    hostmem_pg_size : 4096
    pmtu            : 1024
    atomic_enabled  : True
    num_sq_wqes     : 16
    num_rrq_wqes    : 4
    num_rq_wqes     : 16
    num_rsq_wqes    : 4
    sq_in_nic       : 0
    rq_in_nic       : 0

