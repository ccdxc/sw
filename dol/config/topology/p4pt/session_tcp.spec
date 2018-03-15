# Flow generation configuration template.
meta:
    id: SESSION_TCP_P4PT

proto: tcp

entries:
    - entry:
        label: p4pt
        tracking: False
        timestamp: False
        responder:
            port : const/3260
        initiator:
            port : const/45443

