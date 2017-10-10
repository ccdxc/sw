# Flow generation configuration template.
meta:
    id: SESSION_MAC_FTE

proto: None
entries:
    - entry:
        label: fte
        fte: True
        initiator:
            ethertype: 0xCCC0
        responder:
            ethertype: 0xCCC0

    - entry:
        label: arp
        fte: True
        initiator:
            ethertype: 0x806
        responder:
            ethertype: 0x806

