# Segment Configuration Spec
meta:
    id: DOS_SRC_DST

entries:
    - entry:
        id: SRC_DST_TCP
        sgs     :
            - label: DOS_SRC_DST
        ingress  :
            peersg  : remote
            service :
                proto   : tcp
                port    : const/47275
                icmp_msg_type: None
            tcp_syn_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10

    - entry:
        id: SRC_DST_UDP
        sgs     :
            - label: DOS_SRC_DST
        ingress  :
            peersg  : remote
            service :
                proto   : udp
                port    : const/47275
                icmp_msg_type: None
            udp_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10

    - entry:
        id: SRC_DST_ICMP
        sgs     :
            - label: DOS_SRC_DST
        ingress  :
            peersg  : remote
            service :
                proto   : icmp
                port    : None
                icmp_msg_type: const/5
            icmp_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10

    - entry:
        id: SRC_DST_ICMPV6
        sgs     :
            - label: DOS_SRC_DST
        ingress  :
            peersg  : remote
            service :
                proto   : icmpv6
                port    : None
                icmp_msg_type: const/133
            icmp_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10

    - entry:
        id: SRC_DST_ANY
        sgs     :
            - label: DOS_SRC_DST
        ingress  :
            peersg  : remote
            service :
                proto   : proto255
                port    : None
                icmp_msg_type: None
            other_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10

