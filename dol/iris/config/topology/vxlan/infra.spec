# Configuration Template.
meta:
    id: VXLANTOPO_TENANT_INFRA

type    : infra
overlay : vlan

segments:
    - spec  : ref://store/specs/id=VXLANTOPO_SEGMENT_INFRA
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_VXLAN

security_profile: ref://store/objects/id=SEC_PROF_DEFAULT
sessions:
    unidest:
        ipv4:
        ipv6: None
        mac: None

lif: ref://store/specs/id=LIF_ETH
