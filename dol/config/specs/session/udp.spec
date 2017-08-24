# Flow generation configuration template.

meta:
    id: SESSION_UDP

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/128
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/2
                    dscp_rw : const/1
                    dscp    : const/3
        initiator: 
            port : const/47273
            flow_info:
                eg_qos:    
                    dscp_rw : const/1
                    dscp    : const/4

    - entry:
        label: span
        responder:
            port : const/129
            span : ref://store/objects/id=SpanSession0001
        initiator: 
            port : const/47275
            span : ref://store/objects/id=SpanSession0002


