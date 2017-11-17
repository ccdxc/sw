meta:
    id: SESSION_IPFIX

proto: udp

entries:
    - entry:
        label: ipfix
        fte: True
        responder:
            port : const/4739
        initiator:
            port : const/32007
