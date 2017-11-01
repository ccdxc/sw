# Flow generation configuration template.

meta:
    id: MULTICAST_SESSION_ICMPV6

proto: icmpv6

entries:
    - entry:
        label: networking
        initiator:
            type: const/130   # MLD Query
            code: const/0
            id  : const/1
        responder:
            type: const/131
            code: const/0
            id  : const/1
