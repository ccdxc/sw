meta:
    id: GFT_TRANSPOSITION_PROFILES

transposition_profiles:
    - profile:
        name    : GFT_TRSPN_VLAN_NONE

    - profile:
        name    : GFT_TRSPN_VLAN_MODIFY
        groups  :
            - ref://store/objects/id=GFT_TRSPN_HDR_GRP_VLAN_MODIFY

    - profile:
        name    : GFT_TRSPN_VLAN_PUSH
        groups  :
            - ref://store/objects/id=GFT_TRSPN_HDR_GRP_VLAN_PUSH

    - profile:
        name    : GFT_TRSPN_VLAN_POP
        groups  :
            - ref://store/objects/id=GFT_TRSPN_HDR_GRP_VLAN_POP

