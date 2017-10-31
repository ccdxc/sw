# Segment Configuration Spec
meta:
    id: DOS

entries:
    - entry:
        id: SERVICE_TCP
        sgs     : local
        ingress :
            peersg  : remote
            service :
                proto   : tcp
                port    : const/22222
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
        egress  :
            peersg  : remote
            service :
                proto   : tcp
                port    : const/22222
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
        id: SRC_VF
        sgs     : local
        ingress :
            peersg  : remote
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

        egress  :
            peersg  : remote
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

