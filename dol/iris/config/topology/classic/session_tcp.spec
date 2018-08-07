# Flow generation configuration template.

meta:
    id: SESSION_TCP_CLASSIC

proto: tcp

entries:
    - entry:
        label: classic
        tracking: False
        timestamp: False
        responder:
            port : const/42001
        initiator:
            port : const/42002
