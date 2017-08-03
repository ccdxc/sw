# Flow generation configuration template.

meta:
    id: SESSION_TCP_PROXY_BASIC

proto: tcp

entries:
    - entry:
        responder: const/80
        initiator: random/40000/48000

    - entry:
        responder: const/443
        initiator: random/32000/40000
