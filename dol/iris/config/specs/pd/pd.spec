# RDMA PD Configuration Spec
meta:
    id: PD_RDMA


#PD-0 and hence EQ-0 is used for Async Events and not for real workload
#PD-1 and resources belonging to it (CQ 0, EQ 1) are used for RDMA Admin Queue
count   : 4
cq      : ref://store/specs/id=CQ_RDMA
eq      : ref://store/specs/id=EQ_RDMA
aq      : ref://store/specs/id=AQ_RDMA
qp      : ref://store/specs/id=QP_RDMA
mr      : ref://store/specs/id=MR_RDMA
mw      : ref://store/specs/id=MW_RDMA
key     : ref://store/specs/id=KEY_RDMA
