# Flow generation configuration template.

meta:
    id: SESSION_UDP

proto: udp

entries:
    - entry:
        label: networking
        responder:
            port : const/128
        initiator: 
            port : const/47273

    - entry:
        label: span
        responder:
            port : const/129
            span : ref://store/objects/id=SpanSession0001
        initiator: 
            port : const/47275
            span : ref://store/objects/id=SpanSession0002


