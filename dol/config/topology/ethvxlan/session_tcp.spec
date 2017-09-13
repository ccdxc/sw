# Flow generation configuration template.

meta:
    id: SESSION_TCP_ETH_VXLAN

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
