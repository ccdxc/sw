# Flow generation configuration template.

meta:
    id: SESSION_UDP_SERVICES

proto: udp

entries:
    - entry:
        label: vxlan
        responder:
            port : const/4789
        initiator: 
            port : const/0

    - entry:
        label: rdma
        responder:
            port : const/0
        initiator: 
            port : const/4791
