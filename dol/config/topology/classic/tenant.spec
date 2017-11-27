# Configuration Template.
meta:
    id: TENANT_CLASSIC

type    : tenant
overlay : vlan

enics:
    classic : 2

segments:
    - spec  : ref://store/specs/id=SEGMENT_CLASSIC_DEFAULT
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_CLASSIC_NATIVE
      count : 1

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_CLASSIC
            - ref://store/specs/id=SESSION_TCP_CLASSIC
            - ref://store/specs/id=SESSION_ICMP_CLASSIC
        ipv6:
            - ref://store/specs/id=SESSION_UDP_CLASSIC
            - ref://store/specs/id=SESSION_TCP_CLASSIC
            - ref://store/specs/id=SESSION_ICMPV6_CLASSIC
        mac:
            - ref://store/specs/id=SESSION_MAC_CLASSIC

lif: ref://store/specs/id=LIF_ETH
