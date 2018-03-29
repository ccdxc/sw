# Flow generation configuration template.

meta:
    id: SESSION_UDP_STATS

proto: udp

entries:
    - entry:
        label: stats
        responder:
            port : const/128
        initiator: 
            port : const/47273

