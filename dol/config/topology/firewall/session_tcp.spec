# Flow generation configuration template.

meta:
    id: SESSION_TCP_FIREWALL

proto: tcp

entries:
    - entry:
        label: firewall
        fte: True
        tracking: False
        responder:
            port : const/42001
        initiator:
            port : const/42002
