# Flow generation configuration template.
meta:
    id: ICACHE_SESSION_TCP

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        timestamp: False
        responder: 
            port : const/1
        initiator: 
            port : const/4
