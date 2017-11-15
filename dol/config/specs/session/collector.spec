# Flow for IPFIX collector specification
meta:
    id: SESSION_IPFIX_COLLECTOR

proto: udp

entries:
    - entry:
        label : collector
        fte : True
        responder:
            port : const/4739
        initiator:
            port : const/32007
