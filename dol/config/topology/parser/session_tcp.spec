# Flow generation configuration template.

meta:
    id: SESSION_TCP_PARSER

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42001
        initiator:
            port : const/42002

