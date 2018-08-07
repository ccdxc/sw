# Flow generation configuration template.

meta:
    id: BASE_SESSION_UDP

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/1
        initiator: 
            port : const/4
