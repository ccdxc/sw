# Flow generation configuration template.

meta:
    id: SESSION_ICMP_TELEMETRY

proto: icmp

entries:
    - entry:
        label: span
        initiator:
            type: const/8   # Echo
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
            type: const/0   # Echo Reply
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

