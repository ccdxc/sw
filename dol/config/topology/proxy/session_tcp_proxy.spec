# Flow generation configuration template.
meta:
    id: SESSION_TCP_PROXY

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

    - entry:
        label: raw-redir
        tracking: False
        timestamp: False
        responder: 
            port : const/23767
        initiator: 
            port : const/23768

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/80
        initiator: 
            port : const/47802
