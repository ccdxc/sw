# Configuration Template.
meta:
    id: TENANT_OVERLAY_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_DEFAULT
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_BLACKHOLE
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_VXLAN
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    l4lb:
        ipv4:
            - ref://store/specs/id=SESSION_L4LB_TCP
            - ref://store/specs/id=SESSION_L4LB_UDP
        
        ipv6:
            - ref://store/specs/id=SESSION_L4LB_TCP
            - ref://store/specs/id=SESSION_L4LB_UDP

    unidest:
        scale: 16
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
        scale: 16
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
