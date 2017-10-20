# Configuration Template.
meta:
    id: TENANT_MULTICAST_INFRA

type    : infra
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_MULTICAST_INFRA
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_VXLAN

security_profile: None

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=MULTICAST_TOPO_SESSION_DUMMY
        ipv6: None
        mac: None

lif: ref://store/specs/id=LIF_ETH
