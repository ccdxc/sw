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
        label: raw-redir-multi
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/19694
        initiator: 
            port : const/19695

    - entry:
        label: raw-redir-known-appid
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/3306 # mysql port
        initiator:
            port : const/46624

    - entry:
        label: raw-redir-known-appid
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/2501 # kismet port
        initiator:
            port : const/46624

    - entry:
        label: proxy-redir-dec
        tracking: False
        timestamp: False
        responder: 
            port : const/23763
        initiator: 
            port : const/23764

    - entry:
        label: proxy-redir-enc
        tracking: False
        timestamp: False
        responder: 
            port : const/23765
        initiator: 
            port : const/23766

    - entry:
        label: p4pt
        tracking: False
        timestamp: False
        responder:
            port : const/3260
        initiator:
            port : const/45443

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/80
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/81
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/82
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/83
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/84
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/85
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/86
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/87
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/88
        initiator: 
            port : const/47802
