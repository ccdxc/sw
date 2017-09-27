# LIF Configuration Template
meta:
    id: LIF_ETH

queue_types:
    - queue_type:
        id          : TX
        type        : 1
        purpose     : LIF_QUEUE_PURPOSE_TX
        size        : 64
        count       : 1
        queues:
            - queue:
                id          : Q0
                count       : 1
                template    : ref://store/templates/id=ETH_QUEUE_TX
                rings:
                    - ring:
                        id          : R0
                        num         : 0
                        size        : 1024
                        desc        : ref://factory/templates/id=DESCR_ETH_TX

                    - ring:
                        id          : R1
                        num         : 1
                        size        : 1024
                        desc        : ref://factory/templates/id=DESCR_ETH_TX_CQ

    - queue_type:
        id          : RX
        type        : 0
        purpose     : LIF_QUEUE_PURPOSE_RX
        size        : 64
        count       : 1
        queues:
            - queue:
                id          : Q0
                count       : 1
                template    : ref://store/templates/id=ETH_QUEUE_RX
                rings:
                    - ring:
                        id          : R0
                        num         : 0
                        size        : 1024
                        desc        : ref://factory/templates/id=DESCR_ETH_RX

                    - ring:
                        id          : R1
                        num         : 1
                        size        : 1024
                        desc        : ref://factory/templates/id=DESCR_ETH_RX_CQ

    - queue_type:
        id          : ADMIN
        type        : 2
        purpose     : LIF_QUEUE_PURPOSE_ADMIN
        size        : 64
        count       : 1
        queues:
            - queue:
                id          : Q0
                count       : 1
                template    : ref://store/templates/id=ETH_QUEUE_ADMIN
                rings:
                    - ring:
                        id          : R0
                        num         : 0
                        size        : 1024
                        desc        : ref://factory/templates/id=DESCR_ETH_ADMIN

                    - ring:
                        id          : R1
                        num         : 1
                        size        : 1024
                        desc        : ref://factory/templates/id=DESCR_ETH_ADMIN_CQ
