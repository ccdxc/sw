# Segment Configuration Spec
meta:
    id: ACL

entries:
    - entry:
        priority:   0
        match:
            interface:
                src: None
                dst: None
            segment: filter://any
            eth:
                ethertype   : ethertype/0x1234
                src         : macaddr/0000.1111.2222
                src_mask    : macaddr/FFFF.FFFF.FFFF
                dst         : macaddr/0000.3333.4444
                dst_mask    : macaddr/FFFF.FFFF.FFFF
            ip: None
            l4: None
        action:
            action: deny 
            ingress_mirror:
                enable  : False
                session : ref://store/objects/id=SpanSession0001
            egress_mirror:
                enable  : False 
                session : ref://store/objects/id=SpanSession0001
    - entry:
        priority:   0
        match:
            interface:
                src: None
                dst: None
            segment: filter://any
            eth: None
            ip:
                src         : ipaddr/150.0.0.1
                dst         : ipaddr/150.0.0.2
                proto       : const/6
            l4:
                tcp:
                    syn         : const/1
                    ack         : const/0
                    fin         : const/0
                    rst         : const/0
                    urg         : const/0
                    port_range  : range/500/1000
                udp: None
                icmp: None
        action:
            action: deny
            ingress_mirror: None
            egress_mirror: None

    - entry:
        priority:   0 
        match:
            interface:
                src: None
                dst: None
            segment: filter://any
            eth: None
            ip:
                src         : ipaddr/150.0.0.1
                dst         : ipaddr/150.0.0.2
                proto       : const/17
            l4:
                tcp: None
                udp:
                    port_range  : range/500/1000
                icmp: None
        action:
            action: deny 
            ingress_mirror: None
            egress_mirror: None

    - entry:
        priority:   0
        match:
            interface:
                src: None
                dst: None
            segment: filter://any
            eth: None
            ip:
                src         : ipaddr/150.0.0.1
                dst         : ipaddr/150.0.0.2
                proto       : const/1
            l4:
                tcp: None
                udp: None
                icmp:
                    code        : const/1
                    type        : const/0
        action:
            action: deny 
            ingress_mirror: None
            egress_mirror: None
