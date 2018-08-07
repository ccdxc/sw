# Flow generation configuration template.

meta:
    id: SESSION_UDP_NORM

proto: udp

entries:
    - entry:
        label: norm
        responder:
            port : const/128
        initiator: 
            port : const/47273

