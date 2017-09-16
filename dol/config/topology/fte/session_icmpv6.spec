# Flow generation configuration template.

meta:
    id: SESSION_ICMPV6_FTE

proto: icmpv6

entries:
    - entry:
        label: fte
        fte: True
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

