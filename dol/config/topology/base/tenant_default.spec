# Configuration Template.
meta:
    id: BASE_TENANT_OVERLAY_VLAN

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=BASE_SEGMENT_NATIVE
      count : 1
    - spec  : ref://store/specs/id=BASE_SEGMENT_DEFAULT
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

#l4lb: ref://store/specs/id=L4LB

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP
            - ref://store/specs/id=SESSION_UDP
            - ref://store/specs/id=SESSION_UDP_SERVICES
            - ref://store/specs/id=SESSION_ICMP
        ipv6:
            - ref://store/specs/id=SESSION_TCP
            - ref://store/specs/id=SESSION_UDP
            - ref://store/specs/id=SESSION_UDP_SERVICES
            - ref://store/specs/id=SESSION_ICMPV6
        mac:
            - ref://store/specs/id=SESSION_MAC

    multidest:
        ipv4:
            - ref://store/specs/id=SESSION_L2MC
            - ref://store/specs/id=SESSION_L2BC
        ipv6:
            - ref://store/specs/id=SESSION_L2MC
            - ref://store/specs/id=SESSION_L2BC
        mac:
            - ref://store/specs/id=SESSION_L2MC
            - ref://store/specs/id=SESSION_L2BC

lif: ref://store/specs/id=LIF_ETH_RDMA
