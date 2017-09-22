# LIF Configuration Template
meta:
    id: LIF_PROXY
    max_pt_entries: 0
    max_keys: 0

queue_types:
    - queue_type:
        id          : TX
        type        : 2
        purpose     : LIF_QUEUE_PURPOSE_TX
        size        : 64
        count       : 16
        queues:
            - queue:
                id          : Q0
                count       : 16
                template    : ref://store/templates/id=ETH_QUEUE_TX
                rings:
                    - ring:
                        id          : R0
                        pi          : 0
                        ci          : 0
                        size        : 1024
                        template    : ref://store/templates/id=ETH_RING_TX
                        desc        : ref://factory/templates/id=DESCR_ETH_TX

    - queue_type:
        id          : RX
        type        : 4
        purpose     : LIF_QUEUE_PURPOSE_RX
        size        : 64
        count       : 16
        queues:
            - queue:
                id          : Q0
                count       : 16
                template    : ref://store/templates/id=ETH_QUEUE_RX
                rings:
                    - ring:
                        id          : R0
                        pi          : 0
                        ci          : 0
                        size        : 1024
                        template    : ref://store/templates/id=ETH_RING_RX
                        desc        : ref://factory/templates/id=DESCR_ETH_RX
