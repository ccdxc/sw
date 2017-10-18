# Flow generation configuration template.

meta:
    id: SESSION_UDP_CLASSIC

proto: udp

entries:
    - entry:
        label: classic
        fte: True
        responder:
            port : const/128
        initiator: 
            port : const/47273
