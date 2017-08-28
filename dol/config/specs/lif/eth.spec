# LIF Configuration Template
meta:
    id: LIF_DEFAULT

rdma: True

entries:
    - entry:
        id          : TXQ
        type        : 0
        purpose     : tx
        count       : 1
        size        : 256
        ring:
            id      : R0
            count   : 1
            pi      : 0
            ci      : 0
            size    : 1024
            template: ref://factory/templates/id=DESCR_ETH_TX

    - entry:
        id          : RXQ
        type        : 1
        purpose     : rx
        count       : 1
        size        : 256
        ring:
            id      : R0
            count   : 1
            pi      : 0
            ci      : 0
            size    : 1024
            template: ref://factory/templates/id=DESCR_ETH_RX

