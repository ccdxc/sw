# Flow generation configuration template.

meta:
    id: SESSION_ICMPV6_TELEMETRY

proto: icmpv6

entries:
    - entry:
        label: span
        initiator:
            type: const/128   # Echo
            code: const/0
            id  : const/2
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006
        responder:
            type: const/129   # Echo Reply
            code: const/0
            id  : const/2
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006

