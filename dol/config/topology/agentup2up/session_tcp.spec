# Flow generation configuration template.

meta:
    id: SESSION_TCP_AGENT_UP2UP

proto: tcp

entries:
    - entry:
        label   : networking
        fte     : True
        tracking: False
        responder:
            port : const/42001
        initiator:
            port : const/42002

