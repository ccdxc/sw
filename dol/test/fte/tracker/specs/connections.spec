# FTE Packet Tracker Specification
connections:
    - connection:
        id  : DEFAULT
        # Starting parameters of the connections
        initiator:
            port    : range/32000/33000
            seq     : 1000
            ack     : 0
        responder:
            port    : range/32000/33000
            seq     : 2000
            ack     : 0
