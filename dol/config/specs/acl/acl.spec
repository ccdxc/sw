# Segment Configuration Spec
meta:
    id: ACL

entries:
    - entry:
        id: ACL_SMAC_DMAC_ETYPE_FLOW_MISS_ACTION_DROP 
        match:
            flow_miss: True
            type: eth
            eth:
                ethertype_mask : const/0xffff
                src_mask       : macaddr/FFFF.FFFF.FFFF
                dst_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: deny 

    - entry:
        id: ACL_SMAC_DMAC_ETYPE_ACTION_DROP 
        match:
            type: eth
            eth:
                ethertype_mask : const/0xffff
                src_mask       : macaddr/FFFF.FFFF.FFFF
                dst_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_SIP_DIP_FLOW_MISS_ACTION_DROP
        match:
            flow_miss: True
            type: ip
            ip:
                type           : v4
                src_prefix_len : const/32
                dst_prefix_len : const/32
        action:
            action: deny

    - entry:
        id: ACL_IPV4_SIP_DIP_ACTION_DROP
        match:
            type: ip
            ip:
                type           : v4
                src_prefix_len : const/32
                dst_prefix_len : const/32
        action:
            action: deny

    - entry:
        id: ACL_IPV4_DIP_TCP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v4
                dst_prefix_len  : const/32
            l4:
                type            : tcp
                tcp: 
                    dst_port    : True
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_DIP_UDP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v4
                dst_prefix_len  : const/32
            l4:
                type            : udp
                udp: 
                    dst_port    : True
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_DIP_PREFIX_ACTION_DROP
        match:
            type: ip
            ip:
                type                : v4
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
        id: ACL_IPV6_SIP_DIP_FLOW_MISS_ACTION_DROP
        match:
            flow_miss: True
            type: ip
            ip:
                type           : v6
                src_prefix_len : const/128
                dst_prefix_len : const/128
        action:
            action: deny

    - entry:
        id: ACL_IPV6_SIP_DIP_ACTION_DROP
        match:
            type: ip
            ip:
                type           : v6
                src_prefix_len : const/128
                dst_prefix_len : const/128
        action:
            action: deny

    - entry:
        id: ACL_IPV6_DIP_TCP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v6
                dst_prefix_len  : const/128
            l4:
                type            : tcp
                tcp: 
                    dst_port    : True
        action:
            action: deny 

    - entry:
        id: ACL_IPV6_DIP_UDP_DPORT_ACTION_DROP
        match:
            type: ip
            ip:
                type            : v6
                dst_prefix_len  : const/128
            l4:
                type            : udp
                udp: 
                    dst_port    : True
        action:
            action: deny 

    - entry:
        id: ACL_IPV6_DIP_PREFIX_ACTION_DROP
        match:
            type: ip
            ip:
                type             : v6
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
                    src_port : True
        action:
            action: deny 

    - entry:
        id: ACL_TCP_DPORT_ACTION_DROP
        match:
            type : ip
            l4:
                type : tcp
                tcp: 
                    dst_port : True
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
                    src_port : True
        action:
            action: deny 
        
    - entry:
        id: ACL_UDP_DPORT_ACTION_DROP
        match:
            type : ip
            l4:
                type : udp
                udp: 
                    dst_port : True
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
                    code_mask   : const/0xff
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
                    code_mask   : const/0xff
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

##############################################################################
##############################################################################
##############################################################################
# SUP REDIRECT
    - entry:
        id: ACL_SMAC_ACTION_SUP_REDIRECT
        match:
            type: eth
            eth:
                src_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: redirect 
            intf: cpu

    - entry:
        id: ACL_SMAC_ETYPE_ACTION_SUP_REDIRECT 
        match:
            type: eth
            eth:
                ethertype_mask : const/0xffff
                src_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: redirect 
            intf: cpu

    - entry:
        id: ACL_DMAC_ACTION_SUP_REDIRECT 
        match:
            type: eth
            eth:
                dst_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: redirect 
            intf: cpu

    - entry:
        id: ACL_ETYPE_ACTION_SUP_REDIRECT 
        match:
            type: eth
            eth:
                ethertype_mask : const/0xffff
        action:
            action: redirect 
            intf: cpu

    - entry:
        id: ACL_IPV4_DIP_ACTION_SUP_REDIRECT
        match:
            type: ip
            ip:
                type           : v4
                dst_prefix_len : const/32
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_IPV6_DIP_ACTION_SUP_REDIRECT
        match:
            type: ip
            ip:
                type           : v6
                dst_prefix_len : const/32
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_TCP_DPORT_ACTION_SUP_REDIRECT
        match:
            type : ip
            l4:
                type : tcp
                tcp: 
                    dst_port : True
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_UDP_DPORT_ACTION_SUP_REDIRECT
        match:
            type : ip
            l4:
                type : udp
                udp: 
                    dst_port : True
        action:
            action: redirect 
            intf: cpu

    - entry:
        id: ACL_DIF_ACTION_REDIRECT
        match:
            dst_if_match : True
        action:
            action: redirect
            intf: uplink

    - entry:
        id: ACL_DIF_ACTION_TUNNEL_REDIRECT
        match:
            dst_if_match : True
        action:
            action: redirect
            intf: tunnel
