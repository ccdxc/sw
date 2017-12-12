# Configuration Template.
meta:
    id: TENANT_INFRA_PROXY

type    : infra
overlay : vlan

segments:
    - spec  : ref://store/specs/id=SEGMENT_INFRA_PROXY
      count : 1

tunnels:
    - spec  : ref://store/specs/id=TUNNEL_VXLAN

security_profile: None
security_policy: ref://store/specs/id=SECURITY_POLICY_PROXY

sessions:
    unidest:
        ipv4:
            - ref://store/specs/id=SESSION_UDP_SERVICES_PROXY
            - ref://store/specs/id=SESSION_ESP_PROXY
        ipv6: 
            - ref://store/specs/id=SESSION_ESP_PROXY
        mac: None

lif: ref://store/specs/id=LIF_ETH
