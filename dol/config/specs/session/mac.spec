# Flow generation configuration template.

meta:
    id: SESSION_MAC

proto: None
entries:
    - entry:
        label: networking
        initiator:
            ethertype: 0xCCC0
        responder:
            ethertype: 0xCCC0

    - entry:
        label: arp
        initiator:
            ethertype: 0x806
        responder:
            ethertype: 0x806

    - entry:
        label: span
        initiator:
            ethertype: 0xCC55
            span: ref://store/objects/id=SpanSession0002
        responder:
            ethertype: 0xCC55
            span: ref://store/objects/id=SpanSession0002

