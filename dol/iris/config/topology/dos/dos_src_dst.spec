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
                icmp_msg: None
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
                icmp_msg: None
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
                icmp_msg:
                    type    : const/8
                    code    : const/0
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
                icmp_msg:
                    type    : const/128
                    code    : const/0
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
                icmp_msg: None
            other_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
