# Flow generation configuration template.

meta:
    id: SESSION_TCP_UP2UP

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        responder:
            port : const/42001
        initiator:
            port : const/42002



    - entry:
        label: gft_drop
        tracking: False
        responder: 
            port : const/22222
            flow_info:
                action: drop
        initiator: 
            port : const/33333
            flow_info:
                action: drop

