# Segment Configuration Spec
meta:
    id: DOS

entries:
    - entry:
        id: SERVICE_TCP
        sgs     : local
        ingress :
            peersg  : None
            service :
                proto   : tcp
                port    : const/47274
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
        id: SERVICE_UDP
        sgs     : local
        ingress :
            peersg  : None
            service :
                proto   : udp
                port    : const/47274
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
        id: SERVICE_ICMP
        sgs     : local
        ingress :
            peersg  : None
            service :
                proto   : icmp
                port    : None
                icmp_msg_type: const/0
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
        sgs     : local
        ingress :
            peersg  : None
            service :
                proto   : icmpv6
                port    : None
                icmp_msg_type: const/129
            icmp_flood_limits:
                restrict_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10
                protect_limits:
                    pps     : const/10000
                    burst   : const/128
                    duration: const/10

#    - entry:
#        id: SERVICE_ANY
#        sgs     : local
#        ingress :
#            peersg  : None
#            other_flood_limits:
#                restrict_limits:
#                    pps     : const/10000
#                    burst   : const/128
#                    duration: const/10
#                protect_limits:
#                    pps     : const/10000
#                    burst   : const/128
#                    duration: const/10

    - entry:
        id: SRC_DST_TCP
        sgs     : remote
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
        sgs     : remote
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
        sgs     : remote
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
        sgs     : remote
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

#    - entry:
#        id: SRC_DST_ANY
#        sgs     : remote
#        ingress  :
#            peersg  : remote
#            other_flood_limits:
#                restrict_limits:
#                    pps     : const/10000
#                    burst   : const/128
#                    duration: const/10
#                protect_limits:
#                    pps     : const/10000
#                    burst   : const/128
#                    duration: const/10

    - entry:
        id: FROM_WORKLOAD
        sgs     : local
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

