# Flow generation configuration template.

meta:
    id: SESSION_TCP_STATS

proto: tcp

entries:
    - entry:
        label: stats
        tracking: False
        timestamp: False
        responder: 
            port : const/22222
        initiator: 
            port : const/33333
