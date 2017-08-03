# LIF Configuration Template
# LIFx.admin.queue[1-4].ring[1-2]
meta:
    id: LIF_ETH

types   :
    - entry:
        type        : admin
        queues      : 1
        qstatesize  : 64
        rings       : 1
    - entry:
        type        : tx
        queues      : 2
        qstatesize  : 64
        rings       : 1
    - entry:
        type        : rx
        queues      : 2
        qstatesize  : 64
        rings       : 1
    - entry:
        type        : cq
        queues      : 1
        qstatesize  : 64
        rings       : 1
    - entry:
        type        : eq
        queues      : 1
        qstatesize  : 64
        rings       : 1

