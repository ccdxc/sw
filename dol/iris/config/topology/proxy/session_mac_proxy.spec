# Flow generation configuration template.
meta:
    id: SESSION_MAC_PROXY

proto: None
entries:
    - entry:
        fte: True
        label: quiesce
        initiator:
            ethertype: 0xaaaa
        responder:
            ethertype: 0xaaab
