# Flow generation configuration template.

meta:
    id: SESSION_UDP_FP

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/128
        initiator: 
            port : const/47273

