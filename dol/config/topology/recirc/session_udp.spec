# Flow generation configuration template.

meta:
    id: SESSION_UDP_RECIRC

proto: udp

entries:
    - entry:
        label     : recirc
        tracking  : False
        initiator : 
            hashgen  : True
            port     : const/0
        responder : 
            port     : const/0
