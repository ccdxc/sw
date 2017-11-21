# Flow generation configuration template.
meta:
    id: SESSION_MAC_CLASSIC

proto: None
entries:
    - entry:
        label: classic
        fte: True
        initiator:
            ethertype: 0xCCC0
        responder:
            ethertype: 0xCCC0

    - entry:
        label: classicl2bc
        fte: True
        initiator:
            ethertype: 0xCCC2
        responder:
            ethertype: 0xCCC2
