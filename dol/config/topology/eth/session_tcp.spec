# Flow generation configuration template.

meta:
    id: SESSION_TCP_ETH

proto: tcp

entries:
    - entry:
        label: networking
        tracking: False
        timestamp: False
        responder:
            port : const/42001
            flow_info:
                eg_qos:
                    cos_rw  : const/1
                    cos     : const/4
        initiator:
            port : const/42002

    - entry:
        label: gft_drop
        tracking: False
        timestamp: False
        responder:
            port : const/42003
            flow_info:
                eg_qos:
                    cos_rw  : const/1
                    cos     : const/4
        initiator:
            port : const/42004

