# Flow generation configuration template.

meta:
    id: SESSION_UDP

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/1
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/2
                    dscp_rw : const/1
                    dscp    : const/3
        initiator: 
            port : const/4
            flow_info:
                eg_qos:    
                    dscp_rw : const/1
                    dscp    : const/4

    - entry:
        label: span
        responder:
            port : const/129
            span : 
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006
        initiator: 
            port : const/47275
            span : 
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006

    - entry:
        label: recirc
        responder:
            port : const/0x704e
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/5
        initiator: 
            port : const/2

    - entry:
        label: gft_drop
        tracking: False
        timestamp: False
        responder: 
            port : const/12345
            flow_info:
                action  : drop
        initiator: 
            port : const/54321
            flow_info:
                action  : drop
