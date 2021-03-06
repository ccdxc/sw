# Flow generation configuration template.
meta:
    id: SESSION_TCP_APP_REDIR

proto: tcp

entries:
    - entry:
        label: raw-redir
        tracking: False
        timestamp: False
        responder: 
            port : const/23767
        initiator: 
            port : const/23768

    - entry:
        label: raw-redir-span
        tracking: False
        timestamp: False
        responder: 
            port : const/23765
        initiator: 
            port : const/23766

    - entry:
        label: raw-redir-flow-miss
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/19694
        initiator: 
            port : const/19695

    - entry:
        label: raw-redir-span-flow-miss
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/19696
        initiator: 
            port : const/19697

    - entry:
        label: raw-redir-known-appid
        tracking: False
        timestamp: False
        fte: True
        responder: 
            port : const/3307 # mysql port
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

#Enable when L7 support is reinstated in P4+ TCP Proxy
#    - entry:
#        label: proxy-redir
#        tracking: False
#        timestamp: False
#        responder: 
#            port : const/23763
#        initiator: 
#            port : const/23764
#
#    - entry:
#        label: proxy-redir-span
#        tracking: False
#        timestamp: False
#        responder: 
#            port : const/23761
#        initiator: 
#            port : const/23762
#
#    - entry:
#        label: proxy-redir-e2e
#        tracking: False
#        timestamp: False
#        fte: True
#        responder:
#            port : const/89
#        initiator:
#            port : const/47802
#
#    - entry:
#        label: proxy-redir-span-e2e
#        tracking: False
#        timestamp: False
#        fte: True
#        responder:
#            port : const/8089
#        initiator:
#            port : const/47802

