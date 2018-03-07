# Flow generation configuration template.

meta:
    id: SESSION_TCP_CACHEPERF

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42001
        initiator:
            port : const/43001
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42002
        initiator:
            port : const/43002
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42003
        initiator:
            port : const/43003
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42004
        initiator:
            port : const/43004
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42005
        initiator:
            port : const/43005
