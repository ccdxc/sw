# LIF Configuration Template
meta:
    id: LIF_NVME

nvme:
    enable: True
    max_ns: 4
    max_sess: 64
    host_page_size: 4096

queue_types:
    - queue_type:
        id          : NVME_SQ
        type        : 0
        purpose     : LIF_QUEUE_PURPOSE_NVME_SQ
        size        : 64    # Size of Qstate
        count       : 8     # Number of queues of this type
        queues:
            - queue:
                id          : SQ
                count       : 8
                rings:
                    - ring:
                        id          : SQ
                        hw_ring_id  : 0
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SQ

    - queue_type:
        id          : NVME_CQ
        type        : 1
        purpose     : LIF_QUEUE_PURPOSE_NVME_CQ
        size        : 64
        count       : 8
        queues:
            - queue:
                id          : CQ
                count       : 8
                rings:
                    - ring:
                        id          : CQ
                        hw_ring_id  : 0
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_CQ

    - queue_type:
        id          : NVME_ARMQ
        type        : 2
        purpose     : LIF_QUEUE_PURPOSE_NVME_ARMQ
        size        : 64
        count       : 8
        queues:
            - queue:
                id          : ARMQ
                count       : 8
                rings:
                    - ring:
                        id          : ARMQ
                        hw_ring_id  : 0
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_ARMQ

    - queue_type:
        id          : NVME_SESS_XTS_TX
        type        : 3
        purpose     : LIF_QUEUE_PURPOSE_NVME_SESS_XTS_TX
        size        : 64
        count       : 128
        queues:
            - queue:
                id          : SESSXTSTX
                count       : 128
                rings:
                    - ring:
                        id          : PREXTS
                        hw_ring_id  : 0
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SESSQ
                    - ring:
                        id          : POSTXTS
                        hw_ring_id  : 1
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SESSQ

    - queue_type:
        id          : NVME_SESS_DGST_TX
        type        : 4
        purpose     : LIF_QUEUE_PURPOSE_NVME_SESS_DGST_TX
        size        : 64
        count       : 128
        queues:
            - queue:
                id          : SESSDGSTTX
                count       : 128
                rings:
                    - ring:
                        id          : PREDGST
                        hw_ring_id  : 0
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SESSQ
                    - ring:
                        id          : POSTDGST
                        hw_ring_id  : 1
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SESSQ

    - queue_type:
        id          : NVME_SESS_XTS_RX
        type        : 5
        purpose     : LIF_QUEUE_PURPOSE_NVME_SESS_XTS_RX
        size        : 64
        count       : 128
        queues:
            - queue:
                id          : SESSXTSRX
                count       : 128
                rings:
                    - ring:
                        id          : PREXTS
                        hw_ring_id  : 0
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SESSQ
                    - ring:
                        id          : POSTXTS
                        hw_ring_id  : 1
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SESSQ

    - queue_type:
        id          : NVME_SESS_DGST_RX
        type        : 6
        purpose     : LIF_QUEUE_PURPOSE_NVME_SESS_DGST_RX
        size        : 64
        count       : 128
        queues:
            - queue:
                id          : SESSDGSTRX
                count       : 128
                rings:
                    - ring:
                        id          : PREDGST
                        hw_ring_id  : 0
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_TCPRQ
                    - ring:
                        id          : POSTDGST
                        hw_ring_id  : 1
                        pi          : 0
                        ci          : 0
                        size        : 32
                        desc        : ref://factory/templates/id=DESC_NVME_SESSQ
