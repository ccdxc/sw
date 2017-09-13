# Flow generation configuration template.
meta:
    id: SESSION_MAC_ETH_VXLAN

proto: None
entries:
    - entry:
        label: networking
        initiator:
            ethertype: 0xCCC0
            flow_info:
                eg_qos:    
                    cos_rw  : const/1
                    cos     : const/5
        responder:
            ethertype: 0xCCC0

    - entry:
        label: gft_drop
        tracking: False
        timestamp: False
        responder: 
            ethertype: 0xDDDD
            flow_info:
                action  : drop
        initiator: 
            ethertype: 0xDDDD
            flow_info:
                action  : drop

    - entry:
        label: arp
        initiator:
            ethertype: 0x806
        responder:
            ethertype: 0x806

