# Flow generation configuration template.

meta:
    id: SESSION_TCP_TELEMETRY

proto: tcp

entries:
    - entry:
        label: span
        tracking: False
        timestamp: False
        responder: 
            port : const/81
            span :
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006
        initiator: 
            port : const/41000
            span :
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006

