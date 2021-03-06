# LIF Configuration Template
meta:
    id: LIF_ETH

queue_types:
    - queue_type:
        id          : RX
        type        : 0
        purpose     : LIF_QUEUE_PURPOSE_RX
        size        : 64     # Size of Qstate
        count       : 1     # Number of queues of this type
        queues:
            - queue:
                size        : 4096    # Number of descriptors in each ring
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
        size        : 128     # Size of Qstate
        count       : 1       # Number of queues of this type
        queues:
            - queue:
                size        : 4096    # Number of descriptors in each ring
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
        size        : 128    # Size of Qstate
        count       : 1     # Number of queues of this type
        queues:
            - queue:
                size        : 8    # Number of descriptors in each ring
                rings:
                    - ring:
                        id          : R0
                        desc        : ref://factory/templates/id=DESCR_ADMIN

                    - ring:
                        id          : R1
                        desc        : ref://factory/templates/id=DESCR_ADMIN_CQ

    # DOL ONLY: Nicmgr will NOT allocate Eth EQ qtype in the lif qstate map
    - queue_type:
        id          : EQ
        type        : 3
        purpose     : LIF_QUEUE_PURPOSE_EQ
        size        : 64    # EQ is 16B, but min size in qstate map is 64B
        count       : 1     # Number of queues of this type
        queues:
            - queue:
                size        : 8    # Number of descriptors in each ring
                rings:
                    - ring:
                        id          : R1
                        desc        : ref://factory/templates/id=DESCR_ETH_EQ
