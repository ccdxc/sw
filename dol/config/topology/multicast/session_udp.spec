# Flow generation configuration template.

meta:
    id: MULTICAST_TOPO_SESSION_DUMMY

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/128
        initiator: 
            port : const/47273

