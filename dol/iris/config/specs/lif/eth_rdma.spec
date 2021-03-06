# LIF Configuration Template
meta:
    id: LIF_ETH_RDMA

rdma:
    enable: True
# max_pt_entries depends on slab-size and allignment. Please check HAL logs for rdma_memory_register to get this.
    max_pt_entries: 131072
    max_keys: 8192
    max_ahs: 1024
    # Allocating 8MB for hbm barmap size
    hbm_barmap_size: 8388608
    # These values must match the corresponding values defined in the queue_types below
    max_aq: 1
    max_aqe: 1024

    max_qp: 256

    max_cq: 32
    max_cqe: 2048
# Idea is to allocate one eq for every 8 CQs + 1 (Async EQ)
    max_eq: 8
    max_eqe: 256

    max_mr: 16384
    max_pd: 256
    hostmem_pg_size: 4096
    slab    : ref://store/specs/id=SLAB_RDMA
    dcqcn_config    : ref://store/specs/id=DCQCN_CONFIG_RDMA

queue_types:
    - queue_type:
        id          : RX
        type        : 0
        purpose     : LIF_QUEUE_PURPOSE_RX
        size        : 64    # Size of Qstate
        count       : 1     # Number of queues of this type
        queues:
            - queue:
                size        : 256    # Number of descriptors in each ring
                rings:
                    - ring:
                        id          : R0
                        buf         : ref://factory/templates/id=ETH_BUFFER
                        desc        : ref://factory/templates/id=DESCR_ETH_RX

                    - ring:
                        id          : R1
                        desc        : ref://factory/templates/id=DESCR_ETH_RX_CQ

    - queue_type:
        id          : TX
        type        : 1
        purpose     : LIF_QUEUE_PURPOSE_TX
        size        : 128    # Size of Qstate
        count       : 1     # Number of queues of this type
        queues:
            - queue:
                size        : 256    # Number of descriptors in each ring
                rings:
                    - ring:
                        id          : R0
                        buf         : ref://factory/templates/id=ETH_BUFFER
                        desc        : ref://factory/templates/id=DESCR_ETH_TX

                    - ring:
                        id          : R1
                        desc        : ref://factory/templates/id=DESCR_ETH_TX_CQ

    - queue_type:
        id          : ADMIN
        type        : 2
        purpose     : LIF_QUEUE_PURPOSE_ADMIN
        size        : 128   # Size of Qstate
        count       : 1     # Number of queues of this type
        queues:
            - queue:
                #id          : ETH
                size        : 256    # Number of descriptors in each ring
                rings:
                    - ring:
                        id          : R0
                        desc        : ref://factory/templates/id=DESCR_ADMIN

                    - ring:
                        id          : R1
                        desc        : ref://factory/templates/id=DESCR_ADMIN_CQ

    - queue_type:
        id          : RDMA_AQ
        type        : 2
        purpose     : LIF_QUEUE_PURPOSE_ADMIN
        size        : 128  # Size of Qstate
        count       : 32   # Number of queues of this type
        queues:
            - queue:
                id          : RDMA
                count       : 1
                rings:
                    - ring:
                        id          : AQ
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_AQ

    - queue_type:
        id          : RDMA_SQ
        type        : 3
        purpose     : LIF_QUEUE_PURPOSE_RDMA_SEND
        size        : 512
        count       : 32
        queues:
            - queue:
                id          : Q0
                count       : 32
                rings:
                    - ring:
                        id          : SQ
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_SQ
                    - ring:
                        id          : RRQ
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_RRQ

    - queue_type:
        id          : RDMA_RQ
        type        : 4
        purpose     : LIF_QUEUE_PURPOSE_RDMA_RECV
        size        : 512
        count       : 32
        queues:
            - queue:
                id          : Q0
                count       : 32
                rings:
                    - ring:
                        id          : RQ
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_RQ
                    - ring:
                        id          : RSQ
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_RSQ

    - queue_type:
        id          : RDMA_CQ
        type        : 5
        purpose     : LIF_QUEUE_PURPOSE_CQ
        size        : 64
        count       : 32
        queues:
            - queue:
                id          : Q0
                count       : 32
                rings:
                    - ring:
                        id          : CQ
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_CQ
                    - ring:
                        id          : CQ_ARM
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_CQ
                    - ring:
                        id          : CQ_SARM
                        pi          : 0
                        ci          : 0
                        size        : 0
                        desc        : ref://factory/templates/id=DESC_RDMA_CQ

    - queue_type:
        id          : RDMA_EQ
        type        : 6
        purpose     : LIF_QUEUE_PURPOSE_EQ
        size        : 64
        count       : 8
        queues:
            - queue:
                id          : Q0
                count       : 8
                rings:
                    - ring:
                        id          : EQ
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_RDMA_EQ
