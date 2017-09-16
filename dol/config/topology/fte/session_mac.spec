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
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/5
        responder:
            ethertype: 0xCCC0

    - entry:
        label: fte_arp
        fte: True
        initiator:
            ethertype: 0x806
        responder:
            ethertype: 0x806

