# Flow generation configuration template.
meta:
    id: SESSION_ICMP_NORM

proto: icmp

entries:
    - entry:
        label: norm
        initiator:
            type: const/8   # Echo
            code: const/0
            id  : const/1
        responder:
            type: const/0   # Echo Reply
            code: const/0
            id  : const/1

    - entry:
        label: norm
        initiator:
            type: const/4   # Deprecated msg type
            code: const/0
            id  : const/0
        responder:
            type: const/6   # Deprecated msg type
            code: const/0
            id  : const/0

    - entry:
        label: norm
        initiator:
            type: const/15   # Deprecated msg type
            code: const/0
            id  : const/0
        responder:
            type: const/39   # Deprecated msg type
            code: const/0
            id  : const/0

    - entry:
        label: norm
        initiator:
            type: const/27   # Deprecated msg type
            code: const/0
            id  : const/0
        responder:
            type: const/28   # Deprecated msg type
            code: const/0
            id  : const/0

    - entry:
        label: norm
        initiator:
            type: const/5   # ICMP redirect
            code: const/0
            id  : const/0
        responder:
            type: const/5   # ICMP redirect
            code: const/1
            id  : const/0

    - entry:
        label: norm
        initiator:
            type: const/8   # Echo with invalid code
            code: const/1
            id  : const/0
        responder:
            type: const/0   # Echo reply with invalid code
            code: const/1
            id  : const/0

