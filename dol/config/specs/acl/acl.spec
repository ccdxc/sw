# Segment Configuration Spec
meta:
    id: ACL

entries:
    - entry:
        id: ACL_SMAC_DMAC_ETYPE_ACTION_DROP 
        match:
            type: eth
            eth:
                ethertype      : ethertype/0x1234
                ethertype_mask : const/0xffff
                src            : macaddr/0000.1111.2222
                src_mask       : macaddr/FFFF.FFFF.FFFF
                dst            : macaddr/0000.3333.4444
                dst_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_SIP_DIP_ACTION_DROP
        match:
            type: ip
            ip:
                type           : v4
                alloc_src_ip   : True
                alloc_dst_ip   : True
        action:
            action: deny

    - entry:
        id: ACL_IPV4_DIP_TCP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v4
                alloc_dst_ip    : True
            l4:
                type            : tcp
                tcp: 
                    dst_port_range: range/1024/2047
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_DIP_UDP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v4
                alloc_dst_ip    : True
            l4:
                type            : udp
                udp: 
                    dst_port_range: range/2048/2559
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_DIP_PREFIX_ACTION_DROP
        match:
            type: ip
            ip:
                type                : v4
                alloc_dst_prefix    : True
                dst_prefix_len      : const/16
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_PROTO_ACTION_DROP
        match:
            type: ip
            ip:
                type    : v4
            l4:
                type    : proto
                proto   : const/200
        action:
            action: deny 

#    - entry:
#        id: ACL_IPV4_OPTIONS_ACTION_DROP
#        match:
#            type: ip
#            ip:
#                type    : v4
#                options : True    
#        action:
#            action: deny 

    - entry:
        id: ACL_IPV6_SIP_DIP_ACTION_DROP
        match:
            type: ip
            ip:
                type           : v6
                alloc_src_ip   : True
                alloc_dst_ip   : True
        action:
            action: deny

    - entry:
        id: ACL_IPV6_DIP_TCP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v6
                alloc_dst_ip    : True
            l4:
                type            : tcp
                tcp: 
                    dst_port_range: range/3072/4095
        action:
            action: deny 

    - entry:
        id: ACL_IPV6_DIP_UDP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v6
                alloc_dst_ip    : True
            l4:
                type            : udp
                udp: 
                    dst_port_range: range/4096/4352
        action:
            action: deny 

    - entry:
        id: ACL_IPV6_DIP_PREFIX_ACTION_DROP
        match:
            type: ip
            ip:
                type             : v6
                alloc_dst_prefix : True
                dst_prefix_len   : const/64
        action:
            action: deny 

    - entry:
        id: ACL_IPV6_PROTO_ACTION_DROP
        match:
            type: ip
            ip:
                type    : v6
            l4:
                type    : proto
                proto   : const/190
        action:
            action: deny 

#    - entry:
#        id: ACL_IPV6_OPTIONS_ACTION_DROP
#        match:
#            type : ip
#            ip:
#                type            : v6
#                options         : True    
#        action:
#            action: deny 

    - entry:
        id: ACL_TCP_SPORT_ACTION_DROP
        match:
            type : ip
            l4:
                type : tcp
                tcp: 
                    src_port_range: range/5000/5007
        action:
            action: deny 

    - entry:
        id: ACL_TCP_DPORT_ACTION_DROP
        match:
            type : ip
            l4:
                type : tcp
                tcp: 
                    dst_port_range : range/6000/6015
        action:
            action: deny 

#
#    - entry:
#        id: ACL_TCP_OPTIONS_ACTION_DROP
#

    - entry:
        id: ACL_TCP_FLAGS_ACTION_DROP
        match:
            type: ip
            l4:
                type: tcp
                tcp:
                    syn: True    
                    ack: True
                    fin: False
        action:
            action: deny

    - entry:
        id: ACL_UDP_SPORT_ACTION_DROP
        match:
            type : ip
            l4:
                type : udp
                udp: 
                    src_port_range : range/7000/7007
        action:
            action: deny 
        
    - entry:
        id: ACL_UDP_DPORT_ACTION_DROP
        match:
            type : ip
            l4:
                type : udp
                udp: 
                    dst_port_range : range/8000/8063
        action:
            action: deny 

    - entry:
        id: ACL_ICMP_ACTION_DROP
        match:
            type : ip
            ip:
                type: v4
            l4:
                type : icmp
                icmp :
                    code        : const/0
                    code_mask   : const/0xff
                    type        : const/0x08
                    type_mask   : const/0xff
        action:
            action: deny

    - entry:
        id: ACL_ICMPV6_ACTION_DROP
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    code        : const/0
                    code_mask   : const/0xff
                    type        : const/0x80
                    type_mask   : const/0xff
        action:
            action: deny

    - entry:
        id: ACL_TENANT_ACTION_DROP
        match:
            tenant_match : True
        action:
            action: deny

    - entry:
        id: ACL_SEGMENT_ACTION_DROP
        match:
            segment_match : True
        action:
            action: deny

    - entry:
        id: ACL_VNID_ACTION_DROP
        match:
            segment_match : True
        action:
            action: deny

    - entry:
        id: ACL_DIF_ACTION_DROP
        match:
            dst_if_match : True
        action:
            action: deny

    - entry:
        id: ACL_SIF_ACTION_DROP
        match:
            src_if_match : True
        action:
            action: deny
