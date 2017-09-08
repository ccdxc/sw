# Flow generation configuration template.

meta:
    id: SESSION_STORAGE

proto: udp

entries:
    - entry:
        label: rdma
        responder:
            port : const/0
        initiator: 
            port : const/4791
