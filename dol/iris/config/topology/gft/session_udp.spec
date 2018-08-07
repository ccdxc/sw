# Flow generation configuration template.

meta:
    id: SESSION_UDP_GFT

proto: udp

entries:
    - entry:
        label: GFT
        responder:
            port : const/40001
        initiator: 
            port : const/41001

