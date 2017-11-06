# L4LB Flow generation spec
meta:
    id: SESSION_L4LB_TCP

proto: tcp

entries:
    - entry:
        label: l4lb
        tracking: False
        timestamp: False
        responder: 
            port : None # Comes from the L4LbService object
        initiator: 
            port : const/44444
