# Flow generation configuration template.

meta:
    id: SESSION_TCP_RECIRC

proto: tcp

entries:
    - entry:
        label     : recirc
        tracking  : False
        initiator : 
            hashgen  : True
            port     : const/0
        responder : 
            port     : const/0
