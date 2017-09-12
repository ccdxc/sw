# Flow generation configuration template.

meta:
    id: SESSION_ICMP

proto: icmp

entries:
    - entry:
        label: networking
        initiator:
            type: const/8   # Echo
            code: const/0
            id  : const/1
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/6
                    dscp_rw : const/1
                    dscp    : const/5
        responder:
            type: const/0   # Echo Reply
            code: const/0
            id  : const/1
    
    - entry:
        label: span
        initiator:
            type: const/8   # Echo
            code: const/0
            id  : const/2
            span:
                ingress:
                    - ref://store/objects/id=SpanSession0001
        responder:
            type: const/0   # Echo Reply
            code: const/0
            id  : const/2
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0002

    - entry:
        label: gft_drop
        tracking: False
        timestamp: False
        responder:
            type: const/8   # Echo
            code: const/0
            id  : const/1234
            flow_info:
                action: drop
        initiator: 
            type: const/0   # Echo Reply
            code: const/0
            id  : const/1234
            flow_info:
                action: drop
