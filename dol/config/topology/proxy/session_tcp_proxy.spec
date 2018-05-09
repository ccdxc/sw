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
        label: tcp-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/80
        initiator: 
            port : const/47274

    - entry:
        label: tcp-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/80
        initiator: 
            port : const/47275

    - entry:
        label: tcp-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/80
        initiator: 
            port : const/47276

    - entry:
        label: tcp-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/80
        initiator: 
            port : const/47277

    - entry:
        label: ipsec-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/44444
        initiator: 
            port : const/44445

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
        tls_sess_profile: ref://factory/templates/id=TLS_PROXY_SESS_PROFILE_RSA
        responder: 
            port : const/83
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        tls_sess_profile: ref://factory/templates/id=TLS_PROXY_SESS_PROFILE_ECDSA
        responder: 
            port : const/84
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        tls_sess_profile: ref://factory/templates/id=TLS_PROXY_SESS_PROFILE_ECDSA
        responder: 
            port : const/85
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        tls_sess_profile: ref://factory/templates/id=TLS_PROXY_SESS_PROFILE_RSA_SERVER
        responder: 
            port : const/86
        initiator: 
            port : const/47802

    - entry:
        label: tcp-proxy-e2e
        tracking: False
        timestamp: False
        fte: True
        tls_sess_profile: ref://factory/templates/id=TLS_PROXY_SESS_PROFILE_ECDSA_SERVER
        responder: 
            port : const/87
        initiator: 
            port : const/47802

    - entry:
        label: proxy-redir-e2e
        tracking: False
        timestamp: False
        fte: True
        responder:
            port : const/89
        initiator:
            port : const/47802

    - entry:
        label: proxy-redir-span-e2e
        tracking: False
        timestamp: False
        fte: True
        responder:
            port : const/8089
        initiator:
            port : const/47802

