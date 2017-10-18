# Flow generation configuration template.

meta:
    id: SESSION_ICMPV6_CLASSIC

proto: icmpv6

entries:
    - entry:
        label: classic
        fte: True
        initiator:
            type: const/128   # Echo
            code: const/0
            id  : const/1
        responder:
            type: const/129   # Echo Reply
            code: const/0
            id  : const/1

