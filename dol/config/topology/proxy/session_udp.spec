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
   
