# Firewall Packet Tracker Specification
connections:
    - connection:
        id  : DEFAULT
        # Starting parameters of the connections
        initiator:
            port    : range/32000/33000
            seq     : 0xffffff00
            ack     : 0
            window  : 8192
            scale   : 1
            mss     : 1460
        responder:
            port    : range/32000/33000
            seq     : 0xfffffff0
            ack     : 0
            window  : 8192
            scale   : 1
            mss     : 1460
    - connection:
        id  : SEQ_ROLLOVER
        # Starting parameters of the connections
        initiator:
            port    : range/34000/35000
            seq     : 0xfffffff0
            ack     : 0
            window  : 8192
            scale   : 1
            mss     : 1460
        responder:
            port    : range/34000/35000
            seq     : 2000
            ack     : 0
            window  : 8192
            scale   : 1
            mss     : 1460

