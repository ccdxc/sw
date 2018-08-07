# Flow generation configuration template.

meta:
    id: SESSION_TCP_FTE

proto: tcp

entries:
    - entry:
        label: fte
        fte: True
        tracking: False
        timestamp: False
        responder: 
            port : const/22222
        initiator: 
            port : const/47273
