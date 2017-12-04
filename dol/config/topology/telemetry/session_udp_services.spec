# Flow generation configuration template.

meta:
    id: SESSION_UDP_SERVICES_TELEMETRY

proto: udp

entries:
    - entry:
        label: vxlan
        responder:
            port : const/4789
        initiator: 
            port : const/0
