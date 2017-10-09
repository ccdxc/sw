# RDMA QP Configuration Spec
meta:
    id: QP_RDMA


count           : 4
hostmem_pg_size : 4096
pmtu            : 1024
atomic_enabled  : True
num_sq_sges     : 2
num_sq_wqes     : 16
num_rrq_wqes    : 4
num_rq_sges     : 2
num_rq_wqes     : 64
num_rsq_wqes    : 4
