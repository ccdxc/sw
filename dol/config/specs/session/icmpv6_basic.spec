# Flow generation configuration template.

meta:
    id: SESSION_ICMPV6_BASIC

proto: icmpv6

entries:
    - entry:
        initiator:
            type: const/8   # Echo
            code: const/0
            id  : const/1
        responder:
            type: const/0   # Echo Reply
            code: const/0
            id  : const/1
        span: erspan
