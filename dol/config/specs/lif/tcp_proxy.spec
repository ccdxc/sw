# LIF Configuration Template
# LIFx.admin.queue[1-4].ring[1-2]
meta:
    id: LIF_TCP_PROXY

types   :
    - entry:
        type        : tcp_cb
        queues      : 4
        qstatesize  : 512
        rings       : 1
