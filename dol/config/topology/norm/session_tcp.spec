# Flow generation configuration template.

meta:
    id: SESSION_TCP_NORM

proto: tcp

entries:
    - entry:
        label: norm
        tracking: False
        timestamp: False
        responder:
            port : const/42001
        initiator:
            port : const/42002
