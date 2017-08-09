# Configuration Template.
meta:
    id: INFRA

type    : infra
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_INFRA
      count : 2

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_VXLAN

security_profile: ref://store/objects/id=SEC_PROF_DEFAULT
sessions:
    unidest:
        scale: 16
        ipv4:
            - ref://store/specs/id=SESSION_UDP_VXLAN

lif:
    spec        : ref://store/specs/id=LIF_ETH
    count       : 8
