# Flow generation configuration template.

meta:
    id: SESSION_MAC

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
        label: arp
        initiator:
            ethertype: 0x806
        responder:
            ethertype: 0x806

    - entry:
        label: span
        initiator:
            ethertype: 0xCC55
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0002
        responder:
            ethertype: 0xCC55
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0002

