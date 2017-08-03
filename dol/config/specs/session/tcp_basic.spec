# Flow generation configuration template.

meta:
    id: SESSION_TCP_BASIC

proto: tcp

entries:
    - entry:
        responder: const/80
        initiator: const/42000
