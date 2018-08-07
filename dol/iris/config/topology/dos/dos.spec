# Segment Configuration Spec
meta:
    id: DOS

entries:
    - entry:
        id: SERVICE_TCP
        sgs     :
            - label: DOS_TO_WL
        ingress :
            peersg  : None
            service :
                proto   : tcp
                port    : const/47274
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
        id: SERVICE_UDP
        sgs     :
            - label: DOS_TO_WL
        ingress :
            peersg  : None
            service :
                proto   : udp
                port    : const/47274
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
        id: SERVICE_ICMP
        sgs     :
            - label: DOS_TO_WL
        ingress :
            peersg  : None
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
        id: SERVICE_ICMPV6
        sgs     :
            - label: DOS_TO_WL
        ingress :
            peersg  : None
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
        id: SERVICE_ANY
        sgs     :
            - label: DOS_TO_WL
        ingress :
            peersg  : None
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

    - entry:
        id: FROM_WORKLOAD
        sgs     :
            - label: DOS_FROM_WL
        egress  :
            peersg  : None
            tcp_syn_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
            udp_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
            icmp_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
            other_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10

