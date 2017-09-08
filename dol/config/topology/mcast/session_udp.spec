# Flow generation configuration template.

meta:
    id: MCAST_TOPO_SESSION_DUMMY

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/128
        initiator: 
            port : const/47273

