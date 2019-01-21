# Configuration Template.
meta:
    id: MPLS_UDPTOPO_TENANT_OVERLAY_MPLS_UDP

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=MPLS_UDPTOPO_SEGMENT_DEFAULT
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_MPLS_UDP
      local : False

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=MPLS_UDPTOPO_SESSION_TCP
            - ref://store/specs/id=MPLS_UDPTOPO_SESSION_UDP
            - ref://store/specs/id=MPLS_UDPTOPO_SESSION_ICMP
        ipv6:
            - ref://store/specs/id=MPLS_UDPTOPO_SESSION_TCP
            - ref://store/specs/id=MPLS_UDPTOPO_SESSION_UDP
            - ref://store/specs/id=MPLS_UDPTOPO_SESSION_ICMPV6
        mac:
            - ref://store/specs/id=MPLS_UDPTOPO_SESSION_MAC

lif: ref://store/specs/id=LIF_ETH
