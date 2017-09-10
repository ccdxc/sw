# Flow generation configuration template.

meta:
    id: SESSION_UDP_RDMA

proto: udp

entries:
    - entry:
        label: rdma
        responder:
            port : const/4791
        initiator: 
            port : const/0
