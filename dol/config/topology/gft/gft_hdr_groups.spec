# GFT Exact Match Profiles Specification
# Profile Defaults:
meta:
    id: GFT_HEADER_GROUPS

header_groups:
    - group:
        name    : GFT_HGEXM_ETH
        headers :
            ethernet_header : True
        fields  :
            dst_mac_addr    : True
            src_mac_addr    : True

    - group:
        name    : GFT_HGEXM_ETH_NATIVE
        inherit :
            - ref://store/objects/id=GFT_HGEXM_ETH
        headers :
            no_encap        : True

    - group:
        name    : GFT_HGEXM_ETH_QTAG
        inherit :
            - ref://store/objects/id=GFT_HGEXM_ETH
        fields  :
            customer_vlan_id: True
            dot1p_priority  : True

    - group:
        name    : GFT_HGEXM_ETH_QINQ
        inherit :
            - ref://store/objects/id=GFT_HGEXM_ETH
        fields  :
            customer_vlan_id: True
            provider_vlan_id: True
            dot1p_priority  : True

    - group:
        name    : GFT_HGEXM_IPV4_NATIVE
        inherit :
            - ref://store/objects/id=GFT_HGEXM_ETH_NATIVE
        headers :
            ipv4_header     : True
        fields  :
            src_ip_addr     : True
            dst_ip_addr     : True
            ip_ttl          : True
            ip_protocol     : True
            ip_dscp         : True

    - group:
        name    : GFT_HGEXM_IPV4_QTAG
        inherit :
            - ref://store/objects/id=GFT_HGEXM_ETH_QTAG
        headers :
            ipv4_header     : True
        fields  :
            src_ip_addr     : True
            dst_ip_addr     : True
            ip_ttl          : True
            ip_protocol     : True
            ip_dscp         : True

    - group:
        name    : GFT_HGEXM_IPV4_QINQ
        inherit :
            - ref://store/objects/id=GFT_HGEXM_ETH_QINQ
        headers :
            ipv4_header     : True
        fields  :
            src_ip_addr     : True
            dst_ip_addr     : True
            ip_ttl          : True
            ip_protocol     : True
            ip_dscp         : True

    - group:
        name    : GFT_HGEXM_IPV4_UDP_NATIVE
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_NATIVE
        headers :
            udp_header      : True
        fields  :
            src_port        : True
            dst_port        : True

    - group:
        name    : GFT_HGEXM_IPV4_UDP_QTAG
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_QTAG
        headers :
            udp_header      : True
        fields  :
            src_port        : True
            dst_port        : True

    - group:
        name    : GFT_HGEXM_IPV4_UDP_QINQ
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_QINQ
        headers :
            udp_header      : True
        fields  :
            src_port        : True
            dst_port        : True


    - group:
        name    : GFT_HGEXM_IPV4_TCP_NATIVE
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_NATIVE
        headers :
            tcp_header      : True
        fields  :
            src_port        : True
            dst_port        : True

    - group:
        name    : GFT_HGEXM_IPV4_TCP_QTAG
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_QTAG
        headers :
            tcp_header      : True
        fields  :
            src_port        : True
            dst_port        : True

    - group:
        name    : GFT_HGEXM_IPV4_TCP_QINQ
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_QINQ
        headers :
            tcp_header      : True
        fields  :
            src_port        : True
            dst_port        : True

    - group:
        name    : GFT_HGEXM_IPV4_ICMP_NATIVE
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_NATIVE
        headers :
            icmp_header     : True
        fields  :
            icmp_type       : True
            icmp_code       : True

    - group:
        name    : GFT_HGEXM_IPV4_ICMP_QTAG
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_QTAG
        headers :
            icmp_header     : True
        fields  :
            icmp_type       : True
            icmp_code       : True

    - group:
        name    : GFT_HGEXM_IPV4_ICMP_QINQ
        inherit :
            - ref://store/objects/id=GFT_HGEXM_IPV4_QINQ
        headers :
            icmp_header     : True
        fields  :
            icmp_type       : True
            icmp_code       : True
