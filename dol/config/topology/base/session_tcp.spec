# Flow generation configuration template.
meta:
    id: BASE_SESSION_TCP

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
        label: proxy-redir
        tracking: False
        timestamp: False
        responder: 
            port : const/23763
        initiator: 
            port : const/23764

    - entry:
        label: networking
        tracking: False
        timestamp: False
        responder: 
            port : const/1
        initiator: 
            port : const/4
