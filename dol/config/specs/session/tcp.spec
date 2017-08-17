# Flow generation configuration template.
meta:
    id: SESSION_TCP

proto: tcp

entries:
    - entry:
        label: networking
        responder: 
            port : const/80
        initiator: 
            port : const/42000

    - entry:
        label: span
        responder: 
            port : const/81
            span : ref://store/objects/id=SpanSession0002
        initiator: 
            port : const/42001
            span : ref://store/objects/id=SpanSession0002
