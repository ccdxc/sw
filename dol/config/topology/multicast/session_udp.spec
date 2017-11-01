# Flow generation configuration template.

meta:
    id: MULTICAST_SESSION_UDP

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/128
        initiator: 
            port : const/47273

