# Flow generation configuration template.
meta:
    id: SESSION_TCP_FP

proto: tcp

entries:
    - entry:
        label: tcp-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/80
        initiator: 
            port : const/47273

    - entry:
        label: ipsec-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/44444
        initiator: 
            port : const/44445

