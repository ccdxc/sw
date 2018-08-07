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

    - entry:
        label: span
        initiator:
            ethertype: 0xCC55
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006
        responder:
            ethertype: 0xCC55
            span: 
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006

