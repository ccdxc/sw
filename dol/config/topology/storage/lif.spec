# LIF Configuration Template
meta:
    id: LIF_STORAGE

c_lib: storage_test

queue_types:
    - queue_type:
        id          : NVME_SQ
        type        : 0
        purpose     : LIF_QUEUE_PURPOSE_NVME
        size        : 64
        count       : 8

    - queue_type:
        id          : NVME_CQ
        type        : 1
        purpose     : LIF_QUEUE_PURPOSE_NVME
        size        : 64
        count       : 8
