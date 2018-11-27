# RDMA CQ Configuration Spec
meta:
    id: CQ_RDMA

# cq_count >= pd_count * num_qp_per_pd
count           : 15
hostmem_pg_size : 4096
num_wqes        : 16
useAdmin        : True
