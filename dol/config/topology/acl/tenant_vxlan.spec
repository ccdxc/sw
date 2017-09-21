# Configuration Template.
meta:
    id: ACL_TENANT_OVERLAY_VXLAN

type    : tenant
overlay : vxlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_ACL
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_VXLAN_ACL
      count : 2

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 4

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_TCP_ACL
            - ref://store/specs/id=SESSION_UDP_ACL
            - ref://store/specs/id=SESSION_ICMP_ACL
        ipv6:
            - ref://store/specs/id=SESSION_TCP_ACL
            - ref://store/specs/id=SESSION_UDP_ACL
            - ref://store/specs/id=SESSION_ICMPV6_ACL
        mac:
            - ref://store/specs/id=SESSION_MAC_ACL

    multidest:
        scale: 16
        ipv4:
        ipv6:
        mac:

lif: ref://store/specs/id=LIF_DEFAULT
