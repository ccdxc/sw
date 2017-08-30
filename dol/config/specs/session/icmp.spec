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
