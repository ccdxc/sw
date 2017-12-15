# Flow generation configuration template.

meta:
    id: SESSION_UDP_PROXY

proto: udp

entries:
    - entry:
        label: ipsec-proxy 
        responder:
            port : const/44444
        initiator: 
            port : const/44445

    - entry:
        label: raw-redir-known-appid
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/53 # dns port
        initiator:
            port : const/46624
