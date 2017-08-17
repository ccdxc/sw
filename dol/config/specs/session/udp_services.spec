# Flow generation configuration template.

meta:
    id: SESSION_UDP_SERVICES

proto: udp

entries:
    - entry:
        label: vxlan
        responder:
            port : const/4789
            span : ref://store/objects/id=SpanSession0001
        initiator: 
            port : const/0
            span : ref://store/objects/id=SpanSession0002

    - entry:
        label: rdma
        responder:
            port : const/0
            span : ref://store/objects/id=SpanSession0003
        initiator: 
            port : const/4791
            span : ref://store/objects/id=SpanSession0001
