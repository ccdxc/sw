# Flow generation configuration template.

meta:
    id: SESSION_TCP_GFT

proto: tcp

entries:
    - entry:
        label: gft
        tracking: False
        timestamp: False
        responder:
            port : const/42001
        initiator:
            port : const/42002
