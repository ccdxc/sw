# Configuration Template.
meta:
    id: TENANT_PROXY

type    : tenant
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_NATIVE_PROXY
      count : 1
    - spec  : ref://store/specs/id=SEGMENT_PROXY
      count : 1

# NFV Endpoints:
# - They will attach to a 'TRUNK' Enic.
# - All the segments will be enabled on these Enics              
nfveps: 0

security_profile: ref://store/objects/id=SEC_PROF_ACTIVE

sessions:
    unidest:
        ipv6:
            - ref://store/specs/id=SESSION_TCP_PROXY
            - ref://store/specs/id=SESSION_ESP_PROXY
        ipv4:
            - ref://store/specs/id=SESSION_TCP_PROXY
            - ref://store/specs/id=SESSION_ESP_PROXY
            - ref://store/specs/id=SESSION_ICMP_IPSEC_ENC
            - ref://store/specs/id=SESSION_UDP_PROXY
        mac:
            - ref://store/specs/id=SESSION_MAC_PROXY

lif: ref://store/specs/id=LIF_ETH
