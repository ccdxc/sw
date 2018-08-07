# Configuration Template.
meta:
    id: TENANT_INFRA_FTE

type    : infra
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_INFRA_FTE
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_VXLAN

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_SERVICES_VXLAN
        ipv6: None
        mac: None

lif: ref://store/specs/id=LIF_ETH
