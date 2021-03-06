meta:
    id: GFT_TRANSPOSITION_HEADER_GROUPS

header_groups:
    - group:
        name    : GFT_TRSPN_HDR_GRP_VLAN_MODIFY
        action  : MODIFY
        headers :
            ethernet_header : True
        fields  :
            customer_vlan_id: True

    - group:
        name    : GFT_TRSPN_HDR_GRP_VLAN_PUSH
        action  : PUSH
        headers :
            ethernet_header : True
        fields  :
            customer_vlan_id: True

    - group:
        name    : GFT_TRSPN_HDR_GRP_VLAN_POP
        action  : POP
        headers :
            ethernet_header : True
        fields  :
            customer_vlan_id: True
