# Flow generation configuration template.

meta:
    id: SESSION_ICMPV6_UP2UP

proto: icmpv6

entries:
    - entry:
        label: networking
        initiator:
            type: const/128   # Echo
            code: const/0
            id  : const/1
        responder:
            type: const/129   # Echo Reply
            code: const/0
            id  : const/1
    
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
