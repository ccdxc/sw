# Flow generation configuration template.

meta:
    id: SESSION_MAC_BASIC

proto: None
entries:
    - entry:
        initiator:
            ethertype: 0xCCC0
        responder:
            ethertype: 0xCCC0
        span     : rspan

    - entry:
        initiator:
            ethertype: 0xCCC1
        responder:
            ethertype: 0xCCC1
        span     : rspan_vxlan
