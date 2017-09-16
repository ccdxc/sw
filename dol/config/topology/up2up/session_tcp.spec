# Flow generation configuration template.

meta:
    id: SESSION_TCP_UP2UP

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
        label: fte
        fte: True
        tracking: False
        timestamp: False
        responder: 
            port : const/22222
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/4
                    dscp_rw : const/1
                    dscp    : const/1
        initiator: 
            port : const/33333
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/3
                    dscp_rw : const/1
                    dscp    : const/2

