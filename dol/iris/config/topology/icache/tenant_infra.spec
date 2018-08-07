# Configuration Template.
meta:
    id: ICACHE_TENANT_INFRA

type    : infra
overlay : vlan

segments:
    - spec  : ref://store/specs/id=ICACHE_SEGMENT_INFRA
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_VXLAN

security_profile: ref://store/objects/id=SEC_PROF_DEFAULT
sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_SERVICES
        ipv6: None
        mac: None

lif: ref://store/specs/id=LIF_ETH_RDMA
