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
        id: ACL_FROM_ENIC_DMAC_ACTION_DROP 
        match:
            direction : from_enic
            type: eth
            eth:
                dst_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: deny 

    - entry:
        id: ACL_FROM_UPLINK_DMAC_ACTION_DROP 
        match:
            direction : from_uplink
            type: eth
            eth:
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

    - entry:
        id: ACL_FROM_ENIC_IPV4_DIP_ACTION_DROP
        match:
            direction : from_enic
            type: ip
            ip:
                type             : v4
                dst_prefix_len   : const/16
        action:
            action: deny

    - entry:
        id: ACL_FROM_UPLINK_IPV4_DIP_ACTION_DROP
        match:
            direction : from_uplink
            type: ip
            ip:
                type             : v4
                dst_prefix_len   : const/16
        action:
            action: deny

    - entry:
        id: ACL_IPV4_OPTIONS_ACTION_DROP
        match:
            type: ip
            ip:
                type    : v4
                options : True    
        action:
            action: deny 

    - entry:
        id: ACL_IPV4_FRAGMENT_ACTION_DROP
        match:
            type: ip
            ip:
                type     : v4
                fragment : True    
        action:
            action: deny 

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

    - entry:
        id: ACL_FROM_ENIC_IPV6_DIP_ACTION_DROP
        match:
            direction : from_enic
            type: ip
            ip:
                type             : v6
                dst_prefix_len   : const/64
        action:
            action: deny

    - entry:
        id: ACL_FROM_UPLINK_IPV6_DIP_ACTION_DROP
        match:
            direction : from_uplink
            type: ip
            ip:
                type             : v6
                dst_prefix_len   : const/64
        action:
            action: deny

    - entry:
        id: ACL_IPV6_OPTIONS_ACTION_DROP
        match:
            type : ip
            ip:
                type            : v6
                options         : True    
        action:
            action: deny 

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

    - entry:
        id: ACL_TEP_MISS_ACTION_DROP
        match:
            tep_miss : True
        action:
            action : deny

##############################################################################
# SUP REDIRECT
##############################################################################
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
        id: ACL_IPV4_IGMP_ACTION_SUP_REDIRECT
        config_flow_miss: False
        match:
            type: ip
            ip:
                type : v4
            l4:
                type : proto
                proto : const/2
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_ICMPV6_MLD_REQ_ACTION_SUP_REDIRECT
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    type        : const/130
                    code_mask   : const/0xff
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_ICMPV6_MLD_RES_ACTION_SUP_REDIRECT
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    type        : const/131
                    code_mask   : const/0xff
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_ICMPV6_MLD_DONE_ACTION_SUP_REDIRECT
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    type        : const/132
                    code_mask   : const/0xff
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_ICMPV6_RS_ACTION_SUP_REDIRECT
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    type        : const/133
                    code_mask   : const/0xff
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_ICMPV6_RA_ACTION_SUP_REDIRECT
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    type        : const/134
                    code_mask   : const/0xff
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_ICMPV6_NS_ACTION_SUP_REDIRECT
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    type        : const/135
                    code_mask   : const/0xff
        action:
            action: redirect
            intf: cpu

    - entry:
        id: ACL_ICMPV6_NA_ACTION_SUP_REDIRECT
        match:
            type : ip
            ip:
                type: v6
            l4:
                type : icmp
                icmp :
                    type        : const/136
                    code_mask   : const/0xff
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
        id: ACL_ETYPE_FLOW_MISS_ACTION_SUP_REDIRECT 
        config_flow_miss: True
        match:
            type: eth
            eth:
                ethertype_mask : const/0xffff
        action:
            action: redirect 
            intf: cpu

    - entry:
        id: ACL_UDP_DPORT_FLOW_MISS_ACTION_SUP_REDIRECT
        config_flow_miss: True
        match:
            type : ip
            l4:
                type : udp 
                udp: 
                    dst_port : True
        action:
            action: redirect
            intf: cpu

##############################################################################
# REDIRECT
##############################################################################
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

    - entry:
        id: ACL_SIF_ACTION_REDIRECT
        match:
            src_if_match : True
        action:
            action: redirect
            intf: uplink

    - entry:
        id: ACL_SIF_ACTION_TUNNEL_REDIRECT
        match:
            src_if_match : True
        action:
            action: redirect
            intf: tunnel

    - entry:
        id: ACL_DMAC_FLOW_MISS_ACTION_REDIRECT
        match:
            flow_miss: True
            type: eth
            eth:
                dst_mask       : macaddr/FFFF.FFFF.FFFF
        action:
            action: redirect
            intf: uplink

    - entry:
        id: ACL_TCP_DPORT_FLOW_MISS_ACTION_REDIRECT
        match:
            flow_miss: True
            type : ip
            l4:
                type : tcp
                tcp: 
                    dst_port : True
        action:
            action: redirect
            intf: uplink

##############################################################################
# LOG 
##############################################################################
    - entry:
        id: ACL_ETYPE_ACTION_LOG 
        match:
            type: eth
            eth:
                ethertype_mask : const/0xffff
        action:
            action: log 

    - entry:
        id: ACL_IPV4_DIP_ACTION_LOG
        match:
            type: ip
            ip:
                type           : v4
                dst_prefix_len : const/32
        action:
            action: log

    - entry:
        id: ACL_IPV6_DIP_ACTION_LOG
        match:
            type: ip
            ip:
                type           : v6
                dst_prefix_len : const/32
        action:
            action: log

    - entry:
        id: ACL_ETYPE_FLOW_MISS_ACTION_LOG 
        config_flow_miss: True
        match:
            type: eth
            eth:
                ethertype_mask : const/0xffff
        action:
            action: log 

    - entry:
        id: ACL_UDP_DPORT_FLOW_MISS_ACTION_LOG
        config_flow_miss: True
        match:
            type : ip
            l4:
                type : udp 
                udp: 
                    dst_port : True
        action:
            action: log

##############################################################################
# Mirror
##############################################################################
    - entry:
        id: ACL_TCP_FLAGS_ACTION_MIRROR
        match:
            type: ip
            l4:
                type: tcp
                tcp:
                    syn: True
                    ack: True
                    fin: True
        action:
            action: permit
            mirror:
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006

    - entry:
        id: ACL_DROPMASK_ACTION_MIRROR
        config_flow_miss: True
        match:
            dropmask : 
                - FLOW_MISS
            type : ip
            l4:
                type : udp 
                udp: 
                    dst_port : True
        action:
            action: permit
            mirror:
                ingress:
                    - ref://store/objects/id=SpanSession0001
                    - ref://store/objects/id=SpanSession0002
                    - ref://store/objects/id=SpanSession0003
                egress:
                    - ref://store/objects/id=SpanSession0004
                    - ref://store/objects/id=SpanSession0005
                    - ref://store/objects/id=SpanSession0006
