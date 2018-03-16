# Flow generation configuration template.
meta:
    id: SESSION_TCP_PROXY_VXLAN

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

