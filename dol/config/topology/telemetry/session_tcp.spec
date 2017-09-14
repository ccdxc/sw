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
        initiator: 
            port : const/41000
            span :
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003

