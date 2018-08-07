# Configuration Template.
meta:
    id: TENANT_DOS_SRC_DST

type    : tenant
overlay : vlan
label   : DOS_SRC_DST

segments:
    - spec  : ref://store/specs/id=SEGMENT_DOS_SRC_DST
      count : 1

security_profile: None

dos: ref://store/specs/id=DOS_SRC_DST

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_FTE
            - ref://store/specs/id=SESSION_TCP_FTE
            - ref://store/specs/id=SESSION_ICMP_FTE
            - ref://store/specs/id=SESSION_PR255_DOS
        ipv6:
            - ref://store/specs/id=SESSION_UDP_FTE
            - ref://store/specs/id=SESSION_TCP_FTE
            - ref://store/specs/id=SESSION_ICMPV6_FTE
            - ref://store/specs/id=SESSION_PR255_DOS
        mac:
            - ref://store/specs/id=SESSION_MAC_FTE

lif: ref://store/specs/id=LIF_ETH
