# Flow generation configuration template.

meta:
    id: SESSION_UDP_BASIC

proto: udp

entries:
    - entry:
        responder: const/128
        initiator: const/47273
        span     : rspan

    - entry:
        responder: const/129
        initiator: const/40000
        span     : rspan_vxlan

    - entry:
        responder: const/0
        initiator: const/4791
        span     : rspan

