# Flow generation configuration template.

meta:
    id: SESSION_UDP_UP2UP

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/128
        initiator: 
            port : const/47273

    - entry:
        label: gft_drop
        responder:
            port : const/128
            flow_info:
                action: drop
        initiator: 
            port : const/47274
            flow_info:
                action: drop

