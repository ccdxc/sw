# LIF Configuration Template
# LIFx.admin.queue[1-4].ring[1-2]
meta:
    id: LIF_UPLINK

types   :
    - entry:
        type         : tx
        queues       : 1
        qstatesize   : 64
        rings        : 1
    - entry:
        type         : rx
        queues       : 1
        qstatesize   : 64
        rings        : 1
