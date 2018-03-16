# Flow generation configuration template.

meta:
    id: SESSION_UDP_ALG

proto: udp

entries:
    - entry:
        label: fte
        fte: True
        responder:
            port : const/128
        initiator: 
            port : const/47273

