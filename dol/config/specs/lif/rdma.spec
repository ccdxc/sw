# LIF Configuration Template
# LIFx.admin.queue[1-4].ring[1-2]
meta:
    id: LIF_RDMA

types   :
    - entry:
        type        : rdma_sq
        queues      : 16
        qstatesize  : 128
        rings       : 4
    - entry:
        type        : rdma_rq
        queues      : 32
        qstatesize  : 128
        rings       : 4
    - entry:
        type        : rdma_cq
        queues      : 16
        qstatesize  : 64
        rings       : 1
    - entry:
        type        : rdma_eq
        queues      : 16
        qstatesize  : 64
        rings       : 1
