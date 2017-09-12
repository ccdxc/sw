# Flow generation configuration template.

meta:
    id: SESSION_ICMPV6

proto: icmpv6

entries:
    - entry:
        label: networking
        initiator:
            type: const/128   # Echo
            code: const/0
            id  : const/1
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/0
        responder:
            type: const/129   # Echo Reply
            code: const/0
            id  : const/1
            flow_info:
                eg_qos:    
                    dscp_rw : const/1
                    dscp    : const/4

    - entry:
        label: span
        initiator:
            type: const/128   # Echo
            code: const/0
            id  : const/2
            span: 
                ingress: 
                    - ref://store/objects/id=SpanSession0003
        responder:
            type: const/129   # Echo Reply
            code: const/0
            id  : const/2
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0003

    - entry:
        label: gft_drop
        tracking: False
        timestamp: False
        responder:
            type: const/128   # Echo
            code: const/0
            id  : const/1234
            flow_info:
                action: drop
        initiator: 
            type: const/129   # Echo Reply
            code: const/0
            id  : const/1234
            flow_info:
                action: drop
